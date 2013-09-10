local M = nilprotect {} -- Submodule

local cache = {}

function M.to_mult_table(...)
    local ret = {}
    for i=1,select("#", ...),2 do
        local trait = select(i, ...)
        local value = select(i+1, ...)
        ret[trait] = value
    end
    return ret
end

-- Utility for resolving arguments to multiplier tables.
-- Resolves a string value 's' as {[s] = 1}
-- and a list of strings as all one-weight.  
function M.resolve_multiplier(arg)
    if type(arg) == "string" then return {[arg] = 1} end
    if #arg > 0 then
        local ret = {}
        for trait in values(arg) do
            ret[trait] = 1
        end
        return ret
    end

    return arg
end


-- Resolve a list of 4 multipliers.
-- If given a single multiplier, duplicates it 4 times.
function M.resolve_multiplier_set(arg)
    -- This is already a multiplier-table
    if type(arg) == "table" and type(arg[1]) == "table" then
        assert(#arg == 4, "'resolve_multiplier_table' expects a set of 4 attack multipliers")
        local copy = {}
        for val in values(arg) do
            table.insert(copy, M.resolve_multiplier(val))
        end
        return copy
    end
    -- Else
    return {dup(M.resolve_multiplier(arg), 4)}
end

return M