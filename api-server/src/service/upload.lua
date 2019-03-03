package.path = "lualib/?.lua;lualib/?/init.lua"
package.cpath = "./luaclib/?.so"
package.ppath = LUA_PPATH
LUA_PPATH = nil

local msgpack = require "cmsgpack"
local send = require "senddata"

function loadfilprocesser(msg)
    -- print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    -- print(">>>>>>msg_len>>>>>", string.len(msg))
    -- print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    local boundary = "----QdreamerBfrxUploader"
    local tablemsg = msgpack.unpack(msg)
    local id = tablemsg.id
    local bodymsg = tablemsg.sendbody
    local host = tablemsg.sendhost    
    local ret = send.xfer("POST", "/api/v1/info/data/", {
                        addr = id,
                        body = bodymsg,
                        field = {
                            ["Content-Type"] = "multipart/form-data;boundary=" .. boundary,
                            Host = host,
                        },
                    })
    if not ret then 
        print "send error"
        print("send error!!!")
    end
end
