local zeus = require "qtk.zeus"
local srvx = require "qtk.zeus.srvx"
local queue = require "qtk.zeus.queue"
local buffer= require "zeus.buffer"
local socket = require "qtk.zeus.socket"
local driver = require "zeus.iris"
local json = require "qtk.dkjson"
local conf = require "global"
local http = require "zeus.http"

local HTTP_DELETE

local test

local iris_ip
local iris_port
local iris_id
local iris_parser
local alloc_id = 0
local reg_id = nil
local reg_co = nil
local aiexc = nil
local init_lock = nil

local topic_readers = {}

local function readerRelease(reader)
   local topic = reader.topic
   if iris_id and topic then
      local data = {}
      data[0] = reader.topic
      data = driver.httppack("POST", "_unwatch", json.encode(data))
      socket.send(iris_id, data)
      reader.topic = nil
   end
end

local reader_meta = {__gc=readerRelease}

local read_buf

-- id -> {
--    topic = ""
--    co = coroutine
--    lock = queue()
--    offset = 0
--       }
local id_readers = {}

local function readerNew(buf)
   local reader = {
      buf = buf,
      waitlist = {},
      eof = false,
   }
   if type(buf) == "table" then
      reader.waitlist = nil
   end
   setmetatable(reader, reader_meta)
   return reader
end

local function readerClientNew(topic, reader, idx)
   repeat
      alloc_id = alloc_id+1
   until id_readers[alloc_id] == nil
   id_readers[alloc_id] = {
      topic = topic,
      reader = reader,
      co = nil,
      lock = queue(),
      offset = idx,
   }
   return alloc_id
end

local function readerWakeup(reader, data)
   if type(reader.buf) == "table" then
      local cur = reader.cur
      reader.buf[cur] = buffer.tostring(data)
      reader.cur = cur + 1
      local waitco = reader.waitlist
      reader.waitlist = nil
      zeus.wakeup(waitco)
   else
      local newdata = nil
      local waitlist = reader.waitlist
      reader.waitlist = {}
      if data:len() > 0 then
         newdata = data:tostring()
         reader.buf:push(newdata)
      end
      if waitlist == nil then
         return
      end
      for id, co in pairs(waitlist)
      do
         local cli = id_readers[id]
         cli.newdata = newdata
         zeus.wakeup(co)
      end
   end
end

local function watch(topic, offset)
   assert(iris_id)
   local data = {}
   if offset == nil then
      data[topic] = ""
   else
      data[topic] = offset
   end
   data = driver.httppack("POST", "/_watch", json.encode(data))
   socket.send(iris_id, data)
end


local function register()
   assert(iris_id)
   local data = {
      services = conf.services,
      tag = "register",
      ncpu = zeus.getcpus(),
   }
   data = driver.httppack("POST", "/register", json.encode(data))
   socket.send(iris_id, data)
   reg_co = coroutine.running()
   zeus.wait()
   reg_co = nil
   if reg_id then
      if test then
         watch(reg_id, "id=0")
      else
         watch(reg_id)
      end
      return true
   else
      return false
   end
end


local function connect(ip, port)
   local function rcb(msg, sz)
      if msg == nil then
         -- closed by remote peer
         zeus.error "closed by Iris"
         zeus.timeout(1000, function() connect(ip, port) end)
         return
      end
      if msg == false then
         -- socket error
         zeus.error "Iris connection error"
         zeus.timeout(1000, function() connect(ip, port) end)
         return
      end
      read_buf:push_rawdata(msg, sz)
      repeat
         local ret, topic, data, meth = iris_parser:feed(read_buf)
         if ret == 1 then
            if topic == reg_id then
               zeus.send(aiexc, "text", data:tostring())
            else
               local reader = topic_readers[topic]
               if reader then
                  if data:len() == 0 then
                     -- print (topic.." is end")
                     if meth == HTTP_DELETE then
                        reader.error = true
                     end
                     reader.eof = true
                  end
                  readerWakeup(reader, data)
               end
            end
         elseif ret == 2 then
            data = json.decode(data:tostring())
            if data.tag == "register" then
               zeus.wakeup(reg_co)
               if test then
                  reg_id = "192.168.0.97:42714"
               else
                  reg_id = data.id
               end
            end
         end
      until ret <= 0
   end
   iris_id = socket.connect(ip, port, rcb);
   if iris_id ~= nil then
      print "connect to Iris success"
      reg_id = nil
      if register() then
         print ("register success "..reg_id)
      else
         print "register failed"
      end
   else
      print "connect to Iris failed"
   end
