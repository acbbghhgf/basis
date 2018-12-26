package.path = LUA_PATH
package.cpath = LUA_CPATH

local dlg = require "dlg"
local xfer = require "xfer"
local new = dlg.new
local clear = dlg.clear
local coroutinePool = setmetatable({}, { __mode = "kv" })

local MSG_ADDR

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

local function write(method, topic, content)
    if not MSG_ADDR then
        print "ERROR: Msg Server Unavaliable"
        return
    end
    if not xfer.xfer(method, topic, {addr = MSG_ADDR, body = content}) then
        print("Write " .. tostring(topic) .. " failed")
    end
end

local G = {
    new = new,
    write = write,
    print = print,
}

function processer(id, script)
    local t = type(id)
    if t == "number" then
        if script == "CONNECT" then
            MSG_ADDR = id
        elseif script == "DISCONNECT" then
            MSG_ADDR = nil
        else
            print("Uknow sig", script)
        end
    elseif t == "string" then
        local f, ret = load(script, "processer", "t", G)
        if not f then print("Invalid script", ret) end
        local co = coCreate(function()
                                f()
                                write("PUT", "/stop", id)
                            end)
        local ok, ret = coroutine.resume(co)
        if not ok then print(ret) end
    else
        print("Unknow id type", t, script)
    end
end

function clientHandler(caddr, script)
    local CG = {
        new = new,
        clear = clear,
        print = print,
    }

    if script == "EXIT" then os.exit() end

    CG.reply = function(status, content)
        if not xfer.xfer(status, {addr=caddr, body=content}) then
            print("Send " .. tostring(content) .. "failed")
        end
    end

    local f, ret = load(script, "clientHandler", "t", CG)
    if not f then
        print("Invalid script", ret)
        print(script)
        return
    end
    local co = coroutine.create(f)
    local ok, ret = coroutine.resume(co)
    if not ok then print(ret) end
end

-- test section
xfer.listen("0.0.0.0:18888", 10)

local testScript = [[
    local testParam = [=[
            {
                "clientId": "xxx",
                "input": "你好"
            } ]=]
    local inst = new("/data/wtk/cloud/yuki/yuki/yuki.cfg")
    if not inst then print "get inst error" end
    inst:calc(testParam)
    local res = inst:getResult()
    print("res is", res)
    inst:close()

    local inst = new("/data/wtk/cloud/yuki/yuki/yuki.cfg")
    if not inst then print "get inst error" end
    inst:calc(testParam)
    local res = inst:getResult()
    print("res is", res)
    inst:close()

    local inst = new("/data/wtk/cloud/yuki/yuki/yuki.cfg")
    if not inst then print "get inst error" end
    inst:calc(testParam)
    local res = inst:getResult()
    print("res is", res)
    write("PUT", "/", res)
    inst:close()
]]

--processer("test", testScript)
