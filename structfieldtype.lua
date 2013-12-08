local flexcore = import "@flexcore"

local M = nilprotect {} -- Submodule

function M.struct_field_type_create(struct)
    local type = flexcore.field_type_create()
    local function meta_resolve(self, meta_f)
        return meta_f(self.typename, struct)
    end
    function type:init(typename, --[[Optional]] initializer, is_embedded)
        type.parent.init(self, typename, initializer)
        self.is_embedded = is_embedded
    end
    function type:on_create(data, position, meta_f)
        -- Return a placeholder, and then set the field properly:
        local cleanup = fmt(
            "rawset(%s, %s, setmetatable({%s, %s}, %s))", 
            data, position, data, position, meta_resolve(self, meta_f)
        ) 
        return "0", cleanup
    end
    function type:on_derive_fields(T, offset)
        for f in struct:primitive_fields() do
            T:primitive_field_add(f)
        end
        if self.is_embedded then
            for name,kind,f_offset in struct:aliases() do
                T:alias_add(name,kind,f_offset)
            end
        end
    end
    function type:emit_valuecheck(val_s, meta_f)
        return fmt("getmetatable(%s)==%s", val_s, meta_resolve(self, meta_f))
    end
    function type:emit_typecheck(S)
        return self.typecheck_template:interpolate{ val = S }
    end
    return type
end

return M