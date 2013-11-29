local structparse = import ".structparse"
local structgen = import ".structgen"

local FieldTypes = import ".FieldTypes"

local StructType = newtype { parent = FieldTypes.BaseType }

function StructType:init(--[[Optional]] namespace, --[[Optional]] name)
    self.namespace = namespace or {}
    self.name = name or false 
	self.fields = {}
	self.name_to_field = {}
	self.parent_types = {}
	self.typetable = {__structinfo = self}
	self.children = 0
end

StructType.extend = structparse.extend
StructType.parse_line = structparse.parse_line
StructType.define_field = structparse.define_field

function StructType:lookup_type(typename)
    if typename == "*" then return FieldTypes.builtin_types.any end
    local type = FieldTypes.builtin_types[typename]
    if type then return type end
    assertf(self.namespace[typename], "No type with name '%s'.", typename)
    return self.namespace[typename]
end

local function filter(iter, k, --[[Optional]] invert)
    local ret = {} ; for v in iter do
        if not invert and v[k] then append(ret, v) 
        elseif invert and not v[k] then append(ret, v) end
    end ; return values(ret)
end

function StructType:all_fields() return values(self.fields) end
function StructType:embedded_fields() return filter(self:all_fields(), "is_embedded") end
function StructType:_all_subfields(subfields, offset, is_embedded)
    for f in self:all_fields() do
        local subfield = table.clone(f) ; append(subfields, subfield)
        subfield.offset = offset + f.offset ; subfield.has_alias = (is_embedded)
        if not f.is_leaf then
            f.type:_all_subfields(subfields, subfield.offset, f.is_embedded)
        end
    end
end
function StructType:all_subfields()
    local subfields = {} ; self:_all_subfields(subfields, 0, true) ; return values(subfields)
end
function StructType:all_leafs(subfields, offset) return filter(self:all_subfields(), "is_leaf") end
function StructType:all_nonleafs(subfields, offset) return filter(self:all_subfields(), "is_leaf", --[[invert]] true) end
function StructType:all_aliases() return filter(self:all_subfields(), "has_alias") end
function StructType:all_required_fields() 
    return filter(self:all_fields(), "initializer", --[[inverse]] true) 
end

function StructType:_preinit(defs, corrections)
    for f in self:all_subfields() do
        if f.type.children > 0 then 
            append(defs, ("setmetatable({%s}, meta_%s)"):format("false, " .. f.offset, f.typename))
            append(corrections, ("rawget(data, %s)[1] = data"):format(f.offset))
        else append(defs, f.type:emit_default()) end
    end
end

StructType.emit_preinit = structgen.emit_preinit
StructType.emit_field_init = structgen.emit_field_init
StructType.emit_field_assign = structgen.emit_field_assign

return StructType