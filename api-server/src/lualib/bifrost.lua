local c = require "bifrost.core"
local xfer = require "xfer.core"
local msgpack = require "cmsgpack"
require "debug"

local traceback = debug.traceback

local bifrost = {
    -- see qtk_bifrost.h
    QTK_TIMER    = 0,
    QTK_ERROR    = 1,
    QTK_SYS      = 2,
    QTK_LUA      = 3,
    QTK_RET      = 4,
    QTK_BFRX     = 5,
    QTK_CMD      = 6,
}

-- weak table for resuing coroutine
local coroutinePool = setmetatable({}, { __mode = "kv" })

local coroutineSession = {}
local coroutineSource = {}
local sessionCoroutine = {}
local sleepCoroutineSession = {}
local wakeUpQueue = {}
local exitFunc = {}

local proto = {}
local suspend       -- suspend is a function

local function coCreate(f)
    local co = table.remove(coroutinePool)
    if co == nil then
        co = coroutine.create(function(...)
            f(...)
            while true do
                f = nil
                coroutinePool[#coroutinePool + 1] = co
                f = coroutine.yield "END"
                f(coroutine.yield())
            end
        end)
    else
        coroutine.resume(co, f)
    end
    return co
end

function bifrost.seqlock()
	local currentThread
	local ref = 0
	local threadQueue = {}

	local function xpcallRet(ok, ...)
		ref = ref - 1
		if ref == 0 then
			currentThread = table.remove(threadQueue,1)
			if currentThread then
				bifrost.wakeUp(currentThread)
			end
		end
		assert(ok, (...))
		return ...
	end

	return function(f, ...)
		local thread = coroutine.running()
		if currentThread and currentThread ~= thread then
			table.insert(threadQueue, thread)
			bifrost.wait()
			assert(ref == 0)
		end
		currentThread = thread

		ref = ref + 1
		return xpcallRet(xpcall(f, traceback, ...))
	end
end

bifrost.pack = c.pack
bifrost.unpack = c.unpack

function bifrost.debug(...)
    local info = debug.getinfo(2)
    local s = tostring(info.name) .. " " .. tostring(info.currentline)
    s = s .. "@" .. info.short_src .. ":"
    for _, arg in ipairs({...}) do
        s = s .. " " .. tostring(arg)
    end
    print(s)
end

function bifrost.timeout(ti, func)
    local session = c.intcommand("TIMEOUT", ti)
    assert(session)
    local co = coCreate(func)
    assert(sessionCoroutine[session] == nil)
    sessionCoroutine[session] = co
end

function bifrost.sleep(ti)
    local session = c.intcommand("TIMEOUT", ti)
    assert(session)
    local succ, ret = coroutine.yield("SLEEP", session)
    sleepCoroutineSession[coroutine.running()] = nil
    if succ then    -- wakeup by timer
        return
    end
    if ret == "BREAK" then     -- wakeup by bifrost.wakeUp
        return "BREAK"
    else
        error(ret)
    end
end

function bifrost.wait(co)
    local session = c.genid()
    coroutine.yield("SLEEP", session)
    co = co or coroutine.running()
    sleepCoroutineSession[co] = nil
    sessionCoroutine[session] = nil
end

function bifrost.wakeUp(co)
    if sleepCoroutineSession[co] then
        table.insert(wakeUpQueue, co)
        return true
    end
end

function bifrost.address(addr)
    if type(addr) == "number" then
        return string.format(":%08x", addr)
    else
        return tostring(addr)
    end
end

function bifrost.launch(...)
    local addr = c.command("LAUNCH", table.concat({...}, " "))
    if addr then
        return tonumber("0x" .. string.sub(addr, 2))
    end
end

function bifrost.kill(name)
    c.command("KILL", bifrost.address(name))
end

function bifrost.register(name)
    c.command("REG", name)
end

function bifrost.name(name, handle)
    c.command("NAME", name .. " " .. bifrost.address(handle))
end

function bifrost.exit()
    local func
    while true do
        func = table.remove(exitFunc)
        if not func then break end
        func()
    end
    c.command("EXIT")
end

function bifrost.atExit(func)
    table.insert(exitFunc, func)
end

local selfHandle
function bifrost.selfHandle()
    if selfHandle then
        return selfHandle
    end
    selfHandle = tonumber("0x" .. string.sub(c.command("REG"), 2))
    return selfHandle
end

function bifrost.genId()
    return c.genid()
end

function bifrost.genUUID()
    return c.genUUID()
end

function bifrost.queryHandle(name)
    local handle = c.command("QUERY", name)
    if handle then
        return tonumber("0x" .. string.sub(handle, 2))
    end
end

function bifrost.registerProtocol(class)
    local name = class.name
    local id = class.id
    assert(proto[name] == nil and proto[id] == nil)
    assert(type(name) == "string" and type(id) == "number")
    proto[name] = class
    proto[id] = class
end

function bifrost.dispatch(typename, func)
    local p = proto[typename]
    if func then
        local ret = p.dispatch
        p.dispatch = func
        return ret
    else
        return p and p.dispatch
    end
end

do
    local REG = bifrost.registerProtocol
    REG {
        name = "lua",
        id  = bifrost.QTK_LUA,
        pack = c.pack,
        unpack = c.unpack,
    }
    REG {
        name = "cmd",
        id = bifrost.QTK_CMD,
        pack = function(cmd, param) return string.pack("zs", cmd, param) end,
        unpack = function(msg, sz)
            local str = c.tostring(msg, sz)
            return string.unpack("zs", str)
        end
    }
end

-- if caller need alloc session automatically, just set session = nil
-- if send succ return session or nil
function bifrost.send(dest, prototype, ...)
    local p = proto[prototype]
    if not p then
        bifrost.debug("Unknown prototype " .. tostring(prototype))
        return
    end
    return c.send(dest, p.id, 0, p.pack(...))
end

function bifrost.rawsend(dest, prototype, session, msg_string)
    local p = proto[prototype]
    if not p then
        bifrost.debug("Unknown prototype " .. tostring(prototype))
        return
    end
    return c.send(dest, p.id, session, msg_string)
end

function bifrost.log(level, str)
    local msg = string.format("{%s}-- [%s] %s", os.date(), level, str)
    return bifrost.rawsend(".logger", bifrost.QTK_LUA, 0, msg)
end

function bifrost.upload(content, host, id)
    local boundary = "----QdreamerBfrxUploader"
    if content.type == "infoData" then
        local infoPart = table.concat{"--", boundary, "\r\n",
                                      "Content-Disposition:form-data;",
                                      "name=\"file_info\"\r\n",
                                      "Content-Type:application/json\r\n\r\n",
                                      content.info or "", "\r\n"}
        local dataPart = table.concat{"--", boundary, "\r\n",
                                      "Content-Disposition:form-data;",
                                      "name=\"myfile\";filename=\"audio\"\r\n",
                                      "Content-Type:application/octet-stream\r\n\r\n",
                                      content.data or "", "\r\n"}
        local eof = table.concat{"--", boundary, "--"}

        local body_msg = msgpack.pack{id = id,
                                    sendbody = table.concat{infoPart, dataPart, eof},
                                    sendhost = host}

        xfer.upload(body_msg)
        
    end
end

local function yieldCall(session)
    local succ, msg, sz = coroutine.yield("CALL", session)
    if not succ then
        error "yieldCall failed"
    end
    return msg, sz
end

function bifrost.call(dest, prototype, ...)
    local p = proto[prototype]
    local session = c.send(dest, p.id, nil, p.pack(...))
    if session == nil then
        error("call send fail")
    end
    return c.unpack(yieldCall(session))
end

function bifrost.ret(...)
    return coroutine.yield("RETURN", c.pack(...))
end

function bifrost.response()
    return coroutine.yield("RESPONSE")
end

local function dispatchWakeUp()
    local co = table.remove(wakeUpQueue, 1)
    if co then
        local session = sleepCoroutineSession[co]
        if session then
            sessionCoroutine[session] = "BREAK"
            return suspend(co, coroutine.resume(co, false, "BREAK")) --can wakeup a sleeping coroutine
        end
    end
end

-- suspend is local function
function suspend(co, result, cmd, param, size)
    if not result then
        local session = coroutineSession[co]
        if session then
            local dest = coroutineSource[co]
            if session ~= 0 then
                bifrost.debug("resume error")
                c.send(dest, bifrost.QTK_ERROR, session, c.pack("resume error"))
            end
            coroutineSession[co] = nil
            coroutineSource[co] = nil
        end
        local errMsg = debug.traceback(co, tostring(cmd))
        --bifrost.log(errMsg)
        error(errMsg)
    end
    if cmd == "END" then -- coroutine end
        coroutineSession[co] = nil
        coroutineSource[co] = nil
    elseif cmd == "CALL" then
        sessionCoroutine[param] = co
    elseif cmd == "RETURN" then
        local session = coroutineSession[co]
        if session == 0 then
            bifrost.debug "session = 0"
            if size ~= nil then
                c.trash(param, size)
            end
            return suspend(co, coroutine.resume(co, false))
        end
        local dest = coroutineSource[co]
        local ret = c.send(dest, bifrost.QTK_RET, session, param, size)
        if ret == false then
            bifrost.debug "send erorr"
        end
        return suspend(co, coroutine.resume(co, ret))
    elseif cmd == "RESPONSE" then
        local session = coroutineSession[co]
        local dest = coroutineSource[co]

        local function response(ok, ...) -- we give back this response handle for future use
            local ret = false
            if session ~= 0 then
                if ok then
                    ret = c.send(dest, bifrost.QTK_RET, session, c.pack(...)) ~= nil
                    if not ret then
                        bifrost.debug("send failed")
                        c.send(dest, bifrost.QTK_ERROR, session, c.pack(""))
                    end
                else -- not ok
                    bifrost.debug("not ok", dest)
                    c.send(dest, bifrost.QTK_ERROR, session, c.pack(""))
                end
            end
            return ret
        end

        return suspend(co, coroutine.resume(co, response))
    elseif cmd == "SLEEP" then
        sessionCoroutine[param] = co
        sleepCoroutineSession[co] = param
    else
		error("Unknown command : " .. command .. "\n" .. debug.traceback(co))
    end
    dispatchWakeUp()
end

function bifrost.dispatchMessage(prototype, msg, sz, session, source)
    if prototype == bifrost.QTK_RET or prototype == bifrost.QTK_TIMER then
        local co = sessionCoroutine[session]
        if co == nil then
            bifrost.log("warning", "unknown response")
            bifrost.debug("unknown response", prototype)
        elseif co == "BREAK" then
            sessionCoroutine[session] = nil
        else
            sessionCoroutine[session] = nil
            suspend(co, coroutine.resume(co, true, msg, sz))
        end
        return
    end

    p = proto[prototype]
    if not p then
        if session ~= 0 then
            c.send(source, bifrost.QTK_ERROR, session, "")
        else
            bifrost.log("warning", "unknown prototype", tostring(prototype))
            bifrost.debug("Unknown prototype", prototype)
        end
        return
    end
    local f = p.dispatch
    if f then
        local co = coCreate(f)
        coroutineSession[co] = session
        coroutineSource[co] = source
        suspend(co, coroutine.resume(co, session, source, p.unpack(msg, sz)))
    else
        bifrost.log("warning", "unknown dispatcher", tostring(prototype))
        bifrost.debug("unregister dispatcher", prototype)
    end
end

local function initService(start_func)
    local ok, err = pcall(start_func)
    if not ok then
        bifrost.debug("initService error: " .. (err or ""))
        bifrost.log("error", "initService error: " .. (err or ""))
        bifrost.send(".launcher", bifrost.QTK_LUA, "LAUNCHERROR")
        bifrost.exit()
    else
        bifrost.send(".launcher", bifrost.QTK_LUA, "LAUNCHOK")
    end
end

function bifrost.newService(name, ...)
    return bifrost.call(".launcher", bifrost.QTK_LUA, "LAUNCH", "blua", name, ...)
end

function bifrost.start(startFunc)
    c.callback(bifrost.dispatchMessage)
    bifrost.timeout(0, function()
        initService(startFunc)
    end)
end

function bifrost.epoch()
    return c.epoch()
end

function bifrost.sepString(s, sep, n)
    local tmp = {}
    string.gsub(s, string.format("[^%s]+", sep), function(w) table.insert(tmp, w) end)
    return table.unpack(tmp, 1, n)
end

function bifrost.packSessionId(appId, deviceId)
    return appId .. "@" .. deviceId
end

function bifrost.unpackSessionId(sessionId)
    return bifrost.sepString(sessionId, "@", 2)
end

function bifrost.packStreamId(appId, deviceId, token, hook)
    return table.concat({appId, deviceId, token, hook}, ":")
end

function bifrost.unpackStreamId(streamId)
    return bifrost.sepString(streamId, ":", 4)
end

function bifrost.unpackUrl(url)
    return bifrost.sepString(url, "/", 4)
end

function bifrost.now()
    return c.now()
end

function bifrost.tableLength(t)
    local len = 0
    for _, _ in pairs(t) do
        len = len + 1
    end
    return len
end

return bifrost
