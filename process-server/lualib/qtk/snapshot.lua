local save = require "snapshot"
local snapshot = {}

local dump = nil

local function dump(S1, S2)
   for k, v in pairs(S2) do
      if S1[k] == nil then
         print(k, v)
      end
   end
end

if save then
   print "snapshot is open"
   snapshot.save = save
   snapshot.dump = dump
else
   snapshot.save = function() return {} end
   snapshot.dump = function() end
end

return snapshot
