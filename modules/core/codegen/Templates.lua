local MethodBuilder = import ".MethodBuilder"
local StructType = import ".StructType"

local M = nilprotect {} -- Submodule

function M.callstring(str, ...)
    if ... then str = str:format(...) end 
    for i, s in ipairs(str:split("\n")) do
        print(i, s)
    end
    local func_loader, err = loadstring(str)
    if err then error(err) end
    return func_loader()
end

M.metamethods = {}
M.methods = {}

local SLICE_UNPACK = "local data,pos = self[1],self[2]"
function M.metamethods.__index(T)
    local B = MethodBuilder.create(T, "self", "k")
    B:add(SLICE_UNPACK)
    local prefix = "if"
    for field in T:all_fields() do
        B:add(prefix .. " k == '%s' then return rawget(data, pos+%d)", field.name, field.offset)
        prefix = "elseif"
    end
    B:add('elseif rawget(TYPETABLE, k) then return rawget(TYPETABLE, k)')
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.metamethods.__newindex(T)
    local B = MethodBuilder.create(T, "self", "k", "v")
    B:add(SLICE_UNPACK)
    local prefix = "if"
    for field in T:all_fields() do
        B:add(prefix .. " k == '%s' then %s", 
            field.name, field.type:emit_field_assign("data", field.offset, "v"))
        prefix = "elseif"
    end
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.methods.create(T)
    local args, assigns, inits = {}, {}, {} 
    for field in T:root_fields() do
        if field.initializers then append(inits, field)
        else append(assigns, field) ; append(args, field.name) end
    end
    local B = MethodBuilder.create(T, unpack(args))
    B:add("local self = setmetatable({ --[[Data]] {}, --[[Offset]] 0}, METATABLE)")
    B:add(SLICE_UNPACK)
    for i, f in ipairs(assigns) do
        B:add(f.type:emit_field_assign("data", f.offset, args[i]))
    end
    for i, f in ipairs(inits) do
        B:add(f.type:emit_field_copy("data", f.offset, inits.initializers))
    end
    B:add("return self")
    return B:emit()
end

function M.emit_global_cache(...) -- For performance, cache common globals
    return "local " .. (","):join {...} .. "=" .. (","):join {...}    
end

function M.type_parse(definition, --[[Optional]] namespaces, --[[Optional]] name)
    local T = StructType.create(namespaces, name)
    local lines = definition:split("\n")
    for line in values(lines) do T:parse_line(line) end
    return T
end

local function find_type_name(type, namespaces) 
    for namespace in values(namespaces) do
        for k, v in pairs(namespace) do if type == v then return k end
    end
end

function M.compile_type(T)
    local parts = {
        M.emit_global_cache("rawget","rawset","setmetatable","getmetatable"),
        "local TYPETABLE = {} ; local METATABLE = {}"
    }
    for k, v in pairs(M.metamethods) do
        local method = v(T)
        append(parts, "METATABLE." .. k .. " = " .. method)
    end
    for k, v in pairs(M.methods) do
        local method = v(T)
        append(parts, "TYPETABLE." .. k .. " = " .. method)
    end
    append(parts, "return TYPETABLE")
    local callable = ("\n"):join(parts)

    local constant_names, constant_vals = {}, {}
    for field in T:all_fields() do
        constant_nam
    end
    local type = M.callstring('return function(%s)\n%s\nend', 
        ('\n'):join(table.key_list(constants)), ("\n"):join(parts)
    )
    type.__structinfo = T
    return type
end

return M