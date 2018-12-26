local zeus = require "qtk.zeus"
local engine = require "zeus.engine"
local srvx = require "qtk.zeus.srvx"
local interface = require "qtk.aisrv.interface"
local dkjson = require "qtk.dkjson"


return function (res, cfg)
   local inst = {}
   inst.id = interface.getInstance(".ifly."..res, cfg)
   inst.start = interface.start
   inst.feed = interface.feed
   inst.stop = interface.stop
   inst.getResult = interface.getResult
   return inst
end
