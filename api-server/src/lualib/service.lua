local bifrost = require "bifrost"
local json = require "dkjson"
local xfer = require "xfer.core"
local env = require "env"
local berror = require "berror"

local service = {}

local function render(key, t)
    local value = t[key]
    if type(value) == "string" then return string.format("%q", value) end
    return tostring(value)
end

function service.getScript(template, t)
    return (string.gsub(template, "%$([%w_%d]+)", function(key) return render(key, t) end))
end

local serviceTable = {
    ["cn.tts"]      = "tts",
    ["en.tts"]      = "tts",
    ["cn.robot"]    = "robot",
    ["cn.asr.rec"]  = "asr",
    ["en.asr.rec"]  = "asr",
    ["en.eval.snt"] = "asr",
    ["en.eval.wrd"] = "asr",
    ["cn.usc.rec"]  = "asr",
    ["cn.asr.mix"]  = "asr",
    ["bfio"]        = "bfio",
}

function service.getAgent(coreType)
    local service = serviceTable[coreType]
    if not service then return end
    local cnt = env.optnumber(service .. "Cnt", 1)
    local name = service .. tostring(math.random(cnt))
    local agent = bifrost.queryHandle("." .. name)
    return bifrost.queryHandle("." .. name), name
end

function service.init(MSG_SOCK_ID, SELF, UPLOAD_POOL_ID)
    service.MSG_SOCK_ID = MSG_SOCK_ID
    service.SELF = SELF
    service.UPLOAD_POOL_ID = UPLOAD_POOL_ID
end

local function asrUpload(stream)
    bifrost.upload ({
        type = "infoData",
        data = table.concat(stream.audio),
        info = json.encode {
            base_info = {
                device_id = stream.deviceId,
                app_id = stream.appId,
                time = stream.initTime,
            },
            record_info = {
                id = stream.streamId,
                type = stream.param.audio.audioType,
                details = stream.asrDetail,
                output = stream.asrOutput,
            },
        },
    }, env.optstring("uploadAddr", ""), service.UPLOAD_POOL_ID)
end

local function dlgUpload(stream)
    bifrost.upload ({
        type = "infoData",
        info = json.encode {
            base_info = {
                device_id = stream.deviceId,
                app_id = stream.appId,
                time = stream.initTime,
            },
            dialog_info = {
                input = stream.dlgInput,
                output = stream.dlgOutput,
                fld = stream.dlgFld,
                details = stream.dlgResult,
            },
        },
    }, env.optstring("uploadAddr", ""), service.UPLOAD_POOL_ID)
end

local uploadTable = {
    ["cn.asr.rec"]  = asrUpload,
    ["en.asr.rec"]  = asrUpload,
    ["en.eval.snt"] = asrUpload,
    ["en.eval.wrd"] = asrUpload,
    ["cn.usc.rec"]  = asrUpload,
    ["cn.robot"]    = dlgUpload,
}

local function streamReset(stream, source)
    local content = json.encode{stream.ETOPIC, stream.WTOPIC}
    if not xfer.xfer("PUT", "/_unwatch", {body=content, addr=service.MSG_SOCK_ID}) then
        bifrost.debug("xfer error")
    end
    bifrost.send(source, "cmd", "DONE", stream.streamId)
    if stream.outsideReset then stream.outsideReset() end
    local uploader = uploadTable[stream.param.request.coreType]
    if uploader then uploader(stream) end
    if stream.errMsg then
        local ret = xfer.xfer("POST", "/api/v1.0/log/error", {
                addr = service.UPLOAD_POOL_ID,
                field = {["Content-Type"]="text/plain",host=env.optstring("uploadAddr", "")},
                body = json.encode{ initTime   = stream.initTime,
                    finishTime = bifrost.epoch(),
                    initPack   = stream.param,
                    isInit     = true,
                    error      = stream.errMsg,
                    appId      = stream.appId,
                deviceId   = stream.deviceId },
            })
        if not ret then bifrost.debug("xfer error") end
        bifrost.log("error", string.format("appId:%s,deviceId:%s,hook:%s,err:%s",
                    stream.appId, stream.deviceId, stream.hook, stream.errMsg))
    else
        local ret = xfer.xfer("POST", "/api/v1.0/log/result", {
                addr = service.UPLOAD_POOL_ID,
                field = {["Content-Type"]="text/plain",host=env.optstring("uploadAddr", "")},
                body = json.encode{ initTime   = stream.initTime,
                    finishTime = bifrost.epoch(),
                    initPack   = stream.param,
                    isInit     = true,
                    result     = stream.result,
                    appId      = stream.appId,
                    deviceId   = stream.deviceId },
            })
        if not ret then bifrost.debug("xfer error") end
        bifrost.log("info", json.encode{
                result = stream.result,
                request_info = stream.param,
                applicationId = stream.appId,
                deviceId = stream.deviceId,
                hook = stream.hook,
            })
    end
end

function service.getStream(streamId, initParam, source, caddr, sn)
    local tmp = {}
    string.gsub(streamId, "[^:]+", function(w) table.insert(tmp, w) end)
    local appId, deviceId, token, hook = table.unpack(tmp, 1, 4)
    local prefix = "/" .. service.SELF .. "/" .. streamId
    local stream = {
        streamId = streamId,
        appId = appId,
        deviceId = deviceId,
        token = token,
        hook = hook,
        param  = initParam,
        initTime = bifrost.epoch(),
        WTOPIC = prefix .. "/rep",
        RTOPIC = prefix .. "/data",
        ETOPIC = prefix .. "/err",
        eof = false,
        sn = sn,
        caddr = caddr,
    }

    if caddr then
        stream.rep = function(body, contentType, status)
            local repStatus = status or 200
            local ret = xfer.xfer(repStatus, { addr  = stream.caddr,
                                               field = { ["Content-Type"] = contentType,
                                                                     hook = hook,
                                                                      lua = stream.dlgResult,
                                                               ["seq-no"] = stream.sn,
                                                                   device = deviceId },
                                               body  = body})
            stream.dlgResult = nil
            if not ret then bifrost.debug "xfer error" end
            if contentType == "text/plain" then stream.result = body end
            if stream.eof then streamReset(stream, source) end
        end
        stream.err = function(errMsg)
            local ret = berror.redirectError(errMsg, stream.caddr, hook, deviceId, sn)
            if not ret then bifrost.debug "xfer error" end
            stream.errMsg = errMsg
            streamReset(stream, source)
        end
    else
        stream.rep = function(body, contentType, status)
            if contentType == "text/plain" then stream.result = body end
            local is_end = status ~= 200 and status ~= nil and 0 or 1
            bifrost.send(source, "cmd", "RESPONSE", string.pack("zzszH",
                         streamId, contentType, body or "", stream.dlgResult or "", is_end))
            stream.dlgResult = nil
            if stream.eof then streamReset(stream, source) end
        end
        stream.err = function(errMsg)
            bifrost.send(source, "cmd", "ERROR", string.pack("zs", streamId, errMsg or ""))
            stream.errMsg = errMsg
            streamReset(stream, source)
        end
    end

    stream.reset = function(reason)
        stream.errMsg = "stream reset: " .. tostring(reason)
        streamReset(stream, source)
    end

    local body = json.encode{[stream.ETOPIC] = "",[stream.WTOPIC] = ""}
    if not xfer.xfer("PUT", "/_watch", {addr=service.MSG_SOCK_ID, body=body}) then
        bifrost.debug "xfer error"
    end
    return stream
end

function service.updateStream(stream, caddr, sn)
    stream.caddr = caddr
    stream.sn = sn
end

return service
