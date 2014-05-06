local flexcore = import "@flexcore"

local M = nilprotect {} -- Submodule

M.primitive_types = {}

function M.primitive_field_type_create(typecheck_template, default)
    local type = flexcore.field_type_create()
    function type:on_create(data, position)
        -- We can also return cleanup code; however its not needed for primitives. 
        return default
    end
    function type:emit_valuecheck(val_s)
        return typecheck_template:interpolate{ val = val_s }
    end
    return type
end

local function istype(t) return "type(${val}) == '" .. t .. "'" end
for k, v in pairs {
    float = {istype("number"), "0"}, 
    double = {istype("number"), "0"}, 
    string = {istype("string"), "''"}, 
    bool = {istype("boolean"), "false"}, 
    int = {istype("number") .. " and (${val}%1==0)", "0"}, 
    list = {istype("table") .. " and not getmetatable(${val})", "{}"},
    map = {istype("table") .. " and not getmetatable(${val}) and #(${val}) == 0", "{}"},
    object = {istype("table") .. " and getmetatable(${val})", "nil"},
    ["function"] = {istype("function"), "do_nothing"},
    any = {nil, "nil"}
} do
    M.primitive_types[k] = M.primitive_field_type_create(unpack(v))
end

return M