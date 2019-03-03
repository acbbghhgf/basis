local zeus = require "qtk.zeus"
require "qtk.zeus.manager"
local srvx = require "qtk.zeus.srvx"

local cmd = {}
local service = {}


local function request(name, func, ...)
   local ok, handle = pcall(func, ...)
   local s = service[name]
   assert(type(s) == "table")
   if ok then
      service[name] = handle
   else
      service[name] = tostring(handle)
   end

   for _,v in ipairs(s) do
      zeus.wakeup(v)
   end

   if ok then
      return handle
   else
      error(tostring(handle))
   end
end

local function waitfor(name , func, ...)
   local s = service[name]
   if type(s) == "number" then
      return s
   end
   local co = coroutine.running()

   if s == nil then
      s = {}
      service[name] = s
   elseif type(s) == "string" then
      error(s)
   end

   assert(type(s) == "table")

   if not s.launch and func then
      s.launch = true
      return request(name, func, ...)
   end

   table.insert(s, co)
   zeus.wait()
   s = service[name]
   if type(s) == "string" then
      error(s)
   end
   assert(type(s) == "number")
   return s
end

local function read_name(service_name)
   if string.byte(service_name) == 64 then -- '@'
      return string.sub(service_name , 2)
   else
      return service_name
   end
end

function cmd.LAUNCH(service_name, subname, ...)
   local realname = read_name(service_name)

   if realname == "srvxd" then
      return waitfor(service_name.."."..subname, srvx.rawnewservice, subname, ...)
   else
      return waitfor(service_name, zeus.newservice, realname, subname, ...)
   end
end

function cmd.LAUNCH_MOCK(service_name, subname, ...)
   local realname = read_name(service_name)

   if realname == "srvxd" then
      return waitfor(service_name.."."..subname, srvx.rawnewservice, "mock"..subname, ...)
   else
      return waitfor(service_name, zeus.newservice, "mock"..realname, subname, ...)
   end
end

function cmd.QUERY(service_name, subname)
   local realname = read_name(service_name)

   if realname == "srvxd" then
      return waitfor(service_name.."."..subname)
   else
      return waitfor(service_name)
   end
end

function cmd.LIST()
   local result = {}
   for k,v in pairs(service) do
      if type(v) == "string" then
         v = "Error: " .. v
      elseif type(v) == "table" then
         v = "Querying"
      else
         v = zeus.address(v)
      end

      result[k] = v
   end

   return result
end


zeus.start(function()
      zeus.dispatch("lua", function(session, address, command, ...)
                         local f = cmd[command]
                         if f == nil then
                            zeus.ret(zeus.pack(nil, "Invalid command " .. command))
                            return
                         end

                         local ok, r = pcall(f, ...)

                         if ok then
                            zeus.ret(zeus.pack(r))
                         else
                            zeus.ret(zeus.pack(nil, r))
                         end
      end)
      local handle = zeus.localname ".service"
      if  handle then
         zeus.error(".service is already register by ", zeus.address(handle))
         zeus.exit()
      else
         zeus.register(".service")
      end
end)
