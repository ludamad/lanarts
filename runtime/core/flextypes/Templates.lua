local AnsiColors = import "terminal.AnsiColors"

local MethodBuilder = import ".MethodBuilder"
local StructType = import ".StructType"

local M = nilprotect {} -- Submodule

local AC = AnsiColors
local function hilight(s)
    for k in values {"local", "elseif", "else", "return", "function", "end", "rawget", "rawset"} do
        for pattern in values {"$", "[%s%(]"} do
            s = s:gsub(k..pattern, function(p1) 
                return p1:gsub(k, function(p2) return AC.WHITE(p2) end) 
            end)
        end
    end ; return s
end

function M.callstring(str)
--    for i, s in ipairs(str:split("\n")) do
--        local lineno = ("%3d) "):format(i)
--        print( AC.WHITE(lineno) .. hilight(s))
--    end
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
    local prefix = "if"
    for field in T:all_aliases() do
        B:add(prefix .. " k == '%s' then return rawget(data, pos+%d)", field.name, field.offset)
        prefix = "elseif"
    end
    B:add('elseif rawget(TYPETABLE, k) then return rawget(TYPETABLE, k)')
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.metamethods.__tostring(T)
    local B = MethodBuilder.create(T, "self", "indent")
    B:add("local parts = {':%s'}", T.name or "<anon>")
    B:add("local function add(s) for i=1,indent do s='  '..s end ; append(parts, s) end")
    B:add("indent = indent or 0")
    local leafs,roots = {},{}
    for field in T:all_fields() do
        if field.is_leaf then 
            local piece = ("'%s ' .. %s"):format(field.name, field.type:emit_tostring("self." .. field.name))
            append(leafs, piece)
        else
            local piece = ("add('%s ' .. self.%s:__tostring(indent+1))"):format(field.name, field.name)
            append(roots, piece)
        end
    end
    if #leafs > 0 then B:add("add(%s)", (".. ', '.."):join(leafs)) end
    B:add(roots)
    B:add("return '['..(' '):join(parts)..']'")
    return B:emit()
end
function M.metamethods.__newindex(T)
    local B = MethodBuilder.create(T, "self", "k", "v")
    B:add(SLICE_UNPACK)
    local prefix = "if"
    for field in T:all_aliases() do
        B:add(prefix .. " k == '%s' then ", field.name)
        B:append(field.type:emit_field_assign("data", field.offset, "v"))
        prefix = "elseif"
    end
    B:add('else error(("No such key \'%s\'"):format(k)) end')
    return B:emit()
end

function M.methods.create(T)
    local args, assigns, inits = {}, {}, {} 
    for field in T:all_fields() do
        if field.initializer then append(inits, field)
        else append(assigns, field) ; append(args, "in_" .. field.name:gsub("-", "_d_")) end
    end
    local B = MethodBuilder.create(T, unpack(args))
    B:add("local self")
    B:add(T:emit_preinit("self"))
    for i, f in ipairs(assigns) do
        B:add(f.type:emit_field_assign("data", f.offset, args[i]))
    end
--    for i, f in ipairs(inits) do
--        B:add(f.type:emit_field_assign("data", f.offset, inits.initializer))
--    end
    B:add("return self")
    return B:emit()
end

function M.emit_global_cache(...) -- For performance, cache common globals
    return "local " .. (","):join {...} .. "=" .. (","):join {...}    
end

local function resolve_namespaces(namespaces) 
    local ret = {} ; for namespace in values(namespaces) do
        for k, v in pairs(namespace) do 
            if type(v) == "table" and rawget(v, "__structinfo") then 
                ret[k] = v.__structinfo 
            end
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
    assert(#T.fields > 0, "Cannot define a struct with no fields! " .. (T.name and '('..T.name..')' or ''))
    local parts = {
        M.emit_global_cache("rawget","rawset","setmetatable","getmetatable"),
        "local TYPETABLE = in_TYPETABLE; local METATABLE = {}",
        "TYPETABLE.__metatable = METATABLE"
    }
    for k, v in pairs(M.metamethods) do
        local method = v(T)
        append(parts, "METATABLE." .. k .. " = " .. method)
        append(parts, "TYPETABLE." .. k .. " = METATABLE." .. k)
    end
    for k, v in pairs(M.methods) do
        local method = v(T)
        append(parts, "TYPETABLE." .. k .. " = " .. method)
    end
    append(parts, "return TYPETABLE")
    local callable = (","):join(parts)

    local arg_names, arg_vals = {"in_TYPETABLE"}, {T.typetable}
    for field in T:all_nonleafs() do
        local var_name = 'meta_'..field.typename
        if not table.contains(arg_names, var_name) then
            append(arg_names, var_name)
            append(arg_vals, field.type.__metatable)
        end
    end
    local type = M.callstring(
        ('return function(%s)\n------------\n%s\nend'):format( 
            (','):join(arg_names), ("\n"):join(parts)
        )
    )(unpack(arg_vals))
    T.__metatable = type.__metatable
    return type
end

return M