local flexcore = import "@flexcore"

local MethodBuilder = import ".MethodBuilder"
local StructType = import ".StructType"

local M = nilprotect {} -- Submodule

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