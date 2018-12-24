local bifrost = require "bifrost"
local xfer = require "xfer.core"
local service = require "service"
local ttsService = require "tts.service"
local json = require "dkjson"
local list = require "list"
local berror = require "berror"

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
    stream.timeStamp = bifrost.now()
    orderStream:tailinsert(stream)
    stream.outsideReset = function()
        idStream[streamId] = nil
        orderStream:remove(stream)
    end
    local request = param.request
    if not request.text then
        stream.err(berror.getError("REQUEST", "tts missing text"))
        return
    end
    local coreType = request.coreType
    local startTopic = "/start/" .. ttsService[coreType]
    -- if coreType == "en.tts" then
    --     request.speed = nil
    --     request.pitch = nil
    --     request.volume = nil
    -- end
    local param = {
        text = request.text,
        volume = request.volume or 1.0,
        speed = request.speed or 1.0,
        pitch = request.pitch or 1.3,
        useStream = request.useStream ~= 0,
        rate = request.rate,
        spectrum = request.spectrum,
    }
    local args = {
        RES = request.res or ttsService.defaultRes[coreType],
        PARAM = json.encode(param),
        WTOPIC = stream.WTOPIC,
        ETOPIC = stream.ETOPIC,
        APPEND0 = request.useStream ~=0,
    }
    local body = string.pack("zz", service.getScript(ttsService.templates[coreType], args), args.ETOPIC)
    if not xfer.xfer("PUT", startTopic, {addr=MSG_SOCK_ID, body=body}) then
        bifrost.debug "xfer error"
    end
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
function cmder.ERROR(param)
    local streamId, body = string.unpack("zs", param)
    local stream = idStream[streamId]
    if not stream then
        bifrost.debug("stream not found", streamId)
        return
    end
    stream.err(body)
end

function cmder.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function cmder.RESPONSE(param)
    local streamId, _, body = string.unpack("zzsz", param)
    local stream = idStream[streamId]
    if not stream then
        bifrost.debug("stream not found", streamId)
        return
    end
    if stream.param.request.useStream ~= 0 then
        if #body > 0 then
            stream.rep(body, "audio/mpeg", 100)
        else
            if not stream.eof then
                stream.eof = true
                stream.rep(body, "audio/mpeg", 200)
            end
        end
    else  -- eq 0 means don't useStream
        if not stream.eof then
            stream.eof = true
            stream.rep(body, "audio/mpeg", 200)
        end
    end
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
        if not stream or now - stream.timeStamp < 1000 then break end
        stream.err(berror.getError("TIMEOUT", "stream timeout"))
    end
    bifrost.timeout(100, cleanTimeoutStream)
end

bifrost.start(function()
    bifrost.dispatch("lua", function(session, source, ...)
        MSG_SOCK_ID, UPLOAD_POOL_ID, SELF = ...
        service.init(MSG_SOCK_ID, SELF)
        bifrost.dispatch("lua", luaDispatcher)
        service.init(MSG_SOCK_ID, SELF, UPLOAD_POOL_ID)
    end)
    bifrost.dispatch("cmd", cmdDispatcher)
    bifrost.timeout(100, cleanTimeoutStream)
end)
