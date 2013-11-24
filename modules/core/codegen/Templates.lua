local MethodBuilder = import ".MethodBuilder"
local StructType = import ".StructType"

local M = nilprotect {} -- Submodule

function M.callstring(str) 
    for i, s in ipairs(str:split("\n")) do
        print(i, s)
    end
    local func_loader, err = loadstring(str)
    if err then error(err) end
    return func_loader()
end

M.metamethods = {}
M.methods = {}

function M.metamethods.__index(T)
    local B = MethodBuilder.create(T, "self", "k")
    local prefix = "if"
    for field in T:all_fields() do
        B:add(prefix .. " k == '%s' then return rawget(self, %d)", field.name, field.offset)
        prefix = "elseif"
    end
    B:add('elseif rawget(TYPETABLE, k) then return rawget(TYPETABLE, k)')
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.metamethods.__newindex(T)
    local B = MethodBuilder.create(T, "self", "k", "v")
    local prefix = "if"
    for field in T:all_fields() do
        B:add(prefix .. " k == '%s' then %s", 
            field.name, field.type:emit_field_assign("self", field.offset, "v"))
        prefix = "elseif"
    end
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.methods.create(T)
    local args, assigns, inits = {}, {}, {} 
    for field in values(T.fields) do
        if field.initializers then append(inits, field)
        else append(assigns, field) ; append(args, field.name) end
    end
    local B = MethodBuilder.create(T, unpack(args))
    B:add("local self = setmetatable({}, METATABLE)")
    for i, f in ipairs(assigns) do
        B:add(f.type:emit_field_assign("self", f.offset, args[i]))
    end
    for i, f in ipairs(inits) do
        B:add(f.type:emit_field_copy("self", f.offset, inits.initializers))
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
    local type = M.callstring(callable)
    type.__structinfo = T
    return type
end

return M