local zeus = require "qtk.zeus"
local srvx = require "qtk.zeus.srvx"

local name, instname = ...

zeus.start(function()
      local aisrv = srvx.queryservice("aisrv")
      print (aisrv.req.engineWakeup(name, instname))
      zeus.exit()
end)
