local FieldTypes = import ".FieldTypes"

local Field = newtype()

function Field:init(name, type, is_root, offset, is_embedded, initializers)
    self.name = name -- Same as type name for embedded member
    self.type = type
    self.is_root = is_root -- Is it a root member or an embedded member ?
    self.offset = offset -- The offset to the object representation of this member, eg a slice object
	self.is_embedded = is_embedded
	self.initializers = initializers or false -- If false, passed as argument to create
end

local StructType = newtype { parent = FieldTypes.BaseType }

function StructType:init(--[[Optional]] namespaces, --[[Optional]] name)
    self.namespaces = namespaces or {}
    self.name = name or false 
	self.fields = {}
	self.children = 0
end

function StructType:_add_component(name, type, is_root, is_embedded, initializers)
    self.children = self.children+1
    append(self.fields, Field.create(name, type, is_root, self.children, is_embedded, initializers))
end

function StructType:define_field(name, type, is_root, is_embedded)
    self:_add_component(name, type, is_root, is_embedded)
    if is_embedded then
        for root in type:root_fields() do
            self:define_field(root.name, root.type, false, root.is_embedded)
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
        if namespace[typename] then return namespace[typename].__structinfo end
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
        self:define_field(name, type, true, is_embedded)
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

function StructType:root_fields()
    local ret = {} ; for f in self:all_fields() do 
        if f.is_root then append(ret, f) end 
    end ; return values(ret) 
end
function StructType:all_fields() return values(self.fields) end

function StructType:emit_field_init(S, kroot, args)
    local parts = {} ; for field in values(self.fields) do
        local offset = (kroot + field.offset - 1)
        append(parts, field.type:emit_init(S, offset))
    end ; return parts
end
function StructType:emit_field_assign(S, kroot, O)
    local parts = {} ; for field in values(self.fields) do
        local offset = (kroot + field.offset - 1)
        append(parts, field.type:emit_init(S, offset))
    end ; return parts
end

return StructType