--- Schemas for checking objects
local M = nilprotect {} -- Submodule

local ALLOW_NIL_METATABLE = {}
function M.allow_nil(constraint)
    return setmetatable({constraint}, ALLOW_NIL_METATABLE)
end

-- Constraints (assigning an empty table gives them a unique value)
M.NOT_NIL = {}
M.NIL = {}

M.TABLE = {}
M.NUMBER = {}
M.STRING = {}
M.FUNCTION = {}

M.TABLE_OR_NIL = M.allow_nil(M.TABLE)
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

local function check_value(k, v, t)
    if getmetatable(v) == ALLOW_NIL_METATABLE then
        if t[k] == nil then -- Allow if nil
            v = M.NIL
        else
            v = v[1] -- Else unbox
        end        
    end

    if v == M.NOT_NIL then
        assert(t[k] ~= nil, "Failed non-nil constraint")
    elseif v == M.NIL then
        assert(t[k] == nil, "Failed nil constraint")
    elseif v == M.TABLE then
        assert(type(t[k]) == "table", "Failed table constraint")
    elseif v == M.NUMBER then
        assert(type(t[k]) == "number", "Failed number constraint")
    elseif v == M.STRING then
        assert(type(t[k]) == "string", "Failed string constraint")
    elseif v == M.FUNCTION then
        assert(type(t[k]) == "function", "Failed function constraint")
    else
        -- Recursive schema check
        assert(type(t[k]) == "table", "Failed table constraint for nested schema")
        M.check(v, t[k])
    end
end

function M.check(schema, t)
    if schema.__metatable then
        assert(schema.__metatable == getmetatable(t))
    end
    for k,v in pairs(schema) do
        if not is_special_member(k) then 
            check_value(k, v, t)
        end
    end
    return t
end

function M.checker_create(schema) 
    return function(t) 
        return M.check(schema, t) 
    end 
end

return M