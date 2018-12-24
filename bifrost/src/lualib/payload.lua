local pb = require "protobuf"
local json = require "dkjson"

local payload = {}

local label = {
    optional = 1,
    required = 2,
    repeated = 3,
}

local desctype = {
    double   = 1,
    foat     = 2,
    int64    = 3,
    uint64   = 4,
    int32    = 5,
    fixed64  = 6,
    fixed32  = 7,
    bool     = 8,
    string   = 9,
    group    = 10,
    message  = 11,
    bytes    = 12,
    uint32   = 13,
    enum     = 14,
    sfixed32 = 15,
    sfixed64 = 16,
    sint32   = 17,
    sint64   = 18,
}

local audio = {
    audioType   = {1, desctype.string, label.optional},
    sampleRate  = {2, desctype.uint32, label.optional},
    sampleBytes = {3, desctype.uint32, label.optional},
    channel     = {4, desctype.uint32, label.optional},
}

local param = {
    text      = {1, desctype.string, label.optional},
    res       = {2, desctype.string, label.optional},
    useStream = {3, desctype.bool, label.optional},
    volume    = {4, desctype.float, label.optional},
    speed     = {5, desctype.float, label.optional},
    pitch     = {6, desctype.float, label.optional},

    env       = {7, desctype.string, label.optional},
    semRes    = {8, desctype.string, label.optional},
    ebnfRes   = {9, desctype.string, label.optional},
    iType     = {10, desctype.enum, label.optional},
}

local desc = {
    audio = {1, audio, label.optional},
    param = {2, param, label.optional},
}

local extra = {
    hook     = {1, desctype.string, label.optional},
    appId    = {2, desctype.string, label.optional},
    deviceId = {3, desctype.string, label.optional},
    token    = {4, desctype.string, label.optional},
}

local schemaAct = {"START", "APPEND", "END"}

local schema = {
    act   = {1, desctype.enum, label.required}, -- start/append/end
    obj   = {2, desctype.string, label.optional}, -- coreType/audio/nil
    desc  = {3, desc, label.optional},            -- param/audio/nil
    data  = {4, desctype.bytes, label.optional},  -- audio
    extra = {5, extra, label.optional},
}

local respSchema = {
    type = {1, desctype.string, label.required},
    bin  = {2, desctype.bytes, label.optional},
    dlg  = {3, desctype.string, label.optional},
    isEnd = {4, desctype.bool, label.optional},
}

function payload.pack(data)
    return pb.pack(data, schema)
end

function payload.unpack(data)
    local p = pb.unpack(data, schema)
    p.act = schemaAct[p.act + 1]
    if p.act == nil then
        p.act = "UNKNOWN"
    end
    return p
end

function payload.packResp(resp)
    return pb.pack(resp, respSchema)
end

return payload
