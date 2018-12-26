local zeus = require "qtk.zeus"
require "qtk.zeus.manager"
require "qtk.zeus.srvx"

zeus.start(function()
      local launch = assert(zeus.launch("zlua", "launcher"))
      zeus.name(".launcher", launch)
      -- zeus.newservice("console")
      zeus.newservice("remote_console")

      zeus.newservice "service_mgr"
      zeus.newservice "main"
      --pcall(zeus.newservice, "main")
      print "bootstrap finish"
      zeus.exit()
end)
