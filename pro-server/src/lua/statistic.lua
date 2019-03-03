local global = require "global"
local xfer = require "xfer.core"
local json = require "dkjson"
local services = require "services"

local idServer = global.idServer
local serviceIds = global.serviceIds

local statistic = {}
--return table
local function split(input, delimiter)  --split input-str delimiter- example":",
    input = tostring(input) 
    delimiter = tostring(delimiter) 
    if (delimiter=='') then return false end 
    local pos,arr = 0, {} 
    for st,sp in function() return string.find(input, delimiter, pos, true) end do 
        table.insert(arr, string.sub(input, pos, st - 1)) 
        pos = sp + 1 
    end 
    table.insert(arr, string.sub(input, pos)) 
    return arr 
end

function statistic.getWeight(req, addr)
    local rep = {}
    for id, server in pairs(idServer) do    --idServer
        idta = split(id, ":")
        local server_serv = json.encode(server.services)
        server_serv = string.gsub(server_serv, '%\"', "")
        idname = idta[1] .. server_serv
        rep[idname] = server.weight               
    end
    xfer.xfer(200, {
            addr = addr,
            body = json.encode(rep),
        })
end

function statistic.getMem(req, addr)
    local rep = {
        main = math.floor(collectgarbage("count")),
    }
    xfer.xfer(200, {
            addr = addr,
            body = json.encode(rep),
        })
end

function statistic.getCnt(req, addr)
    local rep = {}
    for id, server in pairs(idServer) do
        rep[id] = server.count
    end
    xfer.xfer(200, {
            addr = addr,
            body = json.encode(rep),
        })
end

function statistic.getPenalty(req, addr)
    local rep = {}
    for id, server in pairs(idServer) do
        rep[id] = server.penalty
    end
    xfer.xfer(200, {
            addr = addr,
            body = json.encode(rep),
        })
end

return statistic
