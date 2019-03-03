local zeus = require "qtk.zeus"
local engine = require "zeus.engine"
local srvx = require "qtk.zeus.srvx"
local interface = require "qtk.aisrv.interface"


return function (res, cfg)
   local inst = {}
   inst.id = interface.getInstance(".en.tts."..res, cfg)
   inst.start = interface.start
   inst.getResult = interface.getResult
   return inst
end
