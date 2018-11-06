function file_as_string(name)
    local f = io.open(name,"r")
    if f == nil then return nil end
    local contents = f:read("*all")
    f:close()
    return contents
end

function file_dump_string(name, str)
    local f = io.open(name,"w")
    if f == nil then return nil end
    f:write(str)
    f:close()
end

function file_dump_json(name, obj)
    file_dump_string(name, require("json").generate(obj))
end

function file_dump_csv(name, obj)
    local keys = table.sorted_key_list(obj[1])
    local csv = {table.concat(keys, ',')}
    for _, o in ipairs(obj) do
        local vals = map_call(function(key) return tostring(o[key]) end, keys)
        local line = table.concat(vals, ',')
        append(csv, line)
    end
    file_dump_string(name, table.concat(csv, '\n'))
end
