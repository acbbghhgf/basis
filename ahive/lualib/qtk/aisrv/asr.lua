local zeus = require "qtk.zeus"
local engine = require "zeus.engine"
local srvx = require "qtk.zeus.srvx"
local interface = require "qtk.aisrv.interface"
local dkjson = require "qtk.dkjson"
time = 1
hotwords = "hot.t"

return function (res, cfg)
--   time = time + 1
-- --  print("xxxxxtime == ", time)
--   if time > 50 then
--        srvx.queryservice('hot_words').req.download(hotwords)
--        time = 1
--   end
--
   local inst = {}
   if type(cfg) == 'table' then
      local ebnf = cfg.ebnf
      if ebnf then
         ec = srvx.queryservice('ebnf_dl').req.download(ebnf)
   --      print("xxxxxebnf====",ebnf)
         if ec then
            error(dkjson.encode({
                        errId = ec,
                        error = "ebnf net resource ["..ebnf.."] download failed"
                               }), 0) --hide error position
            return
         end
      end
   end

   inst.id = interface.getInstance(".asr."..res)
   inst.start = interface.start
   inst.feed = interface.feed
   inst.stop = interface.stop
   inst.getResult = function (i)
      local start_time = zeus.rawnow()
      local err, res = interface.getResult(i)
      if err == 1 then
         res = dkjson.decode(res)
         res['eng_dly'] = zeus.rawnow() - start_time
         res = dkjson.encode(res)
      end
      return err, res
   end
   return inst
end
