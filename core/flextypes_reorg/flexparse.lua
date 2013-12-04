local M = nilprotect {} -- Submodule

local Struct = import "@Struct"

-- Regex patterns for various parts of the type definition:
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

function M.parse_line(--[[the Struct]] T, line)
    local parts = line:trimsplit(":")
    if #parts == 1 then 
        -- We know this is an embedded type or an extend line
        if parts[1] ~= "" then
--            if parse_extend(T, line) then return end 
            parse_embedded(T, parts[1])
        end
    else
        parse_var(T, parts[1], parts[2])
    end
end

return M