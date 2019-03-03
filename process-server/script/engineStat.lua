local zeus = require "qtk.zeus"
local srvx = require "qtk.zeus.srvx"

name = ...

zeus.start(function()
      local aisrv = srvx.queryservice("aisrv")
      print (aisrv.req.engine(name))
      zeus.exit()
end)
