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