end


local function waittimeout(timeout)
   if timeout then
      if zeus.sleep(timeout) then
         -- break by wakeup
         return true
      else
         -- timeout
         return false
      end
   else
      zeus.wait()
      return true
   end
end


function init()
   HTTP_DELETE = http.DELETE
   test = conf.test
   iris_ip = conf.iris.ip
   iris_port = conf.iris.port
   zeus.register_protocol {
      name = "text",
      id = zeus.PTYPE_TEXT,
      pack = function(...) return ... end
   }
   setmetatable(topic_readers, {__mode="kv"})
   read_buf = buffer.new()
   iris_parser = driver.httpparser()
   if not init_lock then
      init_lock = queue()
   end
   init_lock(function()
         if not aiexc then
            aiexc = zeus.newservice "aiexc"
         end
         if not iris_id then
            connect(iris_ip, iris_port)
         end
   end)
end


function response.msgReader(topic, idx)
   assert(topic_readers[topic] == nil)
   local reader = readerNew({})
   topic_readers[topic] = reader
   reader.topic = topic
   if idx==nil then
      watch(topic)
      idx = 0
      reader.cur = 0
   else
      watch(topic, 'id='..idx)
      reader.cur = idx
   end
   return readerClientNew(topic, reader, idx);
end


function response.streamReader(topic, idx)
   local reader = topic_readers[topic]
   if reader == nil then
      reader = readerNew(buffer.new())
      topic_readers[topic] = reader
      reader.topic = topic
      if idx==nil then
         watch(topic)
      else
         local buflen = reader.buf:len()
         assert(idx <= buflen)
         if idx == buflen then
            watch(topic, 'byte='..idx)
         end
      end
   end
   return readerClientNew(topic, reader, idx);
end


function accept.closeReader(id)
   local reader = id_readers[id]
   readerRelease(reader.reader)
   reader.topic = nil
   reader.reader = nil
   id_readers[id] = nil
end


function response.read(id, timeout)
   local cli = id_readers[id]
   if cli == nil then
      zeus.error("WARNNING: read after close")
      return
   end
   return cli.lock(function()
         -- double check
         cli = id_readers[id]
         if not cli then
            return true
         end
         local reader = cli.reader
         if not reader then
            return true
         end
         local buf = reader.buf
         if type(buf) == "table" then
            -- msg reader
            local mq = buf
            local idx = cli.offset
            assert(idx ~= nil)
            if mq[idx] then
               return reader.eof, mq[idx]
            else
               reader.waitlist = coroutine.running()
               if not waittimeout(timeout) then
                  reader.waitlist = nil
                  return "TIMEOUT"
               end
               local data = mq[idx]
               assert(reader.eof or data)
               if data then
                  cli.offset = idx + 1
               end
               return reader.eof, mq[idx]
            end
         else
            -- stream reader
            local buflen = buf:len()
            local offset = cli.offset
            if offset ~= nil and buflen > offset then
               cli.offset = buflen
               return reader.eof, buf:tailfromidx(offset)
            elseif reader.eof then
               if reader.error then return "BROKEN" else return true end
            else
               reader.waitlist[id] = coroutine.running()
               if not waittimeout(timeout) then
                  reader.waitlist[id] = nil
                  return "TIMEOUT"
               end
               if reader.error then
                  return "BROKEN"
               end
               local newdata = cli.newdata
               if newdata then
                  cli.newdata = nil
                  cli.offset = offset + string.len(newdata)
               end
               return reader.eof, newdata
            end
         end
   end)
end


function accept.write(topic, data)
   assert(iris_id)
   if not data then
      return
   end
   local data = driver.httppack("PUT", topic, data)
   socket.send(iris_id, data)
end


function accept.sendStop(id)
   assert(iris_id)
   local data = driver.httppack("PUT", "/stop")
end
