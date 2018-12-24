local service = {
    ["en.tts"]      = "en.tts",
    ["cn.tts"]      = "cn.tts",
}

service.defaultRes  = {
    ["cn.tts"]      = "girl",
    ["en.tts"]      = "female",
}

service.templates = {
    ["cn.tts"] = [[ERRPATH = $ETOPIC
local inst = tts($RES)
inst:start($PARAM)
local w = writer.open($WTOPIC, $APPEND0)
repeat
    local eof, result = inst:getResult()
    if result then w:write(result) end
until eof ~= 0
w:close()]],

    ["en.tts"] = [[ERRPATH = $ETOPIC
local inst = entts($RES)
inst:start($PARAM)
local w = writer.open($WTOPIC, $APPEND0)
repeat
    local eof, result = inst:getResult()
    if result then w:write(result) end
until eof ~= 0
w:close()]],
}

return service
