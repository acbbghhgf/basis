local statistic = require "statistic"
local xfer = require "xfer.core"
local global = require "global"
local proxy = require "proxy"
local stream = require "stream"
local json = require "dkjson"
local services = require "services"

local serviceIds = global.serviceIds
local idServer = global.idServer

local router = {}

local function raiseError(err, etopic)
    local body = json.encode{errId = 70000, error = err}
    xfer.xfer("PUT", etopic, {addr = global.addr, body = body})
end

local function selectServer(t)
    local ids = serviceIds[t]
    if not ids then print("no server for " .. t) return end

    local id = ids[1]
    local minWeight = idServer[id].weight * (idServer[id].penalty or 1)
    local idPool = {id}

    for idx = 2, #ids do
        local id = ids[idx]
        local server = idServer[id]
        local trueWeight = server.weight * (server.penalty or 1)
        if trueWeight < minWeight then
            minWeight = trueWeight
            idPool = {id}
        elseif trueWeight == minWeight then
            idPool[#idPool + 1] = id
        end
    end

    return idPool[math.random(#idPool)]
end

function router.start(ud, t, restart)
    local script, etopic = string.unpack("zz", ud.body)
    local idSelect = selectServer(t)
    if not idSelect then
        if not etopic then return end
        raiseError("server not found: " .. tostring(t), etopic)
        return
    end
    if restart and idServer[idSelect].penalty then  -- avoid meaningless package among cluster
        if not etopic then return end
        raiseError("no healthy server for restart request", etopic)
        return
    end
    local tmp = {
        script = script,
        id = proxy.genUUID(),
    }
    local body = json.encode(tmp)
    xfer.xfer("PUT", idSelect, {
            addr = global.addr,
            body = body,
        })
    stream.add(tmp.id, idSelect, t, ud.body)
end

router["/statistic"] = function(ud)
    local req = json.decode(ud.body)
    local cmd = req and req.cmd
    if cmd then
        local handler = statistic[cmd]
        if handler then handler(req, ud.id) end
    end
end

router["/register"] = function(ud)
    local server = json.decode(ud.body)
    server.weight = 1
    server.count = {}
    if not server.services then return end
    if not server.id then return end
    local serviceHash = {}
    for _, service in ipairs(server.services) do
        serviceHash[service] = true
        local tmp = serviceIds[service] or {}
        table.insert(tmp, server.id)
        serviceIds[service] = tmp
    end
    if services.mixs then
        for srv, sub in pairs(services.mixs) do
            local valid = true
            for _, item in ipairs(sub) do
                if not serviceHash[item] then
                    valid = false
                    break
                end
            end
            if valid then
                local tmp = serviceIds[srv] or {}
                table.insert(tmp, server.id)
                serviceIds[srv] = tmp
                table.insert(server.services, srv)
            end
        end
    end
    idServer[server.id] = server
    global.uploadServerInfo()
end

router["/unregister"] = function(ud)
    local b = json.decode(ud.body)
    if not b then return end
    if not b.id then return end
    local server = idServer[b.id]
    if not server then return end
    for _, service in ipairs(server.services) do
        for pos, id in ipairs(serviceIds[service]) do
            if id == b.id then
                table.remove(serviceIds[service], pos)
            end
        end
        if #(serviceIds[service]) == 0 then
            serviceIds[service] = nil
        end
    end
    idServer[b.id] = nil
    global.uploadServerInfo()
end

router["/stop"] = function(ud)
    if not ud.body then
        print "Error Empty /stop"
        return
    end
    stream.remove(ud.body)
end

router["/stop.v2"] = function(ud)
    local hook, errId = string.unpack("zJ", ud.body)
    if not hook then
        print("Error Invalid /stop.v2 " .. ud.body)
        return
    end
    stream.remove(hook, errId)
end

router["/restart"] = function(ud)
    local hook, errId = string.unpack("zJ", ud.body)
    if not hook then
        print("Error Invalid /restart " .. ud.body)
        return
    end
    local s = stream.remove(hook, errId)
    if s then router.start({body=s.start}, s.type, true) end
end

return router
