local append,unpack=append,unpack-- Cache for perf.

local M = nilprotect {} -- Submodule

local Struct = import "@Struct"

--------------------------------------------------------------------------------
-- Main interface: a type-component parsing routine.
-- A type definition is a simple series of these.
--------------------------------------------------------------------------------
local function call_on_match(C, f, ...)
    if ... then f(C, ...) ; return true end ; return false
end
local subroutines -- Defined below.
function M.parse_def(--[[Struct]] T, def, --[[Optional]] linenum)
    for i=1,#subroutines do
        local subroutine,pattern = unpack(subroutines[i])
        if call_on_match(T, subroutine, def:match(pattern)) then
            return -- Finished
        end
    end
    errorf("Unexpected line in typedef for '%s'%s: '%s'",
        T.name, (linenum and " on line "..linenum or ""), def)
end
--------------------------------------------------------------------------------
-- Pattern helpers
--------------------------------------------------------------------------------
local function line_pad(s) return "^%s*"..s.."%s*$" end
local function space_pad(s, --[[Optiona]] opt) return opt and ('%s*' .. s .. '%s*') or ('%s+' .. s .. '%s+') end
local function group(s) return '('..s..')' end
-- Subpatterns for parsing:
local IDENTIFIER = "[%w_%-%*]+" ; local IDENTIFIER_LPAD = line_pad(IDENTIFIER)
local IDENTIFIER_LIST = "[%s,%w_%-%*]+"
local TYPESPEC = group(IDENTIFIER)
local TYPESPEC_WITH_INIT = group(IDENTIFIER)..'%s*%('..group(".*")..'%)'
--------------------------------------------------------------------------------
-- Validation helpers
--------------------------------------------------------------------------------
local function validate_varname(name)
    assertf(name:match(IDENTIFIER_LPAD), "'%s' is not a valid name.", name)
end
--------------------------------------------------------------------------------
-- Parsing subroutines
--------------------------------------------------------------------------------
local function parse_extend(C, typename)
    print("parse_extend", typename)
end
local function parse_embed(C, typename, --[[Optional]] initializer, --[[Optional]] field_name)
    local typespec, name = line:match(NAMED_EMBED)
    if not typespec then typespec = line end 
    if not name then name = typename end
    validate_varname(name)
    M.define_field(C, name, typename, --[[Embedded]] true, --[[May-be-null]] initializer)
end

local function parse_field_def(C, vars, typename, --[[Optional]] initializer)
    for name in values(vars:trimsplit(",")) do
        validate_varname(name)
        C.type:field_add()
        M.define_field(C, name, typename, --[[Not embedded]] false, --[[May-be-null]] initializer)
    end
end
--`1  Define subroutines and the patterns that trigger them:
subroutines = { -- Declared above.
    -- Declarations without a typespec:
    {parse_extend, line_pad("extend%s+"..group(IDENTIFIER))}
}
local function typespec_subroutine(handler, matcher)
    append(subroutines, {handler, matcher:interpolate {TYPESPEC = TYPESPEC_WITH_INIT}})
    append(subroutines, {handler, matcher:interpolate {TYPESPEC = TYPESPEC}})
end
typespec_subroutine(parse_embed, line_pad "${TYPESPEC}")
typespec_subroutine(parse_embed, line_pad("${TYPESPEC}" .. space_pad("as") .. group(IDENTIFIER)))
typespec_subroutine(parse_field_def, line_pad (group(IDENTIFIER_LIST)..space_pad(":",--[[space optional]]true).."${TYPESPEC}"))
--------------------------------------------------------------------------------

return M