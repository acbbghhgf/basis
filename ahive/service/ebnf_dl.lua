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
local http_ip = '101.37.172.108'
local http_port = 8000
local http_parser = nil
local root_dir = '/data/qtk/local/ebnf/'

local http_cb
local http_buf
local http_req


local function connect()
   httpc = socket.httpconn(http_ip, http_port, http_cb)
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
      print "http closed by http server"
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
         local md5sum = http_parser:getheader('content-md5')
         if md5sum then
            local fn = root_dir..http_req
            local tmpfn = fn..'.tmp'
            local f = io.open(tmpfn, 'w')
            if not f then
               ebnf.post.error(http_req, err_code.engine_file_write)
               return
            end
            while true
            do
               local data = body:pop(4096)
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
         else
            ebnf.post.error(http_req, err_code.engine_ebnf_dl_failed)
         end
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
   local ebnf = conf.ebnf
   if type(ebnf) == 'table' then
      if ebnf.ip then
         http_ip = ebnf.ip
      end
      if ebnf.port then
         http_port = ebnf.port
      end
      if ebnf.data_dir then
         root_dir = conf.ebnf.data_dir
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
   local fn = root_dir..res
   local f = io.open(fn, 'r')
   if f then
      f:close()
      ebnf_usr[res][co] = true
      ebnf_usr[res] = true
      return true
   end
   http_req = res
   local data = http.request('GET', '/api/v1/ebnf/'..res, '', {
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
   local usr = ebnf_usr[res]
   if usr == true then
      return
   end

   local co = coroutine.running()
   if usr == nil then
      usr = {}
      ebnf_usr[res] = usr
      local ec = download(res, co)
      if ec == true then
         return
      end
      if ec then
         return ec
      end
   end

   usr[co] = err_code.engine_ebnf_dl_unknown
   if zeus.sleep(3000) then
      -- wakeup
      if usr[co] ~= true then
         return usr[co]
      end
   else
      -- timeout
      usr[co] = nil
      return err_code.engine_ebnf_dl_timeout
   end
end
