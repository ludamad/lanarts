local FieldTypes = import ".FieldTypes"

local Field = newtype()

function Field:init(name, type, typename, is_root, offset, is_embedded, initializers)
    self.name = name -- Same as type name for embedded member
    self.type = type
    self.typename = typename
    self.is_root = is_root -- Is it a root member or an embedded member ?
    self.offset = offset -- The offset to the object representation of this member, eg a slice object
	self.is_embedded = is_embedded
	self.initializers = initializers or false -- If false, passed as argument to create
end

local StructType = newtype { parent = FieldTypes.BaseType }

function StructType:init(--[[Optional]] namespace, --[[Optional]] name)
    self.namespace = namespace or {}
    self.name = name or false 
	self.fields = {}
	self.children = 0
end

function StructType:_add_component(name, typename, is_root, is_embedded, initializers)
    local type = self:lookup_type(typename)
    self.children = self.children+1
    append(self.fields, Field.create(name, type, typename, is_root, self.children, is_embedded, initializers))
end

function StructType:define_field(name, typename, is_root, is_embedded)
    if is_root then
        for root in self:root_fields() do
            assert(root.name ~= name, "Name '" .. name .. "' already used in struct!")
        end
    end
    self:_add_component(name, typename, is_root, is_embedded)
    local type = self:lookup_type(typename)
    assert(not (is_embedded and not getmetatable(type) == StructType), "Cannot embed primitive type!")
    if is_embedded then
        for root in type:root_fields() do
            self:define_field(root.name, root.typename, false, root.is_embedded)
        end
    else
        self.children = self.children + type.children
    end
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

function StructType:root_fields()
    local ret = {} ; for f in self:all_fields() do 
        if f.is_root then append(ret, f) end 
    end ; return values(ret) 
end
function StructType:all_fields() return values(self.fields) end
function StructType:all_subfields(subfields)
    subfields = subfields or {} ; for f in self:root_fields() do
        append(subfields, f)
        if f.type.children > 0 then
            f.type:all_subfields(subfields)
        end
    end ; return values(subfields)
end

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
    for f in self:all_fields() do
        if f.type.children == 0 then
            local assign = f.type:emit_field_assign(
                "data",  kroot + f.offset, 
                ("rawget(odata, opos+%s)"):format(f.offset), 
                --[[ellide typecheck]] true
            )
            append(parts, '    ' .. assign)
        end
    end
    append(parts, "end")
    return parts
end

return StructType