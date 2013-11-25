local M = nilprotect {} -- Submodule

-- Custom types inherit from this type.
-- Defines things that work for most types.
local BaseType = newtype() ; M.BaseType = BaseType
function BaseType:emit_field_assign(S, kroot, O, --[[Optional]] ellide_typecheck)
    local typecheck = self:emit_typecheck(O)
    ellide_typecheck = ellide_typecheck or (typecheck == nil)
    local assert = ellide_typecheck and "" or ("assert(%s, 'Incorrect type (failed %s).') ; "):format(typecheck, typecheck:gsub("'",'"'))
    return ("%srawset(%s, %s, %s)"):format(
        assert, S, "pos+" .. kroot, O
    )
end
function BaseType:emit_field_init(S, kroot, args)
    assert(#args == 1, "Primitive initializers need exactly one expression!")
    return self:emit_field_assign(S, kroot, args[1])
end
function BaseType:emit_tostring(S) return ("tostring(%s)"):format(S) end
function BaseType:emit_typecheck(S) return nil end

M.builtin_types = {}

local PrimitiveType = newtype {parent = BaseType}
function PrimitiveType:init(typecheck_template, default)
    self.children = 0
    self.typecheck_template = typecheck_template
    self.default = default
end
function PrimitiveType:emit_default() 
    return self.default
end
function PrimitiveType:emit_typecheck(S)
    return self.typecheck_template:interpolate{ self = S }
end
local function istype(t) return "type(${self}) == '" .. t .. "'" end
for k, v in pairs {
    float = {istype("number"), "0"}, 
    double = {istype("number"), "0"}, 
    string = {istype("string"), "''"}, 
    bool = {istype("boolean"), "false"}, 
    int = {istype("number") .. " and (${self}%1==0)", "0"}, 
    list = {istype("table") .. " and not getmetatable(${self})", "{}"},
    map = {istype("table") .. " and not getmetatable(${self}) and #(${self}) == 0", "{}"},
    object = {istype("table") .. " and getmetatable(${self})", "nil"},
    ["function"] = {istype("function"), "do_nothing"},
    any = {nil, "nil"}
} do
    M.builtin_types[k] = PrimitiveType.create(unpack(v))
end

return M