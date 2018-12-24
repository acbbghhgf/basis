local bifrost = require "bifrost"
local json = require "dkjson"
local xfer = require "xfer.core"
local env = require "env"

local exHandler = {}

local memInfo = {
    main = 0,
    console = 0,
    session = 0,
    proxy = 0,
    sender = 0,
    asr = 0,
    tts = 0,
    robot = 0,
    compat = 0,
    statistic = 0,
}

local status = {
    errLogCnt = 0,
}

function exHandler.getCnt(req, addr)
    local cntInfo = bifrost.call(".statistic", "cmd", "getCnt", "")
    if not xfer.xfer(200, {addr=addr, body=json.encode(cntInfo)}) then
        bifrost.debug "xfer error"
    end
end

function exHandler.getTotalCnt(req, addr)
    local cntInfo = bifrost.call(".statistic", "cmd", "getTotalCnt", "")
    if not xfer.xfer(200, {addr=addr, body=json.encode(cntInfo)}) then
        bifrost.debug "xfer error"
    end
end

function getServiceMem(service)
    local mem = 0
    for i=0, env.optnumber(service .. "Cnt", 1) do
        local dest = "." .. service .. tostring(i)
        local tmp = bifrost.call(dest, "cmd", "getMemInfo", "")
        mem = mem + math.floor(tmp)
    end
    return mem
end

function exHandler.getMemInfo(req, addr)
    local mainMem = bifrost.call(".main", "cmd", "getMemInfo", "")
    memInfo.main = math.floor(mainMem)
    local sessionMem = bifrost.call(".session", "cmd", "getMemInfo", "")
    memInfo.session = math.floor(sessionMem)
    local proxyMem = bifrost.call(".proxy", "cmd", "getMemInfo", "")
    memInfo.proxy = math.floor(proxyMem)
    local senderMem = bifrost.call(".sender", "cmd", "getMemInfo", "")
    memInfo.sender = math.floor(senderMem)

    memInfo.asr = getServiceMem("asr")
    memInfo.tts = getServiceMem("tts")
    memInfo.robot = getServiceMem("robot")

    local compatMem = bifrost.call(".compat", "cmd", "getMemInfo", "")
    memInfo.compat = math.floor(compatMem)
    local statisticMem = bifrost.call(".statistic", "cmd", "getMemInfo", "")
    memInfo.statistic = math.floor(statisticMem)
    collectgarbage()
    memInfo.console = math.floor(collectgarbage("count"))

    if not xfer.xfer(200, {addr=addr, body=json.encode(memInfo)}) then
        bifrost.debug "xfer error"
    end
end

function exHandler.launch(req, addr)
    local name = table.unpack(req.param)
    local inst = bifrost.newService(name, select(2, table.unpack(req.param)))
    if not xfer.xfer(200, {addr=addr, body=tostring(inst)}) then
        bifrost.debug "xfer error"
    end
end

function exHandler.kill(req, addr)
    local param = table.unpack(req.param)
    bifrost.kill(param)
    if not xfer.xfer(200, {addr=addr, body="ok"}) then
        bifrost.debug "xfer error"
    end
end

function exHandler.getStatus(req, addr)
    local errLogCnt = bifrost.call(".sender", "cmd", "getErrLog", "")
    status.errLogCnt = errLogCnt
    if not xfer.xfer(200, {addr=addr, body=json.encode(status)}) then
        bifrost.debug "xfer error"
    end
end

local function dispatcher(session, source, msg)
    if msg and msg.body and #(msg.body) > 0 then
        local req = json.decode(msg.body)
        local cmd = req and req.cmd
        if cmd then
            local handler = exHandler[cmd]
            if handler then handler(req, msg.id) end
        end
    end
end

local cmder = {}
bifrost.start(function()
    bifrost.dispatch("lua", dispatcher)
    bifrost.dispatch("cmd", function(session, source, cmd, param)
        local handler = cmder[cmd]
        if handler then handler(param) end
    end)
end)
