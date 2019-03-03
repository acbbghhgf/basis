local zeus = require "qtk.zeus"
require "qtk.zeus.manager"
local srvx = require "qtk.zeus.srvx"
local socket = require "qtk.zeus.socket"
local http = require "zeus.http"
local buffer = require "zeus.buffer"
local conf = require "global"
local md5 = require "qtk.md5"

local err_code = nil

local ebnf_usr = nil
local httpc = nil
local http_ip = '192.168.0.179'
local http_port = 8000
local http_parser = nil
local root_dir = '/home/test/server/ahive/'
local hot_fn = "hot1.t"

local http_cb
local http_buf
local http_req


local function connect()
   httpc = socket.httpconn(http_ip, http_port, http_cb)
 --  print("ip, port==", http_ip, http_port)
end


local function verify(fn, sum)
   local f = io.open(fn, 'r')
   local md5obj = md5.new()
   while true
   do
      local buf = f:read(4096)
      if not buf then
         break
      end
      md5obj:update(buf)
   end
   f:close()
   local s = md5obj:hexdigest()
   return s == sum
end


function http_cb(msg, sz)
   if msg == nil then
      -- zeus.error "http closed by http server"
      print "hotwords http closed by http server"
      httpc = nil
      return
   end
   if msg == false then
      -- zeus.error "connect to http server failed"
      print "connect to http server failed"
      httpc = nil
      return
   end
   http_buf:push_rawdata(msg, sz)
   local t, code, body = http_parser:feed(http_buf)
   if t == 0 then
      return
   end
   if t == 2 then
      local ebnf = srvx.queryservice("ebnf_dl")
      if code == 200 then
         local f = io.open("hotwords.t", 'w')
         if not f then
            ebnf.post.error(http_req, err_code.engine_file_write)
            return
         end
         while true
         do
            local data = body:pop(4096)
            print("xoxoxdata === ", data)
            if #data > 0 then
               f:write(data)
            else
               break
            end
         end
         f:close()
         if verify(tmpfn, md5sum) then
            os.rename(tmpfn, fn)
         else
            os.remove(tmpfn)
            ebnf.post.error(http_req, err_code.engine_ebnf_dl_failed)
         end
         ebnf.post.success(http_req)
      elseif code == 404 then
         ebnf.post.error(http_req, err_code.engine_ebnf_not_found)
      else
         ebnf.post.error(http_req, err_code.engine_ebnf_dl_failed)
      end
   else
      zeus.error "unexpected response "
   end
end


function init()
   local hot = conf.hotword
   if type(hot) == 'table' then
      if hot.ip then
         http_ip = hot.ip
      end
      if hot.port then
         http_port = hot.port
      end
      if hot.fn then
         hot_fn = hot.fn
      end
      if hot.data_dir then
         root_dir = conf.hotword.data_dir
      end
      if string.sub(root_dir, #root_dir) ~= '/' then
         root_dir = root_dir .. '/'
      end
   end
   err_code = conf.err_code
   ebnf_usr = {}
   http_parser = http.parser()
   http_buf = buffer.new()
   connect()
end


local function download(res, co)
   http_req = res
   zeus.timeout(50000, function() local co = coroutine.running() download(res, co) end)
   local data = http.request('GET', '/api/v1/hot/' .. res, '', {
                                Host = http_ip..':'..http_port,
   })
   if not httpc then
      print 'reconnecting...'
      connect()
      if not httpc then
         print 'connect failed'
         return err_code.engine_ebnf_server_refused
      else
         print 'connected'
      end
   end
   httpc:request(data)
end


function accept.success(res)
   local usr = ebnf_usr[res]
   ebnf_usr[res] = true
   for co, _ in pairs(usr)
   do
      usr[co] = true
      zeus.wakeup(co)
   end
end


function accept.error(res, ec)
   local usr = ebnf_usr[res]
   ebnf_usr[res] = nil
   for co, _ in pairs(usr)
   do
      usr[co] = ec
      zeus.wakeup(co)
   end
end


function response.download(res)
   res = hot_fn
 --  print("xxxxxx1111xxxx")
   local co = coroutine.running()
   download(res, co)
--   zeus.sleep(3000)
end
