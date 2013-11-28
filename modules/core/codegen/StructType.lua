local structparse = import ".structparse"

local FieldTypes = import ".FieldTypes"

local Field = newtype()

function Field:init(name, type, typename, offset, is_embedded, initializer)
    self.name = name -- Same as type name for embedded member
    self.type, self.typename = type, typename
    self.offset = offset -- The offset to the object representation of this member, eg a slice object
	self.is_embedded = is_embedded
	self.initializer = initializer or false -- If false, passed as argument to create
	self.is_leaf = (self.type.children == 0)
	self.has_alias = true
end

local StructType = newtype { parent = FieldTypes.BaseType }

function StructType:init(--[[Optional]] namespace, --[[Optional]] name)
    self.namespace = namespace or {}
    self.name = name or false 
	self.fields = {}
	self.name_to_field = {}
	self.children = 0
end

StructType.parse_line = structparse.parse_line

function StructType:define_field(name, typename, is_embedded, initializer)
    assert(not self.name_to_field[name], "Name '" .. name .. "' already used in type!")
    local type = self:lookup_type(typename)
    assert(not (is_embedded and not getmetatable(type) == StructType), "Cannot embed primitive type!")
    local field = Field.create(name, type, typename, self.children + 1, is_embedded, initializer)
    self.name_to_field[name] = field ; append(self.fields, field)
    self.children = self.children + type.children + 1
end

function StructType:lookup_type(typename)
    if typename == "*" then return FieldTypes.builtin_types.any end
    local type = FieldTypes.builtin_types[typename]
    if type then return type end
    if not self.namespace[typename] then error("No type with name '" .. typename .. "'." ) end
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
    return filter(self:all_subfields(), "initializer", --[[inverse]] true) 
end

function StructType:_preinit(defs, corrections)
    for f in self:all_subfields() do
        if f.type.children > 0 then 
            append(defs, ("setmetatable({%s}, meta_%s)"):format("false, " .. f.offset, f.typename))
            append(corrections, ("rawget(data, %s)[1] = data"):format(f.offset))
        else append(defs, f.type:emit_default()) end
    end
end

function StructType:emit_preinit(slice)
    local defs, corrections = {}, {}
    self:_preinit(defs, corrections)
    return ("local data,pos = {%s},0\n%s = setmetatable({data,pos}, %s)\n%s"):format(
        (",\n"):join(defs), slice, "METATABLE", ("\n"):join(corrections)
    ):split("\n")
end


function StructType:emit_field_init(S, kroot, args)
    local parts = {"rawset(%s, pos + %d, "}
    local parts = {} ; for field in values(self.fields) do
        local offset = kroot + field.offset
        append(parts, field.type:emit_init(S, offset))
    end ; return parts
end
local AssignBatcher = newtype()
function AssignBatcher:init(kroot)
    self.kroot = kroot
    self.offset = false ; self.batches = 0 ; self.dx = false
end
function AssignBatcher:add(parts, f)
    if not self.offset then 
        self.offset = f.offset ; self.batches = 1 ; return
    end
    if not self.dx then self.dx = f.offset - self.offset
    else
        local dist = f.offset - self.offset - (self.batches-1) * self.dx
        if self.dx ~= dist then
            self:emit(parts) ; self:add(parts, f) ; return
        end
    end

    self.batches = self.batches + 1
end
function AssignBatcher:emit(parts)
    if not self.offset then return end
    local i_start, i_end = self.offset, self.offset+self.batches*self.dx
    if self.batches <= 3 then
        for i=i_start,i_end,self.dx do
            append(parts, ("  rawset(data, pos+%d, rawget(odata, opos+%d))"):format(
                self.kroot + i, i
            ))
        end
    else
        append(parts, ("  for i=%d,%d%s do rawset(data, pos+%d+i, rawget(odata, opos+i)) end"):format(
            i_start,i_end, (self.dx == 1) and '' or ','..self.dx, self.kroot
        ))
    end 
    self:init(self.kroot)
end

function StructType:emit_field_assign(S, kroot, O)
    local parts = {("do local odata, opos = rawget(%s,1),rawget(%s,2) --<%s:emit_field_assign>"):format(O, O, self.name)}
    local batcher = AssignBatcher.create(kroot)
    for f in self:all_leafs() do
        batcher:add(parts, f)
    end
    batcher:emit(parts)
    append(parts, "end")
    return parts
end

return StructType