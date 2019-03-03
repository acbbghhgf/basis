local zeus = require "qtk.zeus"
require "qtk.zeus.manager"
local srvx = require "qtk.zeus.srvx"
local socket = require "qtk.zeus.socket"
local buffer = require "zeus.buffer"
local http = require "zeus.http"
local conf = require "global"

local http_parser = http.parser
local http_response = http.response
local traceback = debug.traceback

local CMD = {}

local srv_id
local clients = {}


local function split_cmdline(cmdline)
   local split = {}
   for i in string.gmatch(cmdline, "%S+") do
      table.insert(split, i)
   end
   return split
end


function CMD.launch(cmdline)
   local split = split_cmdline(cmdline)
   local command = split[1]
   if command == "srvx" then
      return xpcall(srvx.newservice, traceback, select(2, table.unpack(split)))
   elseif cmdline ~= "" then
      return xpcall(zeus.newservice, traceback, cmdline)
   end
end


function CMD.kill(name)
   return xpcall(zeus.kill, traceback, name)
end


function CMD.engstat(name)
   local aisrv = srvx.queryservice("aisrv");
   return true, aisrv.req.engine(name)
end


local function client_feed(id, parser, buf)
   local ret, url, data = parser:feed(buf)
   if ret == 1 then
      local cmd, resp
      if string.sub(url, 1, 5) == '/api/' then
         cmd = string.sub(url, 6)
      else
         cmd = string.sub(url, 2)
      end
      local func = CMD[cmd]
      if func then
         local err, msg = func(data:tostring())
         if err then
            if not msg then msg = "OK" end
            resp = http_response(200, msg)
         else
            resp = http_response(500, msg)
         end
      end
      if not resp then
         resp = http_response(404, "you are lost.:)")
      end
      socket.send(id, resp)
   end
end


local function client_rcb(id, msg, sz)
   local cli = clients[id]
   if not cli then return end
   local parser = cli.parser
   local buf = cli.buf
   if not parser then return end
   if msg then
      buf:push_rawdata(msg, sz);
      client_feed(id, parser, buf)
   else
      -- closed
      clients[id] = nil
   end
end


local function new_conn(sid, id)
   if not sid then
      -- listen failed
      socket.close(sid)
      srv_id = nil
      return
   end
   local buf = socket.callback(id, function (msg, sz)
                                  client_rcb(id, msg, sz)
   end)
   local parser = http_parser()
   if buf then
      client_feed(id, parser, buf)
   else
      buf = buffer.new()
   end
   clients[id] = {
      parser = parser,
      buf = buf
   }
end


zeus.start(function()
      srv_id = socket.listen(conf.remote_console.listen,
                             conf.remote_console.port,
                             new_conn,
                             conf.remote_console.backlog)
end)
