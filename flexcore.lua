local M = nilprotect {} -- Submodule

-- Field kinds
M.FIELD_ALIAS,M.SUBFIELD_ALIAS,M.METADATA_ALIAS = 1,2,3

local FieldBase = newtype()
function FieldBase:init(typename, --[[Optional]] initializer)
	self.typename = typename
	self.initializer = initializer or false
end

function FieldBase:emit_assign(get_s, set_f, val_s, --[[Optional]] dont_check_value)
    local valuecheck = self:emit_valuecheck(val_s)
    if dont_check_value or (valuecheck == nil) then
        return set_f(val_s)
    else
        return fmt("assert(%s, 'Incorrect type (failed %s).') ; %s", 
            valuecheck, valuecheck:gsub("'",'"'), set_f(val_s))
    end
end

function FieldBase:emit_default() 
    return nil
end

function FieldBase:emit_valuecheck(val_s, meta_s)
    return nil
end

function M.field_type_create()
	return newtype {parent = FieldBase}
end

local CodeGenContext = newtype()

function CodeGenContext:init(--[[struct]] T) self.type = T ; self.upvalues = {} end
function CodeGenContext:upvalue_add(k, v)
    if self.upvalues[k] ~= v then
        assertf(self.upvalues[k] == nil, "Conflicting value for injected upvalue '%s'!", k)
        self.upvalues[k] = v
    end
end
function CodeGenContext:inject(func_s)
    local fields = table.key_list(self.upvalues)
    return ("function(%s)\n%s\nend"):format((","):join(fields), func_s)
end
function CodeGenContext:inject_and_load(func_s)
    local upvals = table.value_list(self.upvalues)
    return M.callstring(self:inject(func_s)) (unpack(upvals))
end

M.codegen_context_create = CodeGenContext.create

return M