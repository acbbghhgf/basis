local bifrost = require "bifrost"
local xfer = require "xfer.core"
local json = require "dkjson"
local berror = {}

local typeEid    = {
    AUTH       = 40000,
    CINFO      = 40001,
    REQ        = 40002,
    LUA        = 40003,
    PLINK      = 40004,
    PERMISSION = 40005,
    NET        = 40006,
    SEMDLG     = 40007,
    TIMEOUT    = 40008,
    SESSION    = 40009,
    UNKNOWN    = 40010,
    UNFINISH   = 40011,
    UNDEFINED  = 40012,
    REP        = 40013,
}

local sfmt = string.format

function berror.sendError(t, errMsg, caddr, hook, device, sn)
    local errId = typeEid[t] or typeEid.UNDEFINED
    local err = json.encode({errId=errId,error=errMsg})
    local c = {
        addr = caddr,
        body = err,
        field = {["Content-Type"] = "text/plain",
                    hook = hook,
                    device = device,
                    ["seq-no"] = sn,
                },
    }
    if not xfer.xfer(500, c) then bifrost.debug("xfer error", err) end
    return err
end

function berror.redirectError(err, caddr, hook, device, sn)
    local c = {
        addr = caddr,
        body = err,
        field = { ["Content-Type"] = "text/plain",
                     hook = hook,
                     device = device,
                    ["seq-no"] = sn,
                },
    }
    if not xfer.xfer(500, c) then bifrost.debug("xfer error") end
    return err
end

function berror.getError(t, errMsg)
    local errId = typeEid[t] or typeEid.UNDEFINED
    return json.encode({errId=errId,error=errMsg})
end

return berror
