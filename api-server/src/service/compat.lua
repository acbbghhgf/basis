local bifrost = require "bifrost"
local service = require "service"
local xfer = require "xfer.core"
local berror = require "berror"
local json = require "dkjson"
local env = require "env"

local addrStream = {}
local streamAddr = {}
local idStream = {}
local UPLOAD_POOL_ID

local serviceTable = {
    ["cn.tts"]      = {false, false},   -- [needResponse, resultNeedPostProcessing]
    ["en.tts"]      = {false, false},
    ["cn.asr.rec"]  = {true,  true},
    ["en.asr.rec"]  = {true,  true},
    ["en.eval.snt"] = {true,  true},
    ["en.eval.wrd"] = {true,  true},
    ["cn.usc.rec"]  = {true,  true},
}

local iTypeTable = {        -- [needResponse, resultNeedPostProcessing]
    [1] = {true, false},
    [2] = {true, true},
    [3] = {false, true},
    [4] = {false, false},
}

local function needResponse(coreType, iType)
    local ret = serviceTable[coreType]
    if ret ~= nil then return table.unpack(ret) end
    if coreType ~= "cn.robot" then return end
    ret = iTypeTable[iType]
    if ret ~= nil then return table.unpack(ret) end
end

local function processCmdStart(body, msg)
    local param = body.param
    local errMsg, streamId
    local caddr = msg.id
    local appId, deviceId, token, hook
    repeat
        if not param then
            errMsg = berror.getError("REQ", "missing param")
            break
        end
        local app = param.app
        if not app then
            errMsg = berror.getError("REQ", "missing app")
            break
        end
        appId = app.applicationId
        if not appId then
            errMsg = berror.getError("REQ", "missing appId")
            break
        end
        local ip = bifrost.sepString(msg.addr, ":", 2)
        deviceId = ip
        token = "token"
        hook = bifrost.genUUID()

        local deviceCnt = bifrost.call(".statistic", "cmd", "getDeviceCnt",
                                       string.pack("zz", appId, deviceId))
        local maxOpeningStream = env.optnumber("maxOpeningStream", 10)
        if deviceCnt >= maxOpeningStream then
            errMsg = berror.getError("REQ", "too many opening streams")
            break
        end

        streamId = table.concat({appId, deviceId, token, hook}, ":")
        local request = param.request
        if not request then
            errMsg = berror.getError("REQ", "missing request")
            break
        end
        local coreType = request.coreType
        if not coreType then
            errMsg = berror.getError("REQ", "missing coreType")
            break
        end
        local needResponse, resultNeedPostProcessing = needResponse(coreType, request.iType)
        if needResponse == nil then
            errMsg = berror.getError("REQ", "invalid coreType or iType")
            break
        end

        local agent = service.getAgent(coreType) --TODO
        if not agent then
            errMsg = berror.getError("REQ", "invalid coreType or iType")
            break
        end

        local oldStream = addrStream[caddr]
        local newStream = {streamId = streamId, agent = agent, coreType = coreType}
        addrStream[caddr] = newStream
        idStream[streamId] = newStream
        streamAddr[streamId] = caddr

        request.useStream = 0
        if (appId == "Gowild" or appId == "188569ba-987b-11e7-b526-00163e13c8a2") and request.ebnfRes == nil then
            request.ebnfRes = "xiaobai"
        end
        if resultNeedPostProcessing then
            bifrost.send(agent, "lua", "START", streamId, param)
        else
            bifrost.send(agent, "lua", "START", streamId, param, caddr)
        end

        if oldStream then
            bifrost.send(oldStream.agent, "lua", "FLUSH", oldStream.streamId)
            streamAddr[oldStream.streamId] = nil
        end

        if needResponse then
            xfer.xfer(200, {addr=caddr})
        end

        bifrost.send(".statistic", "lua", {
                        action = "newStream",
                        streamId = streamId,
                        service = coreType,
                    })
        return
    until true
    if streamId then
        local tmp = {}
        string.gsub(streamId, "[^:]+", function(w) table.insert(tmp, w) end)
        appId, deviceId, token, hook = table.unpack(tmp, 1, 4)
    end
    local now = bifrost.epoch()
    local ret = xfer.xfer("POST", "/api/v1.0/log/error", {
            addr = UPLOAD_POOL_ID,
            field = {["Content-Type"]="text/plain",host=env.optstring("uploadAddr", "")},
            body = json.encode {
                finishTime = now,
                initTime = now,
                initPack = param,
                isInit = false,
                error = errMsg,
                appId = appId,
                deviceId = "invalid",
            },
        })
    if not ret then bifrost.debug "xfer error" end
    berror.redirectError(errMsg, caddr)
