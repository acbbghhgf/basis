local bifrost = require "bifrost"
local json = require "dkjson"
local xfer = require "xfer.core"
local env = require "env"
local md5 = require "md5.core"
local list = require "list"
local berror = require "berror"
local set = require "set"

local hookCinfo = {}
local orderAuth = list.new()
local cachedSession = list.new()
local idResult = {}

local tokenSessionId = {}
local addrSessionId = {}

local AUTH_POOL_ID

local function urlEncode(s)
    return string.gsub(s, "+", "%%2b")
end

local authHost = env.optstring("authAddr", "192.168.0.126:9003")

local function sendAuthReply(caddr, clientHook, isPass, body)
    local ret = xfer.xfer(isPass and 200 or 500, {
                              addr = caddr,
                              body = body,
                              field = {
                                  ["Content-Type"] = "text/plain",
                                  hook = clientHook,
                              },
                          })
    if not ret then bifrost.debug "xfer error" end
end

local command = {}
function command.GETSESSION(_, param)
    local caddr = tonumber(param)
    if not caddr then -- token
        local sessionId = tokenSessionId[param]
        if sessionId then
            bifrost.ret(true, {sessionId = sessionId, token = param})
        else
            bifrost.ret(false, "Invalid Token")
        end
        return
    end
    local sessionId = addrSessionId[caddr]
    if not sessionId then
        bifrost.ret(false, "Unknown client addr")
        return
    end
    local result = idResult[sessionId]
    if result.isPass then
        bifrost.ret(true, {sessionId = sessionId, token = result.token})
    else
        if result.status == "WAITING" then
            result.rep = bifrost.response()
        else
            bifrost.ret(false, "Auth Was Failed")
        end
    end
end

function command.TOUCH(_, sessionId)
    local result = idResult[sessionId]
    if result and result.isCached then
        cachedSession:remove(result)
        result.isCached = nil
        result.timeStamp = nil
    end
end

function command.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function command.ECONNECT(_, id)
    local id = tonumber(id)
    local sessionId = addrSessionId[id]
    if sessionId then
        addrSessionId[id] = nil
        local result = idResult[sessionId]
        local addrSet = result.addrSet
        addrSet:remove(id)
        bifrost.send(".main", "cmd", "DISABLEID", id)
        if addrSet:len() == 0 and (not result.isCached) then
            result.isCached = true
            result.timeStamp = bifrost.now()
            cachedSession:tailinsert(result)
        end
    end
end

function command.DISCONNECT(_, id)
    command.ECONNECT(nil, id)
end

local function cleanTimeoutAuth()
    local now = bifrost.now()
    while true do
        local cinfo = orderAuth:touchhead()
        if not cinfo then break end
        if (now - cinfo.timeStamp < 500) then break end -- timeout 5s for auth
        hookCinfo[cinfo.hook] = nil
        sendAuthReply(cinfo.caddr, cinfo.clientHook, false, "auth timeout")
        orderAuth:remove(cinfo)
        bifrost.log("error", "TERRIBLE BUG, AUTH TIMEOUT")
        local sessionId = cinfo.sessionId
        local result = idResult[sessionId]
        if result and result.status == "WAITING" and result.rep then
            result.rep(true, false, "Auth TIMEOUT")
        end
    end
    bifrost.timeout(100, cleanTimeoutAuth)
end

local function cleanTimeoutSession()
    local now = bifrost.now()
    while true do
        local result = cachedSession:touchhead()
        if not result or (now - result.timeStamp) < 6000 then break end
        cachedSession:remove(result)
        if result.token then
            tokenSessionId[result.token] = nil
            bifrost.send(".main", "cmd", "DISABLETOKEN", result.token)
        end
        bifrost.log("info", "session timeout: " .. result.sessionId)
        local addrSet = result.addrSet
        for caddr in pairs(addrSet) do
            bifrost.send(".main", "cmd", "DISABLEID", caddr)
            addrSessionId[caddr] = nil
            xfer.destroy(caddr)
        end
        idResult[result.sessionId] = nil
    end
    bifrost.timeout(500, cleanTimeoutSession)
end

