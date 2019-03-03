local proxy = {}
local core = require "proxy.core"

function proxy.now()
    return core.now()
end

function proxy.genUUID()
    return core.genUUID()
end

function proxy.sepString(s, sep, n)
    local tmp = {}
    string.gsub(s, string.format("[^%s]+", sep), function(w) table.insert(tmp, w) end)
    return table.unpack(tmp, 1, n)
end

function proxy.timeout(ti, handler)
    return core.timeout(ti, handler)
end

return proxy
