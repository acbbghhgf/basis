local zeus = require "qtk.zeus"
local driver = require "zeus.socket.driver"
local queue = require "qtk.zeus.queue"
local buffer = require "zeus.buffer"

local socket = {
   -- read qtk_zeus_socket.h
   MODE_RDONLY = 0,
   MODE_WRONLY = 1,
   MODE_RDWR = 2,
}

local socket_pool = {}

local socket_message = {}

local function socket_close(s)
   local id = s.id
   if id then
      driver.close(id)
      if s.buffer then
         s.buffer:clear()
      end
      socket_pool[id] = nil
   end
   s.id = nil
end

-- ZSOCKET_MSG_OPEN
socket_message[1] = function(s)
   if s and s.co then
      zeus.wakeup(s.co)
   end
end

-- ZSOCKET_MSG_CLOSE
-- closed by remote peer
socket_message[2] = function(s)
   print "closed by remote"
   if s and s.co then
      s.id = nil
      zeus.wakeup(s.co)
   end
   s.callback()
end

-- ZSOCKET_MSG_DATA
socket_message[3] = function(s, ...)
   if s.callback then
      s.callback(...)
   else
      local buf = s.buffer
      if not buf then
         buf = buffer.new()
         s.buffer = buf
      end
      buf:push_rawdata(...)
   end
end

-- ZSOCKET_MSG_ERR
socket_message[4] = function(s, ...)
   if s.co then
      zeus.wakeup(s.co)
   end
   if s.callback then
      s.callback(false, zeus.tostring(...))
   else
      s.buffer = nil
   end
   socket_close(s)
end

-- ZSOCKET_MSG_ACCEPT
socket_message[5] = function(s, ...)
   local id = zeus.toint(...)
   if not id then
      print (...)
      return 0
   end
   if s.callback then
      socket_pool[id] = {
         id = id,
         lock = queue(),
      }
      s.callback(s.id, id)
   end
end

zeus.register_protocol {
   name = "socket",
   id = zeus.PTYPE_SOCKET,
   unpack = driver.unpack,
   dispatch = function (_, _, t, id, p, sz)
      local s = socket_pool[id]
      if s then
         s.lock(function()
               socket_message[t](s, p, sz)
         end)
      end
   end
}


function socket.callback(id, cb)
   local s = socket_pool[id]
   if s then
      local buf = s.buffer
      s.buffer = nil
      s.callback = cb
      return buf
   end
end


function socket.open(addr, mode, rcb)
   local s = {
      callback = rcb,
      lock = queue(),
   }
   local id = driver.open(addr, mode)
   s.id = id
   assert(not socket_pool[id], "duplicate open socket")
   s.co = coroutine.running()
   socket_pool[id] = s
   zeus.wait()
   return s.id
end


function socket.connect(addr, port, rcb)
   local s = {
      callback = rcb,
      lock = queue(),
   }
   local id = driver.connect(addr, port)
   s.id = id
   assert(not socket_pool[id], "duplicate open socket")
   s.co = coroutine.running()
   socket_pool[id] = s
   zeus.wait()
   return s.id
end


function socket.listen(addr, port, ncb, backlog)
   local s = {
      callback = ncb,
      lock = queue(),
   }
   local id = driver.listen(addr, port, backlog)
   s.id = id
   assert(not socket_pool[id], "duplicate open socket")
   s.co = coroutine.running()
   socket_pool[id] = s
   zeus.wait()
   return s.id
end


function socket.close(id)
   local s = socket_pool[id]
   if s then
      socket_close(s)
   end
end


function socket.send(id, msg)
   if not id then
      print "send error: socket is closed"
      return
   end
   local s = socket_pool[id]
   if s then
      driver.send(id, msg)
   end
end


function socket.request(id, msg)
   local s = socket_pool[id]
   if s then
      assert(not s.callback)
      driver.send(id, msg)
      zeus.wait()
      return s.buffer
   end
end


function socket.httpconn(addr, port, rcb)
   local s = {
      callback = rcb,
      lock = queue(),
   }
   local id, hc = driver.httpcon(addr, port)
   s.id = id
   assert(not socket_pool[id], "duplicate open httpconn")
   s.co = coroutine.running()
   socket_pool[id] = s
   zeus.wait()
   s.co = nil
   if s.id then
      return hc
   end
end


return socket
