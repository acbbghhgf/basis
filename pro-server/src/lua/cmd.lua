local global = require "global"
local xfer = require "xfer.core"
local services = require "services"
local json = require "dkjson"

local idServer = global.idServer
local serviceIds = global.serviceIds

local cmd = {}

local function watchTopic(t, uniq)
    local c = {
        addr = global.addr,
        body = json.encode(t)
    }
    local url = uniq and "/_watchuniq" or "/_watch"
    if not xfer.xfer("POST", url, c) then
        print "watchTopic error"
    end
end

cmd.CONNECT = function(id)
    print "msg server connect"
    global.addr = id
    watchTopic({
            ["/register"] = "id=0",
            ["/unregister"] = "",
        })
    watchTopic({
            ["/stop"] = "",
            ["/stop.v2"] = "",
            ["/restart"] = "",
        }, true)
    local tmp = {}
    for service in pairs(services) do
        tmp["/start/" .. service] = ""
    end
    watchTopic(tmp, true)
end

cmd.DISCONNECT = function(id)
    print "msg server disconnect"
    global.addr = nil
    global.idServer = {}
    global.serverId = {}
    xfer.destroy(id)
end

return cmd
