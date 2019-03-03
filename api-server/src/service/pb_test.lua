local pb = require "protobuf"

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
pb_person.per = {3, pb_person, label.optional}
pb_person.son_id = {4, desctype.int32, label.repeated}

pb_allperson = {
    person = {1, pb_person, label.repeated},
}

function var_dump(data, max_level, prefix)
    local println = print
    local print = io.write
    if type(prefix) ~= "string" then
        prefix = ""
    end
    if type(data) ~= "table" then
        print(prefix .. tostring(data))
    else
        if max_level ~= 0 then
            local prefix_next = prefix .. "    "
            println(prefix .. "{")

            for k,v in pairs(data) do
                print(prefix_next .. k .. " = ")
                if type(v) ~= "table" or (type(max_level) == "number" and max_level <= 1) then
                    println(v)
                else
                    if max_level == nil then
                        var_dump(v, nil, prefix_next)
                    else
                        var_dump(v, max_level - 1, prefix_next)
                    end
                end
            end
            println(prefix .. "}")
        end
    end
    io.flush()
end

local data = {
    person = {
        {id = 100,
        name = "test",
        son_id = {101,102},
        per = {
        },
        },
    },
}

print "Orignal"
var_dump(data, 5)
local v = pb.pack(data, pb_allperson)
print(v)
local udata = pb.unpack(v, pb_allperson)
print "\n\n"
print "Unpacked"
var_dump(udata, 5)
