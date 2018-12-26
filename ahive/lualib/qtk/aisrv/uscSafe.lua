local zeus = require "qtk.zeus"
local engine = require "zeus.engine"
local srvx = require "qtk.zeus.srvx"
local interface = require "qtk.aisrv.interface"
local dkjson = require "qtk.dkjson"


local function traceback(msg)
   zeus.error(debug.traceback(msg))
   return msg
end


return function (res, cfg)
   local inst = {}
   local ok, id = xpcall(interface.getInstance, traceback, ".usc."..res, cfg)
   if ok then
      inst.id = id
      inst.start = interface.start
      inst.feed = interface.feed
      inst.stop = interface.stop
      inst.getResult = interface.getResult
      return true, inst
   else
      return false, id
   end
end
