local flexcore = import "@flexcore"

local Struct = newtype()

function Struct:init(name)
    self.name = name or '<anon>'
	self.fields = {}
	self.subfields = {}
	self.metadata = {}
	self._aliases = {}
end

function Struct:alias_add(name, kind, offset)
    assertf("Field '%s' already exists!", name)
	self._aliases[name] = {kind, offset}
end

function Struct:field_add(f) append(self.fields, f) end
function Struct:subfield_add(f) append(self.subfields, f) end
function Struct:metadata_add(f) append(self.metadata, f) end

function Struct:emit_unpack()
    local SLICE_CHECK = "rawget(self,0) ~= nil" -- For slices, we store the actual location of 'data' in  
    local SLICE_UNPACK = "data,pos=rawget(self,0),rawget(self,1)"
    return fmt("local data, pos = self, 0 ; if %s then %s end", SLICE_CHECK, SLICE_UNPACK)
end

function Struct:emit_direct_get(name)
    return fmt("rawget(data, pos+%d)", self:offset_of(name))
end

function Struct:emit_direct_set(name, val_s)
    return fmt("rawset(data, pos+%d, %s)", self:offset_of(name), val_s)
end

local F = flexcore
function Struct:offset_of(name)
    local alias = self._aliases[name]
    local kind,offset = alias[1],alias[2]
    local olen, plen = #self.fields, #self.subfields
    if     kind == F.FIELD_ALIAS    then return offset
    elseif kind == F.SUBFIELD_ALIAS then return offset + olen
    elseif kind == F.METADATA_ALIAS then return offset + olen + plen end
end

function Struct:aliases()
    local k = next(self._aliases, nil)
    return function()
        if k == nil then return nil end
        local name, kind = k, (self._aliases[k][1])
        k = next(self._aliases, k)
        return name, kind, self:offset_of(name)
    end
end

function Struct:validate()
    assertf(#self._aliases > 0, "Cannot define a struct with no fields! (for type '%s')", self.name)
end

return Struct