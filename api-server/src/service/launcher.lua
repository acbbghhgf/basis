bifrost = require "bifrost"
local c = require "bifrost.core"

local command = {}
local instance_response = {} -- for function (LAUNCH, LAUNCHOK, LAUNCHERROR)
local debug = bifrost.debug

function command.LAUNCH(_, ...)
    local inst = bifrost.launch(table.concat({...}, " "))
    local response = bifrost.response()
    if inst then
        instance_response[inst] = response
    else
        debug("error return")
        response(false)
    end
end

function command.LAUNCHOK(addr)
    local response = instance_response[addr]
    if response then
        response(true, addr)
        instance_response[addr] = nil
    end
end

function command.LAUNCHERROR(addr)
    local response = instance_response[addr]
    if response then
        response(false)
        instance_response[addr] = nil
    end
end

local function launchCb(session, source, cmd, ...)
    local f = command[cmd]
    if not f then
        debug("Unknown command" .. cmd)
        bifrost.ret(false)
        return
    end
    local ret = f(source, ...)
    if ret then
        bifrost.ret(ret)
    end
end

bifrost.dispatch("lua", launchCb)

c.callback(bifrost.dispatchMessage)
