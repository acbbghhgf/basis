local c = require "bifrost.core"
local bifrost = require "bifrost"
local xfer = require "xfer.core"
local env = require "env"

local services = {"asr", "tts", "robot", "bfio"}

local function initServiceCtx(MSG_SOCK_ID, UPLOAD_POOL_ID)
    for _, service in ipairs(services) do
        for i = 0, env.optnumber(service .. "Cnt", 1) do
            local handle = bifrost.newService(service)
            local name = service .. tostring(i)
            bifrost.name("." .. name, handle)
            bifrost.send(handle, "lua", MSG_SOCK_ID, UPLOAD_POOL_ID, name)
        end
    end
end

local function launchTest()
    bifrost.newService("pb_test")
end

bifrost.start(function()
    local launcher = assert(bifrost.launch("blua", "launcher"))
    bifrost.name(".launcher", launcher)
    local main = bifrost.newService("main")
    bifrost.name(".main", main)
    local console = bifrost.newService("console")
    bifrost.name(".console", console)
    local sender = bifrost.newService("sender")
    bifrost.name(".sender", sender)
    local proxy = bifrost.newService("proxy")
    bifrost.name(".proxy", proxy)
    local compat = bifrost.newService("compat")
    bifrost.name(".compat", compat)
    local session = bifrost.newService("session")
    bifrost.name(".session", session)
    local statistic = bifrost.newService("statistic")
    bifrost.name(".statistic", statistic)
    local pb_main = bifrost.newService("pb_main")
    bifrost.name(".pb_main", pb_main)
    local wlist = bifrost.newService("wlist")
    bifrost.name(".wlist", wlist)

    assert(xfer.listen(env.optstring("listenAddr", "0.0.0.0:8000"), env.optnumber("listenBackLog", 100)))
    MSG_SOCK_ID = xfer.estb(env.optstring("msgAddr", "192.168.0.98:10000"), "LAZY_RECONNECT")
    AUTH_POOL_ID = xfer.newPool(env.optstring("authAddr", "192.168.0.126:9003"),
                                env.optnumber("authPoolNslot", 100))
    UPLOAD_POOL_ID = xfer.newPool(env.optstring("uploadAddr", "192.168.0.126:9003"),
                                  env.optnumber("uploadPoolNslot", 100))
    assert(MSG_SOCK_ID and AUTH_POOL_ID and UPLOAD_POOL_ID)

    initServiceCtx(MSG_SOCK_ID, UPLOAD_POOL_ID)
    --launchTest()

    bifrost.send(session, "lua", AUTH_POOL_ID)
    bifrost.send(main, "lua", UPLOAD_POOL_ID)
    bifrost.send(proxy, "lua", UPLOAD_POOL_ID)
    bifrost.send(compat, "lua", UPLOAD_POOL_ID)
    bifrost.send(sender, "lua", UPLOAD_POOL_ID)
    bifrost.send(pb_main, "lua", UPLOAD_POOL_ID)

    assert(bifrost.launch("gate", MSG_SOCK_ID, UPLOAD_POOL_ID, AUTH_POOL_ID))
    bifrost.exit()
end)
