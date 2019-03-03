local zeus = require "qtk.zeus"
require "qtk.zeus.manager"

handle = ...

zeus.start(function()
      print(zeus.getname(handle))
      zeus.exit()
end)
