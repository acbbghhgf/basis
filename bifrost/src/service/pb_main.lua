local bifrost = require "bifrost"
local json = require "dkjson"
local service = require "service"
local payload = require "payload"
local xfer = require "xfer.core"
local berror = require "berror"
local env = require "env"

local UPLOAD_POOL_ID
local idDeviceInfo = {} -- deviceInfo format {appId, deviceId, wsi, iat}
local streams = {} -- streamId --> {agent, id, coreType}

local function getDeviceInfo(pack)
    local extra = pack.extra
    local deviceInfo = idDeviceInfo[pack.id]
    local appId, deviceId = table.unpack(deviceInfo)
    local serverAllow = deviceInfo[5]
    if not serverAllow or type(serverAllow) ~= 'table' then
        serverAllow = nil
    end
    if not deviceId then
        deviceId = extra.deviceId
        deviceInfo[2] = deviceId
    end
    if not appId then
        appId = extra.appId
        deviceInfo[1] = appId
    end
    local iat = deviceInfo[4] or 0
    return appId, deviceId, serverAllow, iat
end

local function getStreamId(pack)
    local extra = pack.extra
    if not extra then
        return false, berror.getError("REQ", "missing extra")
    end
    local hook = extra.hook
    if not hook then
        return false, berror.getError("REQ", "missing hook")
    end
    local appId, deviceId, serverAllow, iat = getDeviceInfo(pack)
    if not appId or not deviceId then
        return false, berror.getError("REQ", "missing appId or deviceId")
    end
    return true, table.concat({appId, deviceId, iat, hook}, ":"), serverAllow
end

local function setWsi(pack, wsi)
    local deviceInfo = idDeviceInfo[pack.id]
    if not deviceInfo[3] then
        deviceInfo[3] = wsi
    end
end

local function doError(id, errMsg)
    local deviceInfo = idDeviceInfo[id]
    if not deviceInfo then
        return
    end
    local wsi = deviceInfo[3]
    if not wsi then
        bifrost.debug("Should not be here")
        return
    end
    local v = payload.packResp {
        type = "ERROR",
        bin  = errMsg,
        isEnd = true,
    }
    xfer.sendPB(v, id, wsi)
end

local function doResp(id, type, bin, dlg, is_end)
    local deviceInfo = idDeviceInfo[id]
    if not deviceInfo then
        return
    end
    local wsi = deviceInfo[3]
    if not wsi then
        bifrost.debug("Should not be here")
        return
    end
    local v = payload.packResp {
        type = type,
        bin  = bin,
        dlg = dlg,
        isEnd = is_end ~= 0,
    }
    xfer.sendPB(v, id, wsi)
end

local actHandler = {}
function actHandler.START(session, source, pack, wsi)
    setWsi(pack, wsi)
    local errMsg
    repeat
        local succ, ret, serverAllow = getStreamId(pack)
        if not succ then
            errMsg = ret
            break
        end
        local streamId = ret
        local coreType = pack.obj
        if not coreType then
            errMsg = berror.getError("REQ", "missing coreType")
            break
        end
        local agent = service.getAgent(coreType)
        if not coreType then
            errMsg = berror.getError("REQ", "invalid coreType")
            break
        end
        local request
        if pack.desc and pack.desc.param then
            request = pack.desc.param
        else
            request = {}
        end
        request.coreType = coreType
        if request.useStream == false then
            request.useStream = 0
        end
        audio = pack.desc and pack.desc.audio
        bifrost.send(agent, "lua", "START", streamId, {
                        request = request,
                        audio = audio,
                     })
        streams[streamId] = {agent, pack.id, coreType}
        bifrost.send(".statistic", "lua", {
                        action = "newStream",
                        streamId = streamId,
                        service = coreType,
                     })
        return
    until true
    local now = bifrost.epoch()
    local ret = xfer.xfer("POST", "/api/v1.0/log/error", {
            addr = UPLOAD_POOL_ID,
            field = {["Content-Type"]="text/plain",host=env.optstring("uploadAddr", "")},
            body = json.encode {
                finishTime = now,
                initTime = now,
                initPack = json.encode(pack),
                isInit = false,
                error = errMsg,
                appId = appId,
                deviceId = deviceId,
            },
        })
    if not ret then bifrost.debug "xfer error" end
    doError(pack.id, errMsg)
end

local function getStreamAgent(pack, streamId)
    local stream = streams[streamId]
    if not stream then return end
    stream[2] = pack.id
    return stream[1]
end

function actHandler.APPEND(session, source, pack, wsi)
    setWsi(pack, wsi)
    local succ, ret = getStreamId(pack)
    if not succ then
        return doError(pack.id, ret)
    end
    local streamId = ret
    local agent = getStreamAgent(pack, streamId)
    if not agent then
        return doError(pack.id, "Unknown append action")
    end
    local audio = pack.data
    if not audio or #audio == 0 then
        return doError(pack.id, berror.getError("REQ", "missing audio"))
    end
    bifrost.send(agent, "lua", "APPEND", ret, audio)
end

function actHandler.END(session, source, pack, wsi)
    setWsi(pack, wsi)
    local succ, ret = getStreamId(pack)
    if not succ then
        return doError(pack.id, ret)
    end
    local agent = getStreamAgent(pack, ret)
    if not agent then
        return doError(pack.id, "Unknown append action")
    end
    bifrost.send(agent, "lua", "APPEND", ret)
end

function actHandler.UNKNOWN(session, source, pack)
    bifrost.debug("Should Not Be Here")
end

bifrost.dispatch("lua", function(_, _, upl)
    UPLOAD_POOL_ID = upl
    bifrost.dispatch("lua", function(session, source, pack, param)
        if type(pack) == "table" then
            local act = pack.act
            assert(act ~= nil)
            local handler = actHandler[act]
            assert(handler ~= nil)
            handler(session, source, pack, param)
        else
            assert(type(pack) == "number")  -- It Must Be JWT Payload
            local id = pack
            local payload = json.decode(param)
            if payload then
                idDeviceInfo[id] = {payload.aid, payload.did, nil, payload.iat, payload.service}
            end
        end
    end)
end)

local cmder = {}

function cmder.DISCONNECT(id)
    local id = tonumber(id)
    idDeviceInfo[id] = nil
end

function cmder.ECONNECT(id)
    cmder.DISCONNECT(id)
end

function cmder.RESPONSE(param)
    local streamId, contentType, content, lua, is_end = string.unpack("zzszH", param)
    local stream = streams[streamId]
    if not stream then
        return bifrost.debug("missing stream ", streamId)
    end
    if #content == 0 then
        return doResp(stream[2], contentType, nil, nil, 1)
    end
    doResp(stream[2], contentType, content, #lua > 0 and lua or nil, is_end)
end

function cmder.ERROR(param)
    local streamId, err = string.unpack("zs", param)
    local stream = streams[streamId]
    if not stream then
        return bifrost.debug("missing stream ", streamId)
    end
    doError(stream[2], err)
end

function cmder.DONE(param)
    local stream = streams[param]
    if stream then
        bifrost.send(".statistic", "lua", {
                action = "deleteStream",
                streamId = param,
                service = stream[3],
            })
        streams[param] = nil
    end
end

bifrost.dispatch("cmd", function(session, source, cmd, param)
    local f = cmder[cmd]
    if f then f(param) else bifrost.debug("Unknown cmd ", cmd) end
end)

bifrost.start(function()
end)