end

local function processCmdStop(body, caddr)
    local stream = addrStream[caddr]
    if not stream then
        berror.sendError("REQ", "unexpected cmd stop", caddr)
        return
    end
    local agent = stream.agent
    bifrost.send(agent, "lua", "APPEND", stream.streamId, nil, caddr)
end

local function processText(msg)
    if not msg.body then
        berror.sendError("REQ", "request empty text plain", msg.id)
        return
    end
    local body = json.decode(msg.body)
    if not body then
        berror.sendError("REQ", "we expect json", msg.id)
        return
    end
    local cmd = body.cmd
    if cmd == "start" then
        processCmdStart(body, msg)
    elseif cmd == "stop" then
        processCmdStop(body, msg.id)
    else
        berror.sendError("REQ", "invalid cmd", msg.id)
        return
    end
end

local function processAudio(msg)
    if not msg.body or #msg.body == 0 then return end
    local caddr = msg.id
    local stream = addrStream[caddr]
    if not stream then
        berror.sendError("REQ", "unexpected audio", msg.id)
        return
    end
    local agent = stream.agent
    bifrost.send(agent, "lua", "APPEND", stream.streamId, msg.body, caddr)
    if not xfer.xfer(200, {addr=caddr}) then bifrost.debug "xfer error" end
end

local cmdHandler = {}

function cmdHandler.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function cmdHandler.DISCONNECT(id)
    local id = tonumber(id)
    local stream = addrStream[id]
    if stream then
        local agent = stream.agent
        bifrost.send(agent, "lua", "FLUSH", stream.streamId)
        addrStream[id] = nil
        streamAddr[stream.streamId] = nil
    end
end

function cmdHandler.ECONNECT(id)
    cmdHandler.DISCONNECT(id)
end

function cmdHandler.RESPONSE(param)
    local streamId, _, body, lua = string.unpack("zzsz", param)
    local caddr = streamAddr[streamId]
    if caddr then
        local dlgResult
        if #lua > 0 then
            dlgResult = lua
        end
        xfer.xfer(200, {
                            addr = caddr,
                            field = {
                                ["Content-Type"] = "text/plain",
                                lua = dlgResult,
                            },
                            body = json.encode{result = json.decode(body)}
                       })
    else
        bifrost.debug(streamId .. " Not Found")
    end
end

function cmdHandler.ERROR(param)
    local streamId, err = string.unpack("zs", param)
    local caddr = streamAddr[streamId]
    if caddr then
        xfer.xfer(500, {addr=caddr, field={["Content-Type"]="text/plain"}, body = err})
    else
        bifrost.debug(streamId .. " Not Found")
    end
end

function cmdHandler.DONE(streamId)
    local caddr = streamAddr[streamId]
    if caddr then
        streamAddr[streamId] = nil
        addrStream[caddr] = nil
    end

    local s = idStream[streamId]
    if s then
        bifrost.send(".statistic", "lua", {
                action = "deleteStream",
                streamId = streamId,
                service = s.coreType,
            })
    end
end

bifrost.start(function()
    bifrost.dispatch("lua", function(session, source, upl)
        UPLOAD_POOL_ID = upl
        bifrost.dispatch("lua", function(session, source, msg)
            local field = msg.field
            if not field then
                berror.sendError("REQ", "request no field", msg.id)
                return
            end
            local contentType = field.contentType
            if contentType == "text/plain" then
                processText(msg)
            else
                processAudio(msg)
            end
        end)
    end)

    bifrost.dispatch("cmd", function(session, source, cmd, param)
        local f = cmdHandler[cmd]
        if f then f(param) end
    end)
end)
