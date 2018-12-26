local proxy = require "proxy"
local global = require "global"
local list = require "list"
local services = require "services"

local stream = {}

local hookStream = {}
local orderStream = list.new()

local idServer = global.idServer

function stream.add(hook, serverId, t, start)
    local s = {
        hook = hook,
        id = serverId,
        type = t,
        time = proxy.now(),
        start = start,
    }
    hookStream[hook] = s
    local server = idServer[serverId]
    server.weight = server.weight + services[t]
    server.count[t] = (server.count[t] or 0) + 1
    orderStream:tailinsert(s)
end

function stream.remove(s, errId)
    if type(s) == "string" then
        local pre = s
        s = hookStream[s]
        if not s then
            print("stream not found " .. tostring(pre))
            return
        end
    end
    local hook = s.hook
    hookStream[hook] = nil
    orderStream:remove(s)
    local id = s.id
    local t = s.type
    if not id or not t then print "invalid stream" return end
    local server = idServer[id]
    if not server then print "server not found" return end
    server.weight = server.weight - services[t]
    if server.weight <= 0 then
        server.weight = 1
        print "should not be here"
    end
    if not server.count[t] then print "serious bug" return end
    server.count[t] = server.count[t] - 1
    if errId then
        if errId == 60002 then -- do punishment
            global.punish(id)
        end
    end
    return s
end

local function streamTimeout()
    local now = proxy.now()
    while true do
        local s = orderStream:touchhead()
        if (not s) or (now - s.time < 6000) then break end
        stream.remove(s)
    end
    proxy.timeout(100, streamTimeout)
end

do          -- init action
    proxy.timeout(100, streamTimeout)
end

return stream
