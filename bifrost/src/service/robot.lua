local bifrost = require "bifrost"
local service = require "service"
local berror = require "berror"
local xfer = require "xfer.core"
local robotService = require "robot.service"
local json = require "dkjson"
local list = require "list"

local MSG_SOCK_ID
local UPLOAD_POOL_ID
local SELF

local idStream = {}
local orderStream = list.new()

local function startDlg(stream, text)
    local request = stream.param.request
    local semRes = request.semRes == "xiaohe" and "yuki" or request.semRes
    if request.useJson then
        local asr_result = json.decode(request.text)
        if asr_result then
            request.text = asr_result.rec
        end
    end
    local args = {
        CFN = semRes or "yuki",
        WTOPIC = stream.WTOPIC,
        ETOPIC = stream.ETOPIC,
        PARAM = json.encode {
            clientId = stream.appId .. stream.deviceId,
            input = text or request.text,
            env = request.env,
        }
    }
    local template = robotService.templates["dlg"]
    local body = string.pack("zz", service.getScript(template, args), args.ETOPIC)
    if not xfer.xfer("PUT", "/start/dlgv2.0", {addr=MSG_SOCK_ID, body=body}) then
        bifrost.debug "xfer error"
    end
    stream.status = "WAIT_DLG_RESULT"
end

local function startTts(stream, text)
    local request = stream.param.request
    local param = {
        request = {
            coreType = "cn.tts",
            text = text,
            volume = request.volume,
            pitch = request.pitch,
            speed = request.speed,
            useStream = request.useStream,
            res = request.synRes,
        },
    }
    local agent = service.getAgent("cn.tts")
    bifrost.send(agent, "lua", "START", stream.streamId .. "|tts", param, nil, stream.sn)
    stream.status = "WAIT_TTS_RESULT"
end

local function ttsWaiter(stream, body)
    local useStream = stream.param.request.useStream ~= 0
    if useStream then
        if #body > 0 then
            stream.rep(body, "audio/mpeg", 100)
        else
            if not stream.eof then
                stream.eof = true
                stream.rep(body, "audio/mpeg", 200)
            end
        end
    else
        if not stream.eof then
            stream.eof = true
            stream.rep(body, "audio/mpeg", 200)
        end
    end
end

local function handleDlgResult(stream, result)
    local jResult = json.decode(result)
    if not jResult then
        stream.err(berror.getError("REP", "dlg result not json"))
        return
    end
    stream.dlgResult = result
    stream.dlgInput = jResult.input
    stream.dlgOutput = jResult.output
    stream.dlgFld = jResult.fld
    return jResult
end

local function dlgWaiter(stream, body)
    local result = handleDlgResult(stream, body)
    if not result then return end
    local iType = stream.param.request.iType
    if iType == 2 or iType == 3 then
        stream.eof = true
        stream.rep(body, "text/plain", 200)
    else
        local text = result.output
        if not text or #text == 0 then
            stream.err(berror.getError("REP", "dlg result empty output"))
            return
        end
        startTts(stream, text)
        stream.repHandler = ttsWaiter
    end
end

local function asrWaiter(stream, body)
    local iType = stream.param.request.iType
    if stream.status == "ASR_APPEND" then
        stream.err(berror.getError("REP", "unexpected response, still appending data"))
        return
    end
    local jResult = json.decode(body)
    if not jResult.rec or #jResult.rec == 0 then
        stream.eof = true
        stream.rep(body, "text/plain", 200)
        return
    end
    startDlg(stream, jResult.rec)
    stream.repHandler = dlgWaiter
end

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
    stream.caddr = caddr
    stream.timeStamp = bifrost.now()
    orderStream:tailinsert(stream)
    idStream[streamId] = stream
    stream.outsideReset = function()
        idStream[streamId] = nil
        orderStream:remove(stream)
    end
    local request = param.request
    local iType = request.iType
    stream.status = "INIT"
    if iType == 1 or iType == 2 then
        local agent = service.getAgent("cn.asr.rec")
        bifrost.send(agent, "lua", "START", streamId .. "|asr", {
                request = {
                    coreType = "cn.asr.rec",
                    res = request.recRes or "comm",
                    env = request.env,
                    stripSpace = request.stripSpace,
                },
                audio = param.audio,
            }, nil, sn)
        stream.status = "ASR_APPEND"
        stream.repHandler = asrWaiter
        stream.asrAgent = agent
    elseif iType == 3 or iType == 4 then
        if not request.text then
            stream.err(berror.getError("REQ"))
            return
        end
        startDlg(stream)
        stream.repHandler = dlgWaiter
    else
        stream.err(berror.getError("REQ", "unknow iType"))
    end
end

function actor.APPEND(_, streamId, body, addr, sn)
    local stream = idStream[streamId]
    if not stream then bifrost.debug("stream not found", streamId) return end
    service.updateStream(stream, addr, sn)
    if stream.status ~= "ASR_APPEND" then
        stream.err(berror.getError("REQ", "unexpected request"))
        return
    end
    if not body or #body == 0 then stream.status = "WAIT_ASR_RESULT" end
    bifrost.send(stream.asrAgent, "lua", "APPEND", streamId .. "|asr", body, addr, sn)
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
    if not stream then bifrost.debug "stream not found" return end
    stream.err(body)
end

function cmder.RESPONSE(param)
    local streamId, _, body = string.unpack("zzsz", param)
    local sep = string.find(streamId, "|")
    if sep then
        streamId = string.sub(streamId, 1, sep - 1)
    end
    local stream = idStream[streamId]
    if not stream then bifrost.debug "stream not found" return end
    stream.repHandler(stream, body)
end

function cmder.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function cmder.DONE(streamId)
end

local function cmdDispatcher(session, source, cmd, param)
    local f = cmder[cmd]
    if not f then bifrost.debug("Unknow Cmd", cmd) return end
    f(param)
end

local function lengthDebug()
    bifrost.debug(bifrost.tableLength(idStream))
    bifrost.timeout(100, lengthDebug)
end

local function cleanTimeoutStream()
    local now = bifrost.now()
    while true do
        local stream = orderStream:touchhead()
        if not stream or now - stream.timeStamp < 6000 then break end
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
    --bifrost.timeout(100, lengthDebug)
end)
