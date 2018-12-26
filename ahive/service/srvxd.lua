local zeus = require "qtk.zeus"
local c = require "zeus.core"
local srvx_interface = require "qtk.zeus.srvx.interface"
--local profile = require "skynet.profile"
local srvx = require "qtk.zeus.srvx"

local srvx_name = tostring(...)
local func = srvx_interface(srvx_name, _ENV)
--local snax_path = pattern:sub(1,pattern:find("?", 1, true)-1) .. srvx_name ..  "/"
--package.path = snax_path .. "?.lua;" .. package.path

SERVICE_NAME = srvx_name
--SERVICE_PATH = snax_path

-- local profile_table = {}

-- local function update_stat(name, ti)
--    local t = profile_table[name]
--    if t == nil then
--       t = { count = 0,  time = 0 }
--       profile_table[name] = t
--    end
--    t.count = t.count + 1
--    t.time = t.time + ti
-- end

local traceback = debug.traceback

local function return_f(f, ...)
   return zeus.ret(zeus.pack(f(...)))
end

local function timing( method, ... )
   local err, msg
   --profile.start()
   if method[2] == "accept" then
      -- no return
      err,msg = xpcall(method[4], traceback, ...)
   else
      err,msg = xpcall(return_f, traceback, method[4], ...)
   end
   --local ti = profile.stop()
   --update_stat(method[3], ti)
   assert(err,msg)
end

zeus.start(function()
      local init = false
      local function dispatcher( session , source , id, ...)
         local method = func[id]

         if method[2] == "system" then
            local command = method[3]
            if command == "hotfix" then
               local hotfix = require "snax.hotfix"
               zeus.ret(zeus.pack(hotfix(func, ...)))
            elseif command == "init" then
               assert(not init, "Already init")
               local initfunc = method[4] or function() end
               initfunc(...)
               zeus.ret()
               init = true
            else
               assert(init, "Never init")
               assert(command == "exit")
               local exitfunc = method[4] or function() end
               exitfunc(...)
               zeus.ret()
               init = false
               zeus.exit()
            end
         else
            assert(init, "Init first")
            timing(method, ...)
         end
      end
      zeus.dispatch("srvx", dispatcher)

      -- set lua dispatcher
      function srvx.enablecluster()
         zeus.dispatch("lua", dispatcher)
      end
end)
