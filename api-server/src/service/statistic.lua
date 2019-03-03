-- specific action will trigger corresponding statistic
local bifrost = require "bifrost"

local cntInfo = {
    total = 0,
 -- [service] = cnt,
 -- [appId] = {
 --      [deviceId] = {
 --          total = cnt,
 --          [service] = cnt,
 --      },
 --      total = cnt,
 --      [service] = cnt,
 -- }
}

local actionHandler = {}

function actionHandler.newStream(_, _, msg)
    local streamId = msg.streamId
    if not streamId then
        bifrost.debug("missing streamId")
        return
    end
    local appId, deviceId, token, hook = bifrost.unpackStreamId(streamId)
    local service = msg.service

    cntInfo.total = cntInfo.total + 1
    cntInfo[service] = (cntInfo[service] or 0) + 1

    local appInfo = cntInfo[appId] or {}
    appInfo.total = (appInfo.total or 0) + 1
    appInfo[service] = (appInfo[service] or 0) + 1
    cntInfo[appId] = appInfo

    local deviceInfo = appInfo[deviceId] or {}
    deviceInfo.total = (deviceInfo.total or 0) + 1
    deviceInfo[service] = (deviceInfo[service] or 0) + 1
    appInfo[deviceId] = deviceInfo
end

function actionHandler.deleteStream(_, _, msg)
    local streamId = msg.streamId
    if not streamId then
        bifrost.debug("missing streamId")
        return
    end
    local appId, deviceId, token, hook = bifrost.unpackStreamId(streamId)
    local service = msg.service
    cntInfo.total = cntInfo.total - 1
    cntInfo[service] = cntInfo[service] - 1

    local appInfo = cntInfo[appId]
    appInfo.total = appInfo.total - 1
    if appInfo.total == 0 then
        cntInfo[appId] = nil
        return
    end
    appInfo[service] = appInfo[service] - 1
    local deviceInfo = appInfo[deviceId]
    deviceInfo.total = deviceInfo.total - 1
    if deviceInfo.total == 0 then
        appInfo[deviceId] = nil
        return
    end
    deviceInfo[service] = deviceInfo[service] - 1
end

local function dispatcher(session, source, msg)
    local action  = msg.action
    local handler = actionHandler[action]
    if handler then handler(session, source, msg) end
end

local cmder = {}
function cmder.getMemInfo()
    collectgarbage()
    bifrost.ret(collectgarbage("count"))
end

function cmder.getCnt()
    bifrost.ret(cntInfo)
end

function cmder.getTotalCnt()
    local cntTotal = {}
    for server, cntNum in pairs(cntInfo) do 
        if type(cntNum) ~= type(cntTotal) then
            cntTotal[server] = cntNum
        end
    end
    bifrost.ret(cntTotal)
end

function cmder.getDeviceCnt(_, _, param)
    local appId, deviceId = string.unpack("zz", param)
    local appInfo = cntInfo[appId]
    if not appInfo then
        bifrost.ret(0)
        return
    end
    local deviceInfo = appInfo[deviceId]
    if not deviceInfo then
        bifrost.ret(0)
        return
    end
    bifrost.ret(deviceInfo.total)
end

bifrost.start(function()
    bifrost.dispatch("lua", dispatcher)
    bifrost.dispatch("cmd", function(sessoin, source, cmd, param)
        local handler = cmder[cmd]
        if handler then handler(session, source, param) end
    end)
    -- debug
    --local function debugCnt()
        --local json = require "dkjson"
        --print(json.encode(cntInfo))
        --bifrost.timeout(200, debugCnt)
    --end
    --bifrost.timeout(200, debugCnt)
end)
