local srvx = require "qtk.zeus.srvx"
local dkjson = require "qtk.dkjson"
local conf = require "global"

local reader = {}
local iris = nil


local function readerRelease(r)
   if r.id and iris then
      iris.post.closeReader(r.id)
      r.id = nil
   end
end


local reader_meta = {__gc=readerRelease, __index=reader}


local function openIrisReader(topic, offset, isQueue)
   if not iris then
      iris = srvx.queryservice("iris")
   end
   local id
   if isQueue then
      id = iris.req.msgReader(topic, offset)
   else
      id = iris.req.streamReader(topic, offset)
   end
   if id then
      local r = {
         id = id
      }
      setmetatable(r, reader_meta)
      return r
   end
end


function reader.open(name, offset, isQueue)
   if string.sub(name, 1, 7) == 'iris://' then
      return openIrisReader(string.sub(name, 7), offset, isQueue)
   else
      return openIrisReader(name, offset, isQueue)
   end
end


function reader.close(r)
   readerRelease(r)
end


function reader.read(r, timeout)
   if r.id == nil then
      -- reader is already closed, return eof
      print "******bug*********"
      return true
   end
   if timeout == nil then
      -- default timeout 10s
      timeout = 10000
   end
   local eof, data = iris.req.read(r.id, timeout)
   if eof == "TIMEOUT" then
      error(dkjson.encode({
                  errId = conf.err_code.read_input_timeout,
                  error = "read timeout"
      }), 0)
   elseif eof == "BROKEN" then
      error(false, 0)
   else
      return eof, data
   end
end


return reader
