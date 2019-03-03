package.path = LUA_PATH;
package.cpath = LUA_CPATH;
package.ppath = LUA_PPATH

local cmd = require "cmd"
local router = require "router"
local global = require "global"
local proxy = require "proxy"

local coroutinePool = setmetatable({}, { __mode = "kv" })

local function routine(ud, ...)
    -- Note: if this coroutine yield, outside c loop will destroy ud
    assert(ud ~= nil)
    if type(ud) == "string" then
        local f = cmd[ud]
        if not f then
            print("Unknown cmd " .. ud)
        else
            f(...)
        end
    else
        if not global.addr then print "NO MSG SERVER AVAILABLE" return end
        local url = ud.url
        if string.match(url, "^/start/") then -- url is /start/type
            router.start(ud, select(2, proxy.sepString(url, "/", 2)))
        else
            local f = router[url]
            if not f then
                print("Unknown url " .. url)
            else
                f(ud)
            end
        end
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

function processer(ud, ...)
    local co = coCreate(routine)
    local ok, ret = coroutine.resume(co, ud, ...)
    if not ok then print(ret) end
end
