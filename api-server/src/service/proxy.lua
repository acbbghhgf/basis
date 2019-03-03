local bifrost = require "bifrost"
local berror = require "berror"
local json = require "dkjson"
local service = require "service"
local xfer = require "xfer.core"
local env = require "env"

local idStream = {}
local Wlist = {}
local deviceLock = {}

local UPLOAD_POOL_ID

local ebnfDefaultByAppId = {
    Gowild = "xiaobai",
    ["188569ba-987b-11e7-b526-00163e13c8a2"] = "xiaobai",
    ["86ff94ba-4447-11e8-b526-00163e13c8a2"] = "8550613c4f0fb9dcdf0ca8ade444e613",
}

local function sepString(s, sep, n)
    local tmp = {}
    string.gsub(s, string.format("[^%s]+", sep), function(w) table.insert(tmp, w)end)
    return table.unpack(tmp, 1, n)
end

local function streamAppend(stream, msg)
    local contentType = msg.field.contentType
    local sn = msg.field.sn
    local hook = msg.field.hook
    if contentType == "text/plain" then
        if not msg.body then
            berror.sendError("REQ", "we expect cmd stop, but empty body", msg.id, hook, stream.deviceId, sn)
            return
        end
        local body = json.decode(msg.body)
        if not body or body.cmd ~= "stop" then
            berror.sendError("REQ", "we expect cmd stop", msg.id, hook, stream.deviceId, sn)
            return
        end
        bifrost.send(stream.agent, "lua", "APPEND", stream.streamId, nil, msg.id, sn)
        bifrost.log("info", "append stop: " .. stream.streamId)
    else
        if not msg.body or #msg.body == 0 then return end
        bifrost.send(stream.agent, "lua", "APPEND", stream.streamId, msg.body, msg.id, sn)
        bifrost.log("info", string.format("append data [%d]: %s", #msg.body, stream.streamId))
    end
end

local function newStream(streamId, msg)
    local caddr = msg.id
    local contentType = msg.field.contentType
    local sn = msg.field and msg.field.sn
    local errMsg
    local tmp = {}
    string.gsub(streamId, "[^:]+", function(w) table.insert(tmp, w) end)
    local appId, deviceId, token, hook = table.unpack(tmp, 1, 4)
    repeat
        local did = appId .. deviceId
        local lock = deviceLock[did]
        if not lock then
            lock = bifrost.seqlock()
            deviceLock[did] = lock
        end
        local pass = lock(function()
            local deviceCnt = bifrost.call(".statistic", "cmd", "getDeviceCnt",
                                           string.pack("zz", appId, deviceId))
            deviceLock[did] = nil
            local maxOpeningStream = env.optnumber("maxOpeningStream", 10)
            if deviceCnt >= maxOpeningStream then
                errMsg = berror.getError("REQ", "too many opening streams")
                return false
            end
            return true
        end)
        if not pass then
            break
        end
        local stream = idStream[streamId] -- check it again
        if stream then
            return stream, false
        end
        if contentType ~= "text/plain" then
            errMsg = berror.getError("REQ", "1st pack should be text/plain")
            break
        end
        if not msg.body then
            errMsg = berror.getError("REQ", "empty body for 1st pack", caddr)
            break
        end
        local body = json.decode(msg.body)
        if not body then
            errMsg = berror.getError("REQ", "1st pack is not json", caddr)
            break
        end
        local param = body.param
        if not param then
            errMsg = berror.getError("REQ", "missing param", caddr)
            break
        end
        local request = param.request
        if not request then
            errMsg = berror.getError("REQ", "missing request", caddr)
            break
        end
        local coreType = request.coreType
        if not coreType then
            errMsg = berror.getError("REQ", "missing coreType", caddr)
            break
        end
        local agent = service.getAgent(coreType)
        if not agent then
            errMsg = berror.getError("REQ", "invalid coreType or iType", caddr)
            break
        end
        if request.ebnfRes == nil then
            request.ebnfRes = ebnfDefaultByAppId[appId]
        end
        bifrost.send(agent, "lua", "START", streamId, param, caddr, sn)
        bifrost.log("info", "stream start: " .. streamId)
        bifrost.send(".statistic", "lua", {
                        action = "newStream",
                        streamId = streamId,
                        service = coreType,
                     })
        return {
            streamId = streamId,
            agent = agent,
            process = streamAppend,
            coreType = coreType,
        }, true
    until true
    local now = bifrost.epoch()
    local ret = xfer.xfer("POST", "/api/v1.0/log/error", {
            addr = UPLOAD_POOL_ID,
            field = {["Content-Type"]="text/plain",host=env.optstring("uploadAddr", "")},
            body = json.encode {
                finishTime = now,
                initTime = now,
                initPack = contentType == "text/plain" and msg.body or contentType,
                isInit = false,
                error = errMsg,
                appId = appId,
                deviceId = deviceId,
            },
        })
    if not ret then bifrost.debug "xfer error" end
    berror.redirectError(errMsg, caddr, hook, sn)
end

local function verifyIp(appId, ip)
    if appId == "qdreamer_test" then
        return true
    end
    local ips = Wlist[appId]
    
    if not ips then
        ips = bifrost.call(".wlist", "cmd", appId, "")
        if not ips then return false end
    end
    local expire = ips[ip]
    if not expire then
        expire = ips["0.0.0.0"]
    end
    if not expire then return false end
    local now = bifrost.epoch()
    if expire * 1000 > now then
        return true
    end
    return false
end

bifrost.dispatch("lua", function(session, source, upi)
    UPLOAD_POOL_ID = upi
    bifrost.dispatch("lua", function(session, source, msg)
        local hook = msg.field and msg.field.hook
        local sn = msg.field and msg.field.sn
        if not hook then
            berror.sendError("REQ", "Missing hook", msg.id, sn)
            return
        end
        local _, appId, deviceId = sepString(msg.url, "/", 3)
        if not appId or not deviceId then
            berror.sendError("REQ", "Invalid url " .. msg.url, msg.id, hook, sn)
            return
        end
        local ip = sepString(msg.addr, ":", 2)
        if not verifyIp(appId, ip) then
            berror.sendError("AUTH", "Unkown Host " .. ip, msg.id, hook, sn)
            return
        end
        local streamId = table.concat({appId, deviceId, "proxy", hook}, ":")
        local stream = idStream[streamId]
        if not stream then
            stream = newStream(streamId, msg)
            if not stream then return end
            idStream[streamId] = stream
        else
            stream.process(stream, msg)
        end
    end)
end)

local cmder = {}
function cmder.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function cmder.DONE(streamId)
    bifrost.log("info", "stream done: " .. streamId)
    local stream = idStream[streamId]
    idStream[streamId] = nil
    bifrost.send(".statistic", "lua", {
                    action = "deleteStream",
                    streamId = streamId,
                    service = stream.coreType,
                })
end

function cmder.UpdateWlist(list)
    Wlist = json.decode(list)
end

bifrost.dispatch("cmd", function(session, source, cmd, param)
    local f = cmder[cmd]
    if f then f(param) else bifrost.debug("unknow cmd", cmd) end
end)

bifrost.start(function()
end)
