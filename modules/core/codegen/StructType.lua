local FieldTypes = import ".FieldTypes"

local Field = newtype()

function Field:init(name, type, is_root, offset, is_embedded, initializers)
    self.name = name -- Same as type name for embedded member
    self.type = type
    self.is_root = is_root -- Is it a root member or an embedded member ?
    self.offset = offset -- The offset to the object representation of this member, eg a slice object
	self.is_embedded = is_embedded
	self.initializers = initializers -- If false, passed as argument to create
end

local StructType = newtype { parent = FieldTypes.BaseType }

function StructType:init(--[[Optional]] namespaces, --[[Optional]] name)
    self.namespaces = namespaces or {}
    self.name = name or false 
	self.fields = {}
	self.children, self.args = 0, 0
end

function StructType:_add_component(name, type, is_embedded)
    self.children, self.args = self.children+1, self.args+1 
    append(self.fields, Field.create(name, type, true, self.children, self.args, is_embedded))
end

function StructType:define_field(name, type, is_embedded)
    self:_add_component(name, type, is_embedded)
    if is_embedded then
        for root in type:root_fields() do
            self:define_field(root.name, root.type, root.is_embedded)
        end
    else
        self.children = self.children + type.children
    end
end

function StructType:lookup_type(typename)
    if typename == "*" then return FieldTypes.builtin_types.any end
    local type = FieldTypes.builtin_types[typename]
    if type then return type end
    for namespace in values(self.namespaces) do
        if namespace[typename] then return namespace[typename] end
    end
    error("No type with name '" .. typename .. "'." )
end

function StructType:parse_field_spec(line)
    local parts = line:split(":")
    local typename = parts[#parts]:trim()
    local is_embedded = (#parts == 1) 
    local type = self:lookup_type(typename)
    assert(not (is_embedded and not getmetatable(type) == StructType), "Cannot embed primitive type!")
    local names = (is_embedded and {typename} or parts[1]:split(","))
    for name in values(names) do
        name = name:trim()
        assert(name:match("^[%w_]+$"), "'" .. name .. "' is not a valid name.")
        self:define_field(name, type, is_embedded)
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

local function member_filter(l, k) 
    local ret1, ret2 = {}, {}
    for v in values(l) do append(v and ret1 or ret2, v) end
end

function StructType:root_fields()
    local roots, _ = member_filter(self.fields, "is_root")
    return values(roots) 
end
function StructType:all_fields() return values(self.fields) end

function StructType:emit_init(S, kroot, args)
    local parts = {}
    for field in values(self.fields) do
        local offset = (kroot + field.offset - 1)
        append(parts, field.type:emit_init(S, offset))
    end
    return parts
end

return StructType