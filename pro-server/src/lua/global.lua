local xfer = require "xfer.core"
local proxy = require "proxy"
local json = require "dkjson"

local global = {
    idServer = {},
    serviceIds = {},
    addr = nil,
}

local jail = {}

local function jailTimer()
    for id, server in pairs(jail) do
        server.penalty = server.penalty / 2
        if server.penalty <= 1 then
            server.penalty = nil
            jail[id] = nil
        end
    end
    if next(jail) then  -- jail not empty
        proxy.timeout(1000, jailTimer)
    end
end

function global.uploadServerInfo()
    local ncpu = 0
    local nstream = 0
    for _, server in pairs(global.idServer) do
        local streamCnt = 0
        for _, cnt in pairs(server.count) do
            streamCnt = streamCnt + cnt
        end
        ncpu = ncpu + (server.ncpu or 0)
        nstream = nstream + streamCnt
    end
    xfer.xfer("POST", "/statistic", {
        addr = global.addr,
        body = json.encode{ ncpu=ncpu, nstream=nstream }
    })
end

function global.punish(id)
    if not next(jail) then -- empty jail
        proxy.timeout(1000, jailTimer)
    end
    local server = global.idServer[id]
    if server.penalty then
        server.penalty = server.penalty * 2
    else
        server.penalty = 2
        jail[id] = server
    end
end

local function uploadTimer()
    global.uploadServerInfo()
    proxy.timeout(100, uploadTimer)
end

do      -- init action
    proxy.timeout(100, uploadTimer)
end

return global
