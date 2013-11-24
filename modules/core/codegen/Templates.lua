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

local SLICE_UNPACK = "local data,pos = rawget(self,1),rawget(self,2)"
function M.metamethods.__index(T)
    local B = MethodBuilder.create(T, "self", "k")
    B:add(SLICE_UNPACK)
    B:add("print('here with pos =',pos)")
    local prefix = "if"
    for field in T:all_fields() do
        B:add(prefix .. " k == '%s' then return rawget(data, pos+%d)", field.name, field.offset)
        prefix = "elseif"
    end
    B:add('elseif rawget(TYPETABLE, k) then return rawget(TYPETABLE, k)')
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.metamethods.__tostring(T)
    local B = MethodBuilder.create(T, "self")
    B:add("local parts = {}")
    B:add("parts[#parts+1] = 'type %s:'", T.name or "<anon>")
    for field in T:root_fields() do
        B:add("parts[#parts+1] = ' %s = ' .. %s", field.name, field.type:emit_tostring("self." .. field.name))
    end
    B:add("return '[' .. table.concat(parts) .. ']'")
    return B:emit()
end
function M.metamethods.__newindex(T)
    local B = MethodBuilder.create(T, "self", "k", "v")
    B:add(SLICE_UNPACK)
    local prefix = "if"
    for field in T:all_fields() do
        B:add(prefix .. " k == '%s' then ", field.name)
        B:append(field.type:emit_field_assign("data", field.offset, "v"))
        prefix = "elseif"
    end
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.methods.create(T)
    local args, assigns, inits = {}, {}, {} 
    for field in T:root_fields() do
        if field.initializers then append(inits, field)
        else append(assigns, field) ; append(args, "in_" .. field.name) end
    end
    local B = MethodBuilder.create(T, unpack(args))
    B:add("local self")
    B:add(T:emit_preinit("self"))
    for i, f in ipairs(assigns) do
        B:add(f.type:emit_field_assign("data", f.offset, args[i]))
    end
    for i, f in ipairs(inits) do
        B:add(f.type:emit_field_copy("data", f.offset, inits.initializers))
    end
    B:add("print('data=',data[2])")
    B:add("return self")
    return B:emit()
end

function M.emit_global_cache(...) -- For performance, cache common globals
    return "local " .. (","):join {...} .. "=" .. (","):join {...}    
end

local function resolve_namespaces(namespace) 
    local ret = {} ; for namespace in values(namespace) do
        for k, v in pairs(namespace) do 
            if rawget(v, "__structinfo") then ret[k] = v.__structinfo end
        end
    end ; return ret
end

function M.type_parse(definition, --[[Optional]] namespaces, --[[Optional]] name)
    local T = StructType.create(resolve_namespaces(namespaces or {}), name)
    local lines = definition:split("\n")
    for line in values(lines) do T:parse_line(line) end
    return T
end

function M.compile_type(T)
    local parts = {
        M.emit_global_cache("rawget","rawset","setmetatable","getmetatable"),
        "local TYPETABLE = {} ; local METATABLE = {}",
        "TYPETABLE.__metatable = METATABLE"
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
        if getmetatable(field.type) == StructType then
            append(constant_names, field.typename) ; append(constant_vals, field.type.__metatable)
        end
    end
    local type = M.callstring('return function(%s)\n------------\n%s\nend', 
        ('\n'):join(constant_names), ("\n"):join(parts)
    )(unpack(constant_vals))
    T.__metatable = type.__metatable
    type.__structinfo = T
    return type
end

return M