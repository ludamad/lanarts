local FieldTypes = import ".FieldTypes"

local Field = newtype()

function Field:init(name, type, typename, offset, is_embedded, initializers)
    self.name = name -- Same as type name for embedded member
    self.type, self.typename = type, typename
    self.offset = offset -- The offset to the object representation of this member, eg a slice object
	self.is_embedded = is_embedded
	self.initializers = initializers or false -- If false, passed as argument to create
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

function StructType:define_field(name, typename, is_embedded)
    assert(not self.name_to_field[name], "Name '" .. name .. "' already used in type!")
    local type = self:lookup_type(typename)
    assert(not (is_embedded and not getmetatable(type) == StructType), "Cannot embed primitive type!")
    local field = Field.create(name, type, typename, self.children + 1, is_embedded)
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

function StructType:parse_field_spec(line)
    local parts = line:split(":")
    local typename = parts[#parts]:trim()
    local is_embedded = (#parts == 1) 
    local names = (is_embedded and {typename} or parts[1]:split(","))
    for name in values(names) do
        name = name:trim()
        assert(name:match("^[%w_]+$"), "'" .. name .. "' is not a valid name.")
        self:define_field(name, typename, true, is_embedded)
    end
end

function StructType:parse_line(line)
    if line:match("^%s*[%w_]+%s*$") then
        self:parse_field_spec(line) -- Embedded field
    elseif line:match("^%s*[%w_,%s]+:%s*[%w_]+%s*$") then
        self:parse_field_spec(line) -- Normal field
    elseif not line:match("^%s*$") then
        error("Unexpected line '" .. line .. "'.")
    end
end

local function filter(iter, k)
    local ret = {} ; for v in iter do
        if v[k] then append(ret, v) end 
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
function StructType:all_aliases() return filter(self:all_subfields(), "has_alias") end

function StructType:_preinit(defs, corrections)
    for f in self:all_subfields() do
        if f.type.children > 0 then 
            append(defs, ("setmetatable({%s}, %s)"):format("false, " .. f.offset, f.typename))
            append(corrections, ("rawget(data, %s)[1] = data"):format(f.offset))
        else append(defs, f.type:emit_default()) end
    end
end

function StructType:emit_preinit(slice)
    local defs, corrections = {}, {}
    self:_preinit(defs, corrections)
    return ("local data,pos = {%s},0\n%s = setmetatable({data,pos}, %s)\n%s"):format(
        (","):join(defs), slice, "METATABLE", ("\n"):join(corrections)
    ):split("\n")
end

function StructType:emit_field_init(S, kroot, args)
    local parts = {"rawset(%s, pos + %d, "}
    local parts = {} ; for field in values(self.fields) do
        local offset = kroot + field.offset
        append(parts, field.type:emit_init(S, offset))
    end ; return parts
end
function StructType:emit_field_assign(S, kroot, O)
    local parts = {("do local odata, opos = rawget(%s,1),rawget(%s,2) --<%s:emit_field_assign>"):format(O, O, self.name)}
    for f in self:all_leafs() do
        local assign = f.type:emit_field_assign(
            "data",  kroot + f.offset, 
            ("rawget(odata, opos+%s)"):format(f.offset), 
            --[[ellide typecheck]] true
        )
        append(parts, '    ' .. assign)
    end
    append(parts, "end")
    return parts
end

return StructType