local bifrost = require "bifrost"
local service = require "service"
local bfioService = require "bfio.service"
local list = require "list"
local berror = require "berror"
local json = require "dkjson"
local xfer = require "xfer.core"
local msgpack = require "cmsgpack"

local MSG_SOCK_ID
local UPLOAD_POOL_ID
local SELF

local idStream = {}
local orderStream = list.new()

local actor = {}

function actor.START(source, streamId, param, caddr, sn)
    local stream = idStream[streamId]
    if stream then
        bifrost.debug(streamId, "duplicate")
        xfer.xfer("DELETE", stream.RTOPIC, {addr=MSG_SOCK_ID})
        stream.err(berror.getError("UNFINISH", "duplicate streamId"))
        return
    end
    stream = service.getStream(streamId, param, source, caddr, sn)
    idStream[streamId] = stream
    stream.audio = {}
    stream.timeStamp = bifrost.now()
    orderStream:tailinsert(stream)
    stream.outsideReset = function()
        idStream[streamId] = nil
        orderStream:remove(stream)
    end
    local request = param.request
    local coreType = request.coreType
    local startTopic = "/start/" .. bfioService[coreType]
    local args = {
        WTOPIC = stream.WTOPIC,
        RTOPIC = stream.RTOPIC,
        ETOPIC = stream.ETOPIC,
        audio = param.audio,
    }
    local body = string.pack("zz", json.encode(args), args.ETOPIC)
    if not xfer.xfer("PUT", startTopic, {addr=MSG_SOCK_ID, body=body}) then
        bifrost.debug "xfer error"
    end
end

function actor.APPEND(_, streamId, body, caddr, sn)
    local stream = idStream[streamId]
    if not stream then bifrost.debug("stream not found", streamId) return end
    service.updateStream(stream, caddr, sn)
    if not xfer.xfer("PUT", stream.RTOPIC, {addr=MSG_SOCK_ID, body=body}) then
        bifrost.debug "xfer error"
    end
    stream.timeStamp = bifrost.now()
    if body then
        local audio = stream.audio
        audio[#audio+1] = body
    end
    orderStream:movetail(stream)
end

function actor.FLUSH(_, streamId)
    local stream = idStream[streamId]
    if stream then
        xfer.xfer("DELETE", stream.RTOPIC, {addr=MSG_SOCK_ID})
        bifrost.log("warning", "DELETE stream: " .. streamId)
        idStream[streamId] = nil
        stream.reset("flushed")
    end
end

local function luaDispatcher(session, source, action, streamId, ...)
    local f = actor[action]
    if not f then bifrost.debug("Unknow Action", action) return end
    f(source, streamId, ...)
end

local cmder = {}
function cmder.RESPONSE(param)
    local streamId, _, body = string.unpack("zzs", param)
    local stream = idStream[streamId]
    if not stream then bifrost.debug "stream not found" return end
    if body and #body > 0 then
        stream.rep(json.encode(msgpack.unpack(body)), "text/plain", 100)
        return
    end
    stream.eof = true
    stream.rep(nil, "text/plain")
end

function cmder.ERROR(param)
    local streamId, body = string.unpack("zs", param)
    local stream = idStream[streamId]
    if not stream then bifrost.debug "stream not found" return end
    stream.err(body)
end

local function cmdDispatcher(session, source, cmd, param)
    local f = cmder[cmd]
    if not f then bifrost.debug("Unknow Cmd", cmd) end
    f(param)
end

local function cleanTimeoutStream()
    local now = bifrost.now()
    while true do
        local stream = orderStream:touchhead()
        if not stream or now - stream.timeStamp < 3000 then break end
        stream.err(berror.getError("TIMEOUT", "stream timeout"))
    end
    bifrost.timeout(100, cleanTimeoutStream)
end

bifrost.start(function()
    bifrost.dispatch("lua", function(session, source, ...)
        MSG_SOCK_ID, UPLOAD_POOL_ID, SELF = ...
        service.init(MSG_SOCK_ID, SELF, UPLOAD_POOL_ID)
        bifrost.dispatch("lua", luaDispatcher)
    end)
    bifrost.dispatch("cmd", cmdDispatcher)
    bifrost.timeout(100, cleanTimeoutStream)
end)