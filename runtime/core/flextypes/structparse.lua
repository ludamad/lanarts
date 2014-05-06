-- Defines parse_line 'method'

local M = nilprotect {} -- Submodule

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

do -- Restrict scope of lazy import
    local StructType -- Lazy imported
    function M.define_field(T, name, typename, is_embedded, initializer)
        StructType = StructType or import ".StructType" -- Lazy import

        assert(not T.name_to_field[name], "Name '" .. name .. "' already used in type!")
        local type = T:lookup_type(typename)
        assert(not (is_embedded and not getmetatable(type) == StructType), "Cannot embed primitive type!")
        local field = Field.create(name, type, typename, T.children + 1, is_embedded, initializer)
        T.name_to_field[name] = field ; append(T.fields, field)
        T.children = T.children + type.children + 1
    end
end


local IDENTIFIER = "[%w_%-]+"
local IDENTIFIER_ALL = "^"..IDENTIFIER.."$"

local function validate_varname(name)
    assert(name:match(IDENTIFIER_ALL), "'" .. name .. "' is not a valid name.")
end

local EXTEND = "^%s*extend%s+(" .. IDENTIFIER .. ")$"
local function parse_extend(T, line)
    local typename = line:match(EXTEND)
    if typename then
        local parent = assertf(T:lookup_type(typename), "No type named '%s'.", typename)
        M.extend(T, parent)
        return true
    end
    return false
end
-- Return (typename, initializer)
local TYPESPEC = "^("..IDENTIFIER..")$"
local TYPESPEC_WITH_INIT = "^("..IDENTIFIER..")%s*%((.*)%)$"
local function parse_typespec(typespec)
    local name, initializer = typespec:match(TYPESPEC_WITH_INIT)
    if not name then
        local typename = typespec:match(TYPESPEC)
        assertf(typename, "Invalid type specification '%s'.", typespec)
        return typename
    end
    return name, initializer
end

local NAMED_EMBED = "^([^%s]+)%s+as%s+([^%s]+)$"
local function parse_embedded(T, line)
    local typespec, name = line:match(NAMED_EMBED)
    if not typespec then typespec = line end 
    local typename, initializer = parse_typespec(typespec)
    if not name then name = typename end
    validate_varname(name)
    M.define_field(T, name, typename, --[[Embedded]] true, --[[May-be-null]] initializer)
end

local function parse_var(T, vars, typespec)
    local typename, initializer = parse_typespec(typespec)
    for name in values(vars:trimsplit(",")) do
        validate_varname(name)
        M.define_field(T, name, typename, --[[Not embedded]] false, --[[May-be-null]] initializer)
    end
end

function M.parse_line(--[[The StructType]] T, line)
    local parts = line:trimsplit(":")
    if #parts == 1 then 
        -- We know this is an embedded type or an extend line
        if parts[1] ~= "" then
            if parse_extend(T, line) then return end 
            parse_embedded(T, parts[1])
        end
    else
        parse_var(T, parts[1], parts[2])
    end
end

-- Check that aliases do not conflict before adding field
local function check_aliases(T, newf)
    local function err(f, field)
        local from_embedded = table.contains(T.fields, f)
        errorf("%s would shadow %s field '%s:%s'!",
            field, (from_embedded and "the derived" or "an existing"), f.name, f.typename
        )
    end
    for f in T:all_aliases() do
        if f.name == newf.name then err(f, fmt("Field '%s :%s'", newf.name, newf.typename)) end
        for derived in f.type:all_aliases() do
            if derived.name == f.name then
                local from_embedded = table.contains(T.fields, f)
                err(f, fmt("Derived field '%s.%s :%s'", newf.name, derived.name, derived.typename))
            end
        end
    end
end

function M.extend(T, parent)
    append(T.parent_types, parent)
    table.merge(T.typetable, parent.typetable)
    T.typetable.__structinfo = T
    for f in parent:all_fields() do
        M.define_field(T, f.name, f.typename, f.is_embedded, f.initializer)
    end
end

return M