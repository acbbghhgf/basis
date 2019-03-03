local bifrost = require "bifrost"
local curl = require "curl"
local qauth = require "qauth"
local sys = require "sys"
local json = require "dkjson"
local md5 = require "md5.core"
local env = require "env"
local set = require "set"

local c = curl.new()

local appIds = set:new()    -- qdreamer_semdl

local url = env.optstring("ipTableUrl", "")
local alertUrl = env.optstring("alertUrl", "")
local mobiles = {"18502570307", "18252968244"}
--local mobiles = {"18502570307"}
local passwd = "qdreamer"
local fmt = string.format

local function parseIps(s)
    local all = json.decode(s)
    if not all then return end
    local listStr = all.ips
    if not listStr then return end
    local info = all.app_id .. tostring(all.time) .. listStr
    local sign = all.sign
    if not qauth.verify(info, sign) then
        bifrost.log("error", "Verify Not Pass")
        return
    end
    local t = json.decode(listStr)
    if not t then return end
    local ips = {}
    for _, elem in ipairs(t) do
        if elem.ip and elem.expire_date then
            ips[elem.ip] = sys.str2epoch(elem.expire_date, "%Y-%m-%d %H:%M:%S")
        end
    end
    return ips
end

local function alert()
    local now = bifrost.epoch()
    local dynamic = string.sub(tostring(now), -10)
    local sign = md5.sum(passwd .. dynamic)
    for _, mobile in ipairs(mobiles) do
        local args = string.format([[mobile=%s&action=GetIpTableFailed&server=%s&time=%d&sign=%s]],
                                   mobile, sys.hostname(), now, sign)
        local status, body = c:post(alertUrl, args)
        if status ~= 200 then
            bifrost.log("error", "TERRIBLE BUG [alert msg failed] statusCode: " .. tostring(status))
        else
            local reply = json.decode(body)
            if not reply or reply.code ~= 0 then
                bifrost.log("error", "TERRIBLE BUG [alert msg failed]: " .. tostring(body))
            end
        end
        c:reset()
    end
end

local InvalidCnt = 0

local function getIps()
    local all = {}
    local valid = true
    for appId, _ in pairs(appIds) do
        local time = bifrost.epoch()
        local sign = qauth.encode(appId .. tostring(time))
        local field = string.format([[app_id=%s&time=%d&sign=%s]], appId, time, sign)
        local status, body = c:post(url, field)
        if status == 200 then
            local ips = parseIps(body)
            if not ips then
                valid = false
            else
                all[appId] = ips
            end
        else
            bifrost.log("error", "Get Ips Failed")
            alert()
            valid = false
        end
        c:reset()
    end
    if valid then
        InvalidCnt = 0
        bifrost.send(".proxy", "cmd", "UpdateWlist", json.encode(all))
        bifrost.timeout(100*60*60*24, getIps)
    else
        InvalidCnt = InvalidCnt + 1
        if InvalidCnt > 24 then
            bifrost.send(".proxy", "cmd", "UpdateWlist", json.encode(all))
        end
        bifrost.timeout(100*60*60, getIps)
    end
    return all
end

bifrost.dispatch("cmd", function(session, source, cmd, param)
    appIds:add(cmd)
    bifrost.ret(getIps()[cmd])
end)

bifrost.start(function()
end)
