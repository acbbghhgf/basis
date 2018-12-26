local srvx = require "qtk.zeus.srvx"


function init()
   print "mock aisrv init"
end


function response.getInstance(name, cfg)
   return 0
end


function accept.start(id, args)
end


function accept.feed(id, data)
end


function accept.stop(id)
end


function response.getResult(id)
   return 0, "hello world"
end


function accept.release(id)
end
