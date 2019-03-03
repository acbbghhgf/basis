local zeus = require "qtk.zeus"
require "qtk.zeus.manager"
local srvx = require "qtk.zeus.srvx"
local socket = require "qtk.zeus.socket"
local engine = require "zeus.engine"
local buffer = require "zeus.buffer"
local reader = require "qtk.zeus.reader"
local writer = require "qtk.zeus.writer"
local asr = require "qtk.aisrv.asr"
local tts = require "qtk.aisrv.tts"
local entts = require "qtk.aisrv.entts"
local iris = require "zeus.iris"


local function test_http_parser()
   local parser = iris.httpparser()
   local conn = socket.connect("192.168.0.99", 80, function (...)
                                  local buf = buffer.new()
                                  buf:push_rawdata(...)
                                  repeat
                                     local ret, topic, data = parser:feed(buf)
                                     if ret == 2 then
                                        print (topic)
                                        print (data:pop(data:len()))
                                     end
                                  until ret <= 0
   end)
   socket.send(conn, "GET /statics HTTP/1.1\r\n\r\n")
   zeus.sleep(100)
end


zeus.start(function()
      hotwords = "hot.t"
      srvx.uniqueservice("ebnf_dl")
      srvx.uniqueservice("hot_words")
      srvx.uniqueservice("aisrv")
      srvx.uniqueservice("iris")
      srvx.queryservice("hot_words").req.download(hotwords)
      zeus.exit()
      local inst = entts("male")
      inst:start("{\"text\":\"computer\"}")
      local w = writer.open("iris://en.tts/result", true)
      repeat
         local eof, result = inst:getResult()
         if result then
            w:write(result)
         end
      until eof ~= 0
      w:close()
      zeus.exit()
      local cnt = 10000
      while cnt > 0
      do
         --cnt = cnt-1
      local inst = asr("comm", "")
      local r = reader.open("iris://1351742131/192.168.0.173:55070/5477/data", 0)
      local w = writer.open("iris://asr/result", true)
      inst:start("")
      repeat
         local eof, data = r:read()
         if data then
            inst:feed(data)
            data = nil
         end
      until eof
      r:close()
      inst:stop()
      local _, result = inst:getResult()
      w:write(result)
      -- break
      end
      print (result)
      zeus.abort()
      zeus.exit()
      local instname = (zeus.call(".asr.comm", "text", "hello"))
      local outbuf = buffer.new()
      local input = socket.open(instname..'i', socket.MODE_WRONLY)
      local output = socket.open(instname..'o', socket.MODE_RDONLY, function (...)
                                    engine.unpack(outbuf, ...)
                                    repeat
                                       local ret, packbuf = engine.parse2string(outbuf)
                                       if ret == 0 then
                                          -- print (packbuf)
                                       end
                                    until ret == nil
      end)
      zeus.sleep(1000)
      socket.send(input, engine.packcmd(0))
      local r = reader.open("../data/ni-hao.wav")
      local inbuf = buffer.new()
      reader.read(r, inbuf)
      socket.send(input, engine.packbuffer(1, inbuf))
      socket.send(input, engine.packcmd(2))
      socket.send(input, engine.packcmd(3))
      zeus.sleep(1000)
      zeus.exit()
end)
