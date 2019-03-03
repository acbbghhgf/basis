local srvx = require "qtk.zeus.srvx"

local writer = {}
local irisWriter = {}
local iris = nil


local function writerRelease(w)
   if w.topic and w.isStream and iris then
      iris.post.write(w.topic, "")
      w.topic = nil
   end
end


local iris_meta = {__gc=writerRelease, __index=irisWriter}


local function openIrisWriter(topic, isStream)
   local w = {
      topic = topic,
      isStream = isStream
   }
   setmetatable(w, iris_meta)
   return w
end


function writer.open(topic, isStream)
   if string.sub(topic, 1, 7) == "iris://" then
      return openIrisWriter(string.sub(topic, 7), isStream)
   else
      return openIrisWriter(topic, isStream)
   end
end


function irisWriter.close(w)
   writerRelease(w)
end


function irisWriter.write(w, data)
   if w.topic == nil then
      -- writer is already closed, return failure
      return false
   end
   if not iris then
      iris = srvx.queryservice("iris")
   end
   if data and #data then
      iris.post.write(w.topic, data)
   end
end


return writer
