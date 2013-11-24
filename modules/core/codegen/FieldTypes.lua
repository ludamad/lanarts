local M = nilprotect {} -- Submodule

-- Custom types inherit from this type.
-- Defines things that work for most types.
local BaseType = newtype() ; M.BaseType = BaseType
function BaseType:emit_field_assign(S, kroot, O)
    local typecheck = self:emit_typecheck(O) 
    local assert = typecheck and ("assert(%s, 'Incorrect type.') ; "):format(typecheck) or ""
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
function PrimitiveType:init(typecheck_template)
    self.children = 0
    self.typecheck_template = typecheck_template
end
function PrimitiveType:emit_typecheck(S)
    return self.typecheck_template:interpolate{ self = S }
end
local function istype(t) return "type(${self}) == '" .. t .. "'" end
for k, v in pairs {
    float = istype("number"), 
    double = istype("number"), 
    int = istype("number") .. " and (${self}%1==0)", 
    list = istype("table") .. " and not getmetatable(${self})",
    map = istype("table") .. " and not getmetatable(${self}) and #(${self}) == 0",
    object = istype("table") .. " and getmetatable(${self})",
    any = false
} do
    M.builtin_types[k] = PrimitiveType.create(v or nil)
end

return M