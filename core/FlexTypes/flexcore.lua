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

return M