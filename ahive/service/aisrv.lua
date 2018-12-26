local zeus = require "qtk.zeus"
require "qtk.zeus.manager"
local srvx = require "qtk.zeus.srvx"
local queue = require "qtk.zeus.queue"
local buffer= require "zeus.buffer"
local engine= require "zeus.engine"
local socket = require "qtk.zeus.socket"
local dkjson = require "qtk.dkjson"
local conf = require "global"


local free_engine = {}
local used_engine = {}
local alloc_engine_id = 0


local EC_OK = 0
local EC_EOF = 1

local aisrv = {
   start = 0,
   feed = 1,
   stop = 2,
   get_result = 3,
}


local EC = nil


local function engineInit(name)
   zeus.launch("aisrv", name)
   free_engine["."..name] = {}
end


local function allocId(inst)
   local id = alloc_engine_id + 1
   while used_engine[id]
   do
      id = id + 1
   end
   inst.id = id
   used_engine[id] = inst
   alloc_engine_id = id
   inst.cnt = inst.cnt + 1
   inst.state = "alloc"
   return id
end


local function deleteInstance(inst)
   socket.close(inst.sid)
end


function init()
   EC = conf.err_code
   zeus.register_protocol {
      name = "text",
      id = zeus.PTYPE_TEXT,
      pack = function(...) return ... end,
      unpack = zeus.tostring,
      dispatch = function(session, address , cmd)
         print(session)
         print(address)
         print(cmd)
      end,
   }
   local fifo_dir = io.open("fifo")
   if fifo_dir then
      io.close(fifo_dir)
   else
      os.execute("mkdir fifo")
   end
   engineInit("asr.comm")
--   engineInit("tts.girl")
--   engineInit("tts.gxboy")
--   engineInit("en.tts.female")
--   engineInit("en.tts.male")
--   engineInit("ifly.chn")
--   engineInit("ifly.eng")
--   engineInit("usc.comm")
--   engineInit("eval.engsnt_v2")
--   engineInit("eval.engsnt_v3")
--   engineInit("eval.engsnt_v4")
--   engineInit("eval.engwrd_v2")
--   engineInit("eval.engwrd_v3")
   alloc_engine_id = 0
end


function exit()
   zeus.kill(".asr.comm")
--   zeus.kill(".tts.girl")
--   zeus.kill(".tts.gxboy")
--   zeus.kill(".en.tts.female")
--   zeus.kill(".en.tts.male")
--   zeus.kill(".ifly.chn")
--   zeus.kill(".ifly.eng")
--   zeus.kill(".usc.comm")
--   zeus.kill(".eval.engsnt_v2")
--   zeus.kill(".eval.engsnt_v3")
--   zeus.kill(".eval.engsnt_v4")
--   zeus.kill(".eval.engwrd_v2")
--   zeus.kill(".eval.engwrd_v3")
end


function response.getInstance(name, cfg)
   local engines = free_engine[name]
   if not engines then
      return EC.engine_not_found, string.format("engine [%s] not found", name)
   end
   while true
   do
      local inst = table.remove(engines)
      if not inst then
         break
      end
      if inst.id then
         return 0, allocId(inst)
      else
         -- engine may be killed when free
         deleteInstance(inst)
      end
   end
   if cfg == nil then
      cfg = ""
   end
   local instname = zeus.call(name, "text", cfg)
   if string.sub(instname, 1, 5) == 'ERROR' then
      return EC.engine_create_failed, instname
   end
   local inst = {}
   inst.buf = buffer.new()
   local function rcb(msg, sz)
      if not msg and inst.id then
         -- pipe broken
         if inst.co then
            inst.ret = EC.engine_pipe_broken
            inst.pack = "Broken Pipe"
            zeus.wakeup(inst.co)
            inst.co = nil
         end
         inst.id = nil
         return
      end
      engine.unpack(inst.buf, msg, sz)
      repeat
         local ret, pack = engine.parse2string(inst.buf)
         if pack or ret == 1 or ret == 2 then
            inst.ret = ret
            inst.pack = pack
            if inst.co then
               zeus.wakeup(inst.co)
               inst.co = nil
            else
               print ("no coroutine to resume for "..instname)
               print ("\tdata: "..pack)
            end
         end
      until ret == nil
   end
   inst.sid = socket.open("unix:"..instname, socket.MODE_RDWR, rcb)
   inst.engine = engines
   inst.name = instname
   inst.cnt = 0
   return 0, allocId(inst)
end


function accept.start(id, args)
   local inst = used_engine[id]
   if not inst then
      return
   end
   local data
   if args then
      data = engine.packstring(aisrv.start, args)
   else
      data = engine.packcmd(aisrv.start)
   end
   socket.send(inst.sid, data)
   inst.state = "start"
end


function accept.feed(id, data)
   local inst = used_engine[id]
   if not inst then
      return
   end
   socket.send(inst.sid, engine.packstring(aisrv.feed, data))
   inst.state = "feed"
end


function accept.stop(id)
   local inst = used_engine[id]
   if not inst then
      return
   end
   socket.send(inst.sid, engine.packcmd(aisrv.stop))
   inst.state = "stop"
end


function response.getResult(id)
   local inst = used_engine[id]
   if not inst or not inst.id then
      return EC.engine_pipe_broken, "Broken Pipe"
   end
   inst.co = coroutine.running()
   socket.send(inst.sid, engine.packcmd(aisrv.get_result))
   inst.state = "wait result"
   if not zeus.sleep(3000) then
      -- timeout
      inst.co = nil
      -- mark to delete when release
      inst.id = nil
      inst.state = "result timeout"
      return EC.engine_response_timeout, "Engine Response Timeout"
   end
   inst.state = "got result"
   return inst.ret, inst.pack
end


function response.getCount(id)
   local inst = used_engine[id]
   if inst then
      return inst.cnt
   end
end


function accept.release(id, del)
   local inst = used_engine[id]
   -- support duplicate release
   if inst then
      used_engine[id] = nil
      if del or inst.id == nil or inst.state ~= "got result" then
         deleteInstance(inst)
      else
         table.insert(inst.engine, inst)
         inst.state = "idle"
      end
   end
end


function response.engine(name)
   local engine = free_engine[name]
   if not engine then
      return ""
   end
   local free = {}
   local work = {}
   local bad = {}
   for id, inst in pairs(engine)
   do
      table.insert(free, inst.name)
   end
   for id, inst in pairs(used_engine)
   do
      if inst.engine == engine then
         if inst.id then
            work[inst.name] = inst.state
         else
            bad[inst.name] = inst.state
         end
      end
   end
   local data = {
      free = free,
      work = work,
      bad = bad
   }
   return dkjson.encode(data)
end


function response.engineWakeup(name, instname)
   local engine = free_engine[name]
   if not engine then
      return "no such engine"
   end
   for id, inst in pairs(used_engine)
   do
      if inst.id and inst.name == instname then
         if inst.co then
            zeus.wakeup(inst.co)
            return "wakeup "..instname
         else
            return "no need to wakeup "..instname
         end
      else
         return instname.." is bad"
      end
   end
   return instname.." not found"
end
