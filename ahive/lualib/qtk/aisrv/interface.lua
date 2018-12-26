local srvx = require "qtk.zeus.srvx"
local zeus = require "qtk.zeus"
local dkjson = require "qtk.dkjson"

local interface = {}
local aisrv = nil
local inst_pool = {}
local aisrv_start, aisrv_feed, aisrv_stop, aisrv_getResult,aisrv_release
setmetatable(inst_pool, {__mode="k"})


function interface.getInstance(name, cfg)
   if not aisrv then
      aisrv = srvx.queryservice("aisrv")
      aisrv_start = aisrv.post.start
      aisrv_feed = aisrv.post.feed
      aisrv_stop = aisrv.post.stop
      aisrv_getResult = aisrv.req.getResult
      aisrv_release = aisrv.post.release
   end
   local co = coroutine.running()
   local inst_co = inst_pool[co]
   if not inst_co then
      inst_co = {}
      inst_pool[co] = inst_co
   end
   local inst = inst_co[name]
   if not inst then
      local ec
      ec, inst = aisrv.req.getInstance(name, cfg)
      if ec ~= 0 then
         error(dkjson.encode({
                     errId = ec,
                     error = inst
                            }), 0) --hide error position
      end
      inst_co[name] = inst
   end
   return inst
end


function interface.start(self, args)
   assert(self.id)
   aisrv_start(self.id, args)
end


function interface.feed(self, data)
   assert(self.id)
   aisrv_feed(self.id, data)
end


function interface.stop(self)
   assert(self.id)
   aisrv_stop(self.id)
end


function interface.getResult(self)
   local ec, res = aisrv_getResult(self.id)
   -- 1 is EC_EOF
   -- 2 is empty result for asr
   if ec > 2 then
      error(dkjson.encode({
                  errId = ec,
                  error = res
                         }), 0) -- hide error position
   elseif ec == 2 then
      ec = 0
   end
   return ec, res
end


function interface.clear(co)
   if co == nil then
      co = coroutine.running()
   end
   if not inst_pool[co] then
      return
   end
   for name, id in pairs(inst_pool[co])
   do
      aisrv_release(id)
   end
   -- no need to set nil for coroutine multiplex, and inst_pool is weak table for key
   inst_pool[co] = {}
end


return interface
