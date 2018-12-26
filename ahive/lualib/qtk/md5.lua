local c = require "md5.core"


local md5 = {}


md5.sum = c.sum
md5.hex = c.hex
md5.exor = c.exor
md5.crypt = c.crypt
md5.decrypt = c.decrypt
md5.new = c.new


return md5
