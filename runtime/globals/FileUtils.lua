function file_as_string(name)
    local f = io.open(name,"r")
    if f == nil then return nil end
    local contents = f:read("*all")
    f:close()
    return contents
end
