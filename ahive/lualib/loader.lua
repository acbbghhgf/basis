local args = {}
for word in string.gmatch(..., "%S+") do
	table.insert(args, word)
end

SERVICE_NAME = args[1]

local main, pattern

local err = {}
for pat in string.gmatch(LUA_SERVICE, "([^;]+);*") do
	local filename = pat.."/"..SERVICE_NAME..".lua"
	local f, msg = loadfile(filename)
	if not f then
		table.insert(err, msg)
	else
		pattern = pat
		main = f
		break
	end
end

if not main then
	error(table.concat(err, "\n\t"))
end

LUA_SERVICE = nil
package.path , LUA_PATH = LUA_PATH
package.cpath , LUA_CPATH = LUA_CPATH

-- local service_path = string.match(pattern, "(.*/)[^/?]+$")

-- if service_path then
-- 	service_path = string.gsub(service_path, "?", args[1])
-- 	package.path = service_path .. "?.lua;" .. package.path
-- 	SERVICE_PATH = service_path
-- else
-- 	local p = string.match(pattern, "(.*/).+$")
-- 	SERVICE_PATH = p
-- end
SERVICE_PATH = pattern

if LUA_PRELOAD then
	local f = assert(loadfile(LUA_PRELOAD))
	f(table.unpack(args))
	LUA_PRELOAD = nil
end

main(select(2, table.unpack(args)))
