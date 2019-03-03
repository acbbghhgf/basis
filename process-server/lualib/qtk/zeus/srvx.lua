local zeus = require "qtk.zeus"
local srvx_interface = require "qtk.zeus.srvx.interface"

local srvx = {}
local typeclass = {}

local G = {require = function() end}


zeus.register_protocol {
   name = "srvx",
   id = zeus.PTYPE_SRVX,
   pack = zeus.pack,
   unpack = zeus.unpack,
}


function srvx.register(name)
   if typeclass[name] then
      return typeclass[name]
   end

   local si = srvx_interface(name, G)

   local ret = {
      name = name,
      accept = {},
      response = {},
      system = {},
   }

   for _, v in ipairs(si) do
      local id, group, name, f = table.unpack(v)
      ret[group][name] = id
   end

   typeclass[name] = ret
   return ret
end

local zeus_send = zeus.send
local zeus_call = zeus.call

local function gen_post(type, handle)
   return setmetatable({}, {
         __index = function(t, k)
            local id = type.accept[k]
            if not id then
               error(string.format("post %s:%s no exist", type.name, k))
            end
            return function(...)
               zeus_send(handle, "srvx", id, ...)
            end
         end
   })
end

local function gen_req(type, handle)
   return setmetatable({}, {
         __index = function(t, k)
            local id = type.response[k]
            if not id then
               error(string.format("request %s:%s no exist", type.name, k))
            end
            return function(...)
               return zeus_call(handle, "srvx", id, ...)
            end
         end
   })
end

local handle_cache = setmetatable({}, {__mode = "kv"})

function srvx.rawnewservice(name, ...)
   local t = srvx.register(name)
   local handle = zeus.newservice("srvxd", name)
   assert(handle_cache[handle] == nil)
   if t.system.init then
      zeus_call(handle, "srvx", t.system.init, ...)
   end
   return handle
end

local meta = { __tostring = function(v)
                  return string.format("[%s:%x]", v.type, v.handle)
             end}
local function wrapper(handle, name, type)
   return setmetatable({
         post = gen_post(type, handle),
         req = gen_req(type, handle),
         type = name,
         handle = handle,
   }, meta)
end

function srvx.bind(handle, name)
   local ret = handle_cache[handle]
   if ret then
      assert(ret.type == name)
      return ret
   end
   local t = srvx.register(name)
   ret = wrapper(handle, name, t)
   handle_cache[handle] = ret
   return ret
end

function srvx.newservice(name, ...)
   local handle = srvx.rawnewservice(name, ...)
   return srvx.bind(handle, name)
end

function srvx.uniqueservice(name)
   local handle = assert(zeus_call(".service", "lua", "LAUNCH", "srvxd", name))
   return srvx.bind(handle, name)
end

function srvx.uniqueservice_mock(name)
   local handle = assert(zeus_call(".service", "lua", "LAUNCH_MOCK", "srvxd", name))
   return srvx.bind(handle, name)
end

function srvx.queryservice(name)
   local handle = assert(zeus_call(".service", "lua", "QUERY", "srvxd", name))
   return srvx.bind(handle, name)
end

function srvx.kill(obj, ...)
   local t = srvx.register(obj.type)
   zeus_call(obj.handle, "srvx", t.system.exit, ...)
end

function srvx.self()
   return srvx.bind(zeus.self(), SERVICE_NAME)
end

function srvx.exit(...)
   srvx.kill(srvx.self(), ...)
end

function srvx.printf(fmt, ...)
   zeus.error(string.format(fmt, ...))
end

return srvx
