local service = {
    ["en.eval.snt"] = "en.eval",
    ["en.eval.wrd"] = "en.eval",
    ["cn.asr.rec"]  = "cn.asr.rec",
    ["en.asr.rec"]  = "en.asr.rec",
    -- ["en.asr.rec"]  = "ifly",
    ["cn.usc.rec"]  = "usc",
    ["cn.asr.mix"]  = "mix",
}

service.defaultRes = {
    ["en.eval.snt"] = "engsnt_v2",
    ["en.eval.wrd"] = "engsnt_v2",
    ["cn.asr.rec"]  = "comm",
    ["en.asr.rec"]  = "eng",
    ["cn.usc.rec"]  = "comm",
    ["cn.asr.mix"]  = "comm",
}
--hotwords=$HOTWORDS
service.templates = {
    ["cn.asr.rec"] = [[ERRPATH = $ETOPIC
local inst = asr($RES, {ebnf=$EBNFRES})
local r = reader.open($RTOPIC, 0)
local w = writer.open($WTOPIC)
inst:start($PARAM)
repeat
    local eof, data = r:read()
    if data then
        inst:feed(data)
        local _, result = inst:getResult()
        w:write(result)
    end
until eof
inst:stop()
local _, result = inst:getResult()
w:write(result)]],

    ["en.asr.rec"] = [[ERRPATH = $ETOPIC
local inst = asr($RES)
local r = reader.open($RTOPIC, 0)
local w = writer.open($WTOPIC)
inst:start($PARAM)
repeat
    local eof, data = r:read()
    if data then
        inst:feed(data)
        local _, result = inst:getResult()
        w:write(result)
    end
until eof
inst:stop()
local _, result = inst:getResult()
w:write(result)]],

    ["cn.usc.rec"] = [[ERRPATH = $ETOPIC
local inst = usc($RES)
local r = reader.open($RTOPIC, 0)
local w = writer.open($WTOPIC)
inst:start($PARAM)
repeat
    local eof, data = r:read()
    if data then
        inst:feed(data)
        local _, result = inst:getResult()
        w:write(result)
    end
until eof
inst:stop()
local _, result = inst:getResult()
w:write(result)]],

    ["en.eval.snt"] = [[ERRPATH = $ETOPIC
local inst = eval($RES)
local r = reader.open($RTOPIC, 0)
local w = writer.open($WTOPIC)
inst:start($PARAM)
repeat
    local eof, data = r:read()
    if data then
        inst:feed(data)
        local _, result = inst:getResult()
        w:write(result)
    end
until eof
inst:stop()
local _, result = inst:getResult()
w:write(result)]],

    ["en.eval.wrd"] = [[ERRPATH = $ETOPIC
local inst = eval($RES)
local r = reader.open($RTOPIC, 0)
local w = writer.open($WTOPIC)
inst:start($PARAM)
repeat
    local eof, data = r:read()
    if data then
        inst:feed(data)
        local _, result = inst:getResult()
        w:write(result)
    end
until eof
inst:stop()
local _, result = inst:getResult()
w:write(result)]],

    ["cn.asr.mix"] = [[ERRPATH = $ETOPIC
local succ, inst = uscSafe($RES)
if not succ then
    inst = asr($RES, {ebnf=$EBNFRES})
end
local r = reader.open($RTOPIC, 0)
local w = writer.open($WTOPIC)
inst:start($PARAM)
repeat
    local eof, data = r:read()
    if data then
        inst:feed(data)
        local _, result = inst:getResult()
        w:write(result)
    end
until eof
inst:stop()
local _, result = inst:getResult()
w:write(result)]],
}

return service
