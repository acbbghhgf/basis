local bifrost = require "bifrost"
local json = require "dkjson"
local berror = require "berror"
local xfer = require "xfer.core"
local env = require "env"
local service = require "service"

local addrInfo = {}
local tokenInfo = {}
local addrLock = {}

local idStream = {}
local addrStreamIds = {}

local UPLOAD_POOL_ID

local cmder = {}

-- will remove later
local ebnfDefaultByAppId = {
    Gowild = "xiaobai",
    ["188569ba-987b-11e7-b526-00163e13c8a2"] = "xiaobai",
    ["86ff94ba-4447-11e8-b526-00163e13c8a2"] = "8550613c4f0fb9dcdf0ca8ade444e613",
}

function cmder.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function cmder.DISABLEID(id)
    local id = tonumber(id)
    local streamIds = addrStreamIds[id]
    if streamIds then
        for _, streamId in ipairs(streamIds) do
            local stream = idStream[streamId]
            if stream then
                bifrost.send(stream.agent, "lua", "FLUSH", streamId)
            end
        end
        addrStreamIds[id] = nil
    end
    addrInfo[id] = nil
end

function cmder.DISABLETOKEN(token)
    tokenInfo[token] = nil
end

function cmder.DONE(streamId)
    bifrost.log("info", "stream done: " .. streamId)
    local stream = idStream[streamId]
    local caddr = stream.caddr
    local streamIds = addrStreamIds[caddr]
    if streamIds then
        for idx, tmp in ipairs(streamIds) do
            if tmp == streamId then
                table.remove(streamIds, idx)
            end
        end
    end
    idStream[streamId] = nil
    bifrost.send(".statistic", "lua", {
                    action = "deleteStream",
                    streamId = streamId,
                    service = stream.coreType,
                })
end

bifrost.dispatch("cmd", function(session, source, cmd, param)
    local f = cmder[cmd]
    if f then f(param) else bifrost.debug("unknown cmd", cmd) end
end)

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
        bifrost.log("info", "append stop: " .. stream.streamId)
        bifrost.send(stream.agent, "lua", "APPEND", stream.streamId, nil, msg.id, nil)
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
        local lock = addrLock[msg.id]
        if not lock then
            lock = bifrost.seqlock()
            addrLock[msg.id] = lock
        end
        local pass = lock(function()
            local deviceCnt = bifrost.call(".statistic", "cmd", "getDeviceCnt",
                                           string.pack("zz", appId, deviceId))
            addrLock[msg.id] = nil
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
            caddr = msg.id,
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

bifrost.dispatch("lua", function(session, source, upl)
    UPLOAD_POOL_ID = upl
    bifrost.dispatch("lua", function(session, source, msg)
        local hook = msg.field and msg.field.hook
        local sn = msg.field and msg.field.sn
        if not hook then
            berror.sendError("REQ", "request no hook", msg.id, sn)
            return
        end
        local token = msg.field.token
        local info = token and tokenInfo[token] or addrInfo[msg.id]
        if not info then
            local lock = addrLock[msg.id]
            if not lock then
                lock = bifrost.seqlock()
                addrLock[msg.id] = lock
            end
            lock(function()
                local isPass, sessionInfo = bifrost.call(".session", "cmd", "GETSESSION", token or msg.id)
                addrLock[msg.id] = nil
                if not isPass then
                    berror.sendError("AUTH", sessionInfo, msg.id, hook, sn)
                    return
                end
                info = {
                    sessionId = sessionInfo.sessionId,
                    token = sessionInfo.token,
                }
                addrInfo[msg.id] = info
                if token then
                    tokenInfo[token] = info
                end
            end)
        end
        if not info then return end
        local sessionId = info.sessionId
        local appId, deviceId = bifrost.unpackSessionId(sessionId)
        local streamId = table.concat({appId, deviceId, info.token, hook}, ":")
        local stream = idStream[streamId]
        if not stream then
            stream, new = newStream(streamId, msg)
            if not stream then return end
            if new then
                idStream[streamId] = stream
                local streamIds = addrStreamIds[msg.id]
                if not streamIds then
                    streamIds = {}
                    addrStreamIds[msg.id] = streamIds
                end
                table.insert(streamIds, streamId)
                bifrost.send(".session", "cmd", "TOUCH", sessionId)
                return
            end
        end
        stream.process(stream, msg)
    end)
end)

bifrost.start(function()
end)