bifrost.dispatch("lua", function(session, source, cmd, ...)
    assert(AUTH_POOL_ID == nil)
    AUTH_POOL_ID = cmd
    bifrost.dispatch("lua", function(session, source, msg)
        if msg.type == "REQUEST" then       -- process auth request
            local clientHook = msg.field and msg.field.hook
            local sn = msg.field and msg.field.sn
            if not msg.body then
                berror.sendError("AUTH", "missing /auth body", msg.id, clientHook, sn)
                return
            end
            local info = json.decode(msg.body)
            if not (info and info.appId and info.deviceId and info.time and info.sign) then
                berror.sendError("AUTH", "invalid auth body " .. msg.body, msg.id, clientHook, sn)
                return
            end
            local sessionId = bifrost.packSessionId(info.appId, info.deviceId)
            addrSessionId[msg.id] = sessionId
            local result = idResult[sessionId]
            if result then
                if result.isCached then
                    cachedSession:remove(result)
                    result.isCached = nil
                    result.timeStamp = nil
                end
                result.addrSet:add(msg.id)
                if result.status == "WAITING" then
                    berror.sendError("AUTH", "is authing", msg.id, clientHook, sn)
                    return
                end
                if result.isPass then
                    local token = msg.field and msg.field.token
                    if token == result.token then
                        sendAuthReply(msg.id, clientHook, true, json.encode{token = result.token})
                        return
                    end
                end
            else
                idResult[sessionId] = {
                    sessionId = sessionId,
                    status = "WAITING",
                    addrSet = set:new(msg.id),
                }
                bifrost.log("info", "new session: "  .. tostring(sessionId))
            end
            local authPack = string.format("appId=%s&deviceId=%s&time=%s&sign=%s",
                                            info.appId, info.deviceId, info.time, urlEncode(info.sign))
            local hook = tostring(bifrost.genId())
            local cinfo = {
                hook = hook,
                caddr = msg.id,
                clientHook = clientHook,
                timeStamp = bifrost.now(),
                sessionId = sessionId,
            }
            hookCinfo[hook] = cinfo
            orderAuth:tailinsert(cinfo)
            xfer.xfer("POST", "/api/v1/auth/", {
                         addr = AUTH_POOL_ID,
                         body = authPack,
                         field = {
                             ["Content-Type"] = "application/x-www-form-urlencoded",
                             hook = hook,
                             host = authHost,
                         },
                     })
        else                            -- process auth response
            local hook = msg.field and msg.field.hook
            if not hook then
                bifrost.debug("TERRIBLE BUG, AUTH REPLY NO HOOK")
                bifrost.log("error", "TERRIBLE BUG, AUTH REPLY NO HOOK")
                return
            end
            local cinfo = hookCinfo[hook]
            if not cinfo then
                bifrost.debug("TERRIBLE BUG, AUTH REPLY UNKNOWN HOOK")
                bifrost.log("error", "TERRIBLE BUG, AUTH REPLY UNKNOWN HOOK")
                return
            end
            orderAuth:remove(cinfo)
            hookCinfo[msg.field.hook] = nil
            local rep = json.decode(msg.body)
            if rep == nil then
                sendAuthReply(cinfo.caddr, cinfo.clientHook, false, msg.body)
                return
            end
            local sessionId = bifrost.packSessionId(rep.appId, rep.deviceId)
            local res = idResult[sessionId]
            if not res then
                bifrost.debug("unexpected auth reply")
                bifrost.log("error", "unexpected auth reply")
                return
            end
            if rep.code == 0 then
                res.isPass = true
                if res.token then
                    tokenSessionId[res.token] = nil
                end
                local token = md5.sum(rep.sign)
                rep.token = token
                res.token = token
                tokenSessionId[token] = sessionId
            end
            sendAuthReply(cinfo.caddr, cinfo.clientHook, res.isPass, json.encode(rep))
            res.status = nil
            if res.rep then
                if res.isPass then
                    res.rep(true, true, {sessionId = sessionId, token = rep.token})
                else
                    res.rep(true, false, "Auth Failed")
                end
                res.rep = nil
            end
        end
    end)
end)

bifrost.dispatch("cmd", function(session, source, cmd, ...)
    local f = command[cmd]
    if not f then
        debug("Unknown command" .. cmd)
        return
    end
    f(source, ...)
end)

bifrost.start(function()
    bifrost.timeout(500, cleanTimeoutSession)
    bifrost.timeout(100, cleanTimeoutAuth)
end)
