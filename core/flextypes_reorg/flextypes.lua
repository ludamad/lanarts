local M = import_copy("@flexcore", "@flexparse", "@primitives", "@structfieldtype") -- Forward 'private' submodules

local M = nilprotect {} -- Submodule

local IDENTIFIER = "[%w_%-]+"
local IDENTIFIER_ALL = "^"..IDENTIFIER.."$"
local EXTEND = "^%s*extend%s+(" .. IDENTIFIER .. ")$"
-- Return (typename, initializer)
local TYPESPEC = "^("..IDENTIFIER..")$"
local TYPESPEC_WITH_INIT = "^("..IDENTIFIER..")%s*%((.*)%)$"
local NAMED_EMBED = "^([^%s]+)%s+as%s+([^%s]+)$"

local function validate_varname(name)
    assertf(name:match(IDENTIFIER_ALL), "'%s' is not a valid name.", name)
end

local function parse_extend(T, line)
    local typename = line:match(EXTEND)
    if typename then
        local parent = assertf(T:lookup_type(typename), "No type named '%s'.", typename)
        M.extend(T, parent)
        return true
    end
    return false
end
local function parse_typespec(typespec)
    local name, initializer = typespec:match(TYPESPEC_WITH_INIT)
    if not name then
        local typename = typespec:match(TYPESPEC)
        assertf(typename, "Invalid type specification '%s'.", typespec)
        return typename
    end
    return name, initializer
end

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