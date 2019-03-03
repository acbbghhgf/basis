label = {
optional = 1,
required = 2,
repeated = 3,
}

desctype = {
double = 1,
foat = 2,
int64 = 3,
uint64 = 4,
int32 = 5,
fixed64 = 6,
fixed32 = 7,
bool = 8,
string = 9,
group = 10,
message = 11,
bytes = 12,
uint32 = 13,
enum = 14,
sfixed32 = 15,
sfixed64 = 16,
sint32 = 17,
sint64 = 18,
}


pb_person = {}
pb_person.id = {1, desctype.int32, label.optional}
pb_person.name = {2, desctype.string, label.optional}
pb_person.person = {3, pb_person, label.optional}
pb_person.son_id = {4, desctype.int32, label.repeated}


pb_allperson = {
person = {1, pb_person, label.repeated},
}

function var_dump(data, max_level, prefix)
if type(prefix) ~= "string" then
prefix = ""
end
if type(data) ~= "table" then
print(prefix .. tostring(data))
else
if max_level ~= 0 then
local prefix_next = prefix .. "    "
print(prefix .. "{")

for k,v in pairs(data) do
print(prefix_next .. k .. " = ")
if type(v) ~= "table" or (type(max_level) == "number" and max_level <= 1) then
print(v)
else

if max_level == nil then
var_dump(v, nil, prefix_next)
else
var_dump(v, max_level - 1, prefix_next)
end
end
end
print(prefix .. "}")

end
end
end

enc, dec, v = ...
local t = dec(v, pb_allperson)
-- var_dump(t, 5)
return enc(t, pb_allperson)
