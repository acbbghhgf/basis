package.path = "lualib/?.lua;lualib/?/init.lua"
package.cpath = "./luaclib/?.so"
package.ppath = LUA_PPATH
LUA_PPATH = nil

local bifrost = require "bifrost"
local xfer = require "xfer.core"
local json = require "dkjson"
local env = require "env"
local payload = require "payload"

local debug = bifrost.debug

local coroutinePool = setmetatable({}, {__mode = "kv"})

local MSG_SOCK_ID, UPLOAD_POOL_ID, AUTH_POOL_ID = ...
MSG_SOCK_ID = tonumber(MSG_SOCK_ID)
UPLOAD_POOL_ID = tonumber(UPLOAD_POOL_ID)
AUTH_POOL_ID = tonumber(AUTH_POOL_ID)

local mainHandle
local sessionHandle
local senderHandle
local proxyHandle
local compatHandle
local pb_mainHandle

local statisticContent = [[{"ncpu":0,"nstream":0}]]

local msgDispatcher = {}
msgDispatcher["/statistic"] = function(msg)
    statisticContent = msg.body
end

local cliDispatcher = {}
cliDispatcher["/statics"] = function(msg)
    xfer.xfer(200, {addr = msg.id, body = statisticContent})
end

cliDispatcher["/api/v3.0/score"] = function(msg)
    bifrost.send(compatHandle, "lua", msg)
end

cliDispatcher["/"] = function(msg)
    bifrost.send(mainHandle, "lua", msg)
end

cliDispatcher["/auth"] = function(msg)
    bifrost.send(sessionHandle, "lua", msg)
end

cliDispatcher["/api/ctl"] = function(msg)
    bifrost.send(".console", "lua", msg)
end

cliDispatcher["/ping"] = function(msg)
    local field
    if msg.body and #msg.body > 0 then
        field = {ping = string.match(msg.body, "[%g]*")}
    end
    xfer.xfer(200, {addr = msg.id, field = field})
end

local function forwardMsgReply(msg)
    local f = msgDispatcher[msg.url]
    if f then
        f(msg)
        return
    end
    local tmp = {}
    string.gsub(msg.url, "[^/]+", function(w) table.insert(tmp, w) end)
    local dest, streamId, t = table.unpack(tmp, 1, 3)
    local cmd = t == "rep" and "RESPONSE" or "ERROR"
    if t == "rep" then
        bifrost.send("." .. dest, "cmd", "RESPONSE", string.pack("zzsz", streamId, "", msg.body or "", ""))
    else
        bifrost.send("." .. dest, "cmd", "ERROR", string.pack("zs", streamId, msg.body or ""))
    end
end

local function forwardPB(ud)
    pack = payload.unpack(ud.variant)
    pack.id = ud.id
    bifrost.send(pb_mainHandle, "lua", pack, ud.wsi)
end

local function forwardJwt(ud)
    bifrost.send(pb_mainHandle, "lua", ud.id, ud.jwt)
end

local function forwardProto(ud)
    if ud.isprotobuf then
        return forwardPB(ud)
    elseif ud.isjwt then
        return forwardJwt(ud)
    end
    local msg = {
        type = ud.type,
        addr  = ud.addr,
        body  = ud.body,
        id = ud.id,
        field = {
            hook = ud.hook,
            contentType = ud["content-type"],
            sn = ud["seq-no"],
        },
    }
    if ud.type == "RESPONSE" then
        msg.status = ud.status
    else
        msg.url = ud.url
    end
    if msg.id == MSG_SOCK_ID then
        forwardMsgReply(msg)
    elseif msg.id == AUTH_POOL_ID then
        bifrost.send(sessionHandle, "lua", msg)
    elseif msg.id == UPLOAD_POOL_ID then
        if msg.status ~= 200 then
            msg.req = ud.req
            bifrost.send(senderHandle, "lua", msg)
            bifrost.debug("upload error")
        end
    else
        local f = cliDispatcher[msg.url]
        if f then
            f(msg)
        else
            local proxyLen = #"/proxy/"
            local url = msg.url
            if not url then return end
            if #url > proxyLen and string.sub(url, 1, proxyLen) == "/proxy/" then
                bifrost.send(proxyHandle, "lua", msg)
                return
            end
            local hook = msg.field and msg.field.hook
            xfer.xfer(500, {
                addr=msg.id,
                field={hook=hook, ["Content-Type"]="text/plain"},
                body = "you are lost :(",
            })
        end
    end
end

local function forwardNotice(id, notice)
    if notice == "DISCONNECT" or notice == "ECONNECT" then
        bifrost.send(sessionHandle, "cmd", notice, id)
        bifrost.send(compatHandle, "cmd", notice, id)
        bifrost.send(pb_mainHandle, "cmd", notice, id)
        xfer.destroy(id)
    elseif id == MSG_SOCK_ID and notice == "CONNECT" then
        xfer.xfer("PUT", "/_watch", {addr = id, body = [[{"/statistic":""}]]})
    end
end

local function coCreate(f)
    local co = table.remove(coroutinePool)
    if co == nil then
        co = coroutine.create(function(...)
            f(...)
            while true do
                f = nil
                coroutinePool[#coroutinePool + 1] = co
                f = coroutine.yield()
                f(coroutine.yield())
            end
        end)
    else
        coroutine.resume(co, f)
    end
    return co
end

local function routine(ud, ...)
    if type(ud) == "userdata" then
        forwardProto(ud)
    elseif type(ud) == "number" then
        forwardNotice(ud, ...)
    else
        debug("unknown type", type(ud))
    end
end

function processer(ud, ...)
    local co = coCreate(routine)
    local ok, ret = coroutine.resume(co, ud, ...)
    if not ok then debug(ret) end
end

do      -- do some init action
    sessionHandle = bifrost.queryHandle(".session")
    mainHandle = bifrost.queryHandle(".main")
    senderHandle = bifrost.queryHandle(".sender")
    proxyHandle = bifrost.queryHandle(".proxy")
    compatHandle = bifrost.queryHandle(".compat")
    pb_mainHandle = bifrost.queryHandle(".pb_main")
end
