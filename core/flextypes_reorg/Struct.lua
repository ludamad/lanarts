local Struct = newtype()

function Struct:init()
	self.object_fields = {}
	self.primitive_fields = {}
	self.hidden_fields = {}
	self._aliases = {}
end

function Struct:alias_add(name, kind, offset)
    assertf("Field '%s' already exists!", name)
	self._aliases[name] = {kind, offset}
end

function Struct:object_field_add(f) append(self.object_fields, f) end
function Struct:primitive_field_add(f) append(self.primitive_fields, f) end
function Struct:hidden_field_add(f) append(self.hidden_fields, f) end

function Struct:aliases()
    local k = next(self._aliases, nil)
    return function()
        if k == nil then return nil end
        local name = k
        local kind, offset = unpack(self._aliases[k])
        k = next(self._aliases, k)
        return name, kind, offset
    end
end

function Struct:validate()
assert(#T.fields > 0, "Cannot define a struct with no fields! " .. (T.name and '('..T.name..')' or ''))
end

return Struct