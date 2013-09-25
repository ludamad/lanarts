--- Schemas for checking objects
local M = nilprotect {} -- Submodule

local ALLOW_NIL_METATABLE = {}
local DEFAULTED_METATABLE = {}
local ONE_OF_METATABLE = {}
function M.allow_nil(constraint)
    return setmetatable({constraint}, ALLOW_NIL_METATABLE)
end

function M.defaulted(constraint, val)
    return setmetatable({constraint, val}, DEFAULTED_METATABLE)
end

function M.one_of(choices, --[[Optional]] default_values)
    return setmetatable({choices, default_values}, ONE_OF_METATABLE)
end

-- Constraints (assigning an empty table gives them a unique value)
M.NOT_NIL = {}
M.NIL = {}

M.ANY = {}
M.TABLE = {}
M.NUMBER = {}
M.BOOL = {}
M.STRING = {}
M.FUNCTION = {}

M.TABLE_OR_NIL = M.allow_nil(M.TABLE)
M.BOOL_OR_NIL = M.allow_nil(M.BOOL)
M.NUMBER_OR_NIL = M.allow_nil(M.NUMBER)
M.STRING_OR_NIL = M.allow_nil(M.STRING)
M.FUNCTION_OR_NIL = M.allow_nil(M.FUNCTION)

function M.type_constraint(t, allow_nil) 
    local constraint = {__metatable = t}
    return allow_nil and M.allow_nil(constraint) or constraint
end

local function is_special_member(k)
    return k == "__metatable"
end

local function enforce_value(k, v, t)
    if type(v) == "function" then
        v(t, k)
        return
    end
	local meta = getmetatable(v)
	-- Handle 'one of X' constraints
	if meta == ONE_OF_METATABLE then
	   if v[2] and t[k] == nil then
	       t[k] = v[2]
       else
	       assert(table.contains(v[1], t[k]), "Failed value constraint")
	   end
	   return
	end
	-- Handle nil-allowing and defaulted constraints
    if meta == ALLOW_NIL_METATABLE or meta == DEFAULTED_METATABLE then
        if t[k] == nil then -- Allow if nil
        	if meta == DEFAULTED_METATABLE then
        		t[k] = v[2]
        	end
        	v = M.ANY
        else
        	v = v[1] -- Unbox constraint
        end
    end

    if v == M.NOT_NIL then
        assert(t[k] ~= nil, "Failed non-nil constraint")
    elseif v == M.NIL then
        assert(t[k] == nil, "Failed nil constraint")
    elseif v == M.TABLE then
        assert(type(t[k]) == "table", "Failed table constraint "..k)
    elseif v == M.BOOL then
        assert(type(t[k]) == "boolean", "Failed boolean constraint")
    elseif v == M.NUMBER then
        assert(type(t[k]) == "number", "Failed number constraint")
    elseif v == M.STRING then
        assert(type(t[k]) == "string", "Failed string constraint")
    elseif v == M.FUNCTION then
        assert(type(t[k]) == "function", "Failed function constraint")
    elseif v ~= M.ANY then
        -- Recursive schema check
        assert(type(t[k]) == "table", "Failed table constraint for nested schema")
        M.enforce(v, t[k])
    end
end

function M.enforce(schema, t)
    if schema.__metatable then
        assert(schema.__metatable == getmetatable(t), "Failed metatable constraint")
    end
    for k,v in pairs(schema) do
        if not is_special_member(k) then 
            enforce_value(k, v, t)
        end
    end
    return t
end

function M.enforce_function_create(schema) 
    return function(t) 
        return M.enforce(schema, t) 
    end 
end

return M