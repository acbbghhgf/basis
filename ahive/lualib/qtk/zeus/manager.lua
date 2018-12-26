local zeus = require "qtk.zeus"
local c = require "zeus.core"

function zeus.launch(...)
   local addr = c.command("LAUNCH", table.concat({...}," "))
   if addr then
      return tonumber("0x" .. string.sub(addr , 2))
   end
end

function zeus.kill(name)
   if type(name) == "number" then
      skynet.send(".launcher","lua","REMOVE",name, true)
      name = skynet.address(name)
   end
   c.command("KILL",name)
end

function zeus.abort()
   c.command("ABORT")
end

function zeus.register(name)
   c.command("REG", name)
end

function zeus.getname(handle)
   return c.command("GETNAME", handle)
end

function zeus.name(name, handle)
   c.command("NAME", name .. " " .. zeus.address(handle))
end

local dispatch_message = zeus.dispatch_message

function zeus.forward_type(map, start_func)
   c.callback(function(ptype, msg, sz, ...)
         local prototype = map[ptype]
         if prototype then
            dispatch_message(prototype, msg, sz, ...)
         else
            dispatch_message(ptype, msg, sz, ...)
            c.trash(msg, sz)
         end
              end, true)
   zeus.timeout(0, function()
                   zeus.init_service(start_func)
   end)
end

function zeus.filter(f ,start_func)
   c.callback(function(...)
         dispatch_message(f(...))
   end)
   zeus.timeout(0, function()
                     zeus.init_service(start_func)
   end)
end

-- function zeus.monitor(service, query)
--    local monitor
--    if query then
--       monitor = skynet.queryservice(true, service)
--    else
--       monitor = skynet.uniqueservice(true, service)
--    end
--    assert(monitor, "Monitor launch failed")
--    c.command("MONITOR", string.format(":%08x", monitor))
--    return monitor
-- end

return skynet
