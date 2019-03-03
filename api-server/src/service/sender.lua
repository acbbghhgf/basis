local bifrost = require "bifrost"
local json = require "dkjson"
local xfer = require "xfer.core"

local path = "/tmp/bifrostd_sender"
local UPLOAD_POOL_ID

local function fileExist(path)
    local f = io.open(path, 'rb')
    if f then f:close() end
    return f ~= nil
end

local function sendRetry()
    if fileExist(path) then
        for line in io.lines(path) do
            local url, content = string.unpack("zz", line)
            local c = json.decode(content)
            c.addr = UPLOAD_POOL_ID
            if not xfer.xfer("POST", url, c) then
                bifrost.debug("Error Xfer")
            end
        end
        os.remove(path)
    end
    bifrost.timeout(100*60*60, sendRetry)
end

local function cb(session, source, uploadPoolId)
    UPLOAD_POOL_ID = uploadPoolId
    bifrost.timeout(0, sendRetry)
    bifrost.dispatch("lua", function(session, source, msg)
        local content, url
        if msg.type == "REQUEST" then
            content = json.encode{field=msg.field, body=msg.body}
            url = msg.url
        else
            url = msg.req.url
            if url == "/api/v1.0/log/error" or url == "/api/v1.0/log/result" then
                local req = msg.req
                content = json.encode{field=req.field, body=req.body}
            end
        end
        if content then
            local f = io.open(path, "a+")
            f:write(string.pack("zz", url, content))
            f:write("\n")
            f:close()
        end
    end)
end

local cmdHandler = {}
function cmdHandler.getMemInfo()
    bifrost.ret(collectgarbage("count"))
end

function cmdHandler.getErrLog()
    local cnt = 0
    if fileExist(path) then
        for line in io.lines(path) do
            cnt = cnt + 1
        end
    end
    bifrost.ret(cnt)
end

local function cmdCb(session, source, cmd, param)
    local f = cmdHandler[cmd]
    if not f then
        bifrost.log(string.format("[warning] unknown cmd: %s for sender", cmd)) return
    end
    f(param)
end

bifrost.start(function()
    bifrost.dispatch("lua", cb)
    bifrost.dispatch("cmd", cmdCb)
end)
