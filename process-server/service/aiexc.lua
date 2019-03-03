local zeus = require "qtk.zeus"
local json = require "qtk.dkjson"
local srvx = require "qtk.zeus.srvx"
local aisrv_interface = require "qtk.aisrv.interface"
local conf = require "global"

local iris = nil

-- needed by ai script's exc_G
local reader = require "qtk.zeus.reader"
local writer = require "qtk.zeus.writer"
local asr = require "qtk.aisrv.asr"
local tts = require "qtk.aisrv.tts"
local entts = require "qtk.aisrv.entts"
local ifly = require "qtk.aisrv.ifly"
local usc = require "qtk.aisrv.usc"
local uscSafe = require "qtk.aisrv.uscSafe"
local eval = require "qtk.aisrv.eval"

local engine_error_unknown = conf.err_code.engine_unknown
local engine_success = 0
local engine_restart_errid = {
   [conf.err_code.engine_create_failed] = true
}


local function traceback(msg)
   zeus.error(debug.traceback(msg))
   return msg
end


local function script_exc(id, script)
   if not iris then
      iris = srvx.queryservice("iris")
   end

   local exc_G = {
      reader = reader,
      writer = writer,
      asr = asr,
      tts = tts,
      entts = entts,
      ifly = ifly,
      usc = usc,
      uscSafe = uscSafe,
      eval = eval,
   }

   local errid = engine_error_unknown
   local need_restart = nil
   local f, err = load(script, id, "bt", exc_G)
   if not f then
      print(err)
   else
      local ok, err = xpcall(f, traceback)
      -- err may be false if reader broken
      if not ok and type(err) == 'string' then
         local err_data = json.decode(err)
         if type(err_data) == 'table' and err_data.errId then
            errid = err_data.errId
            need_restart = engine_restart_errid[errid]
            if need_restart then
               err = nil
            end
         else
            err = json.encode({
                  errId = errid,
                  error = err
            })
         end
         if exc_G.ERRPATH then
            iris.post.write(exc_G.ERRPATH, err)
         end
      else
         errid = engine_success
      end
      aisrv_interface.clear(coroutine.running())
   end
   if need_restart then
      iris.post.write("/restart", string.pack('zJ', id, errid))
   else
      iris.post.write("/stop.v2", string.pack('zJ', id, errid))
   end
end


zeus.register_protocol {
   name = "text",
   id = zeus.PTYPE_TEXT,
   unpack = zeus.tostring,
   dispatch = function(session, address, data)
      -- print (data)
      data = json.decode(data)
      if data.script and data.id then
         script_exc(data.id, data.script)
      end
   end
}


zeus.start(function()
end)
