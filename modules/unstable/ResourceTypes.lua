-- Convenience functions for dealing with resources.
-- A resource must have a field named 'id'.
-- Name and description are other typical fields for which convenience functions are provided.
-- Resources are defined using a table of values, and a list of closures which modify this table before it is used.
-- This allows for succinct definitions of traits that modify multiple values.

local M = nilprotect {} -- submodule

--- Create functions for operating on global, mutable tables.
-- @param resource_creation_function Optional, the function which creates the resource, given a table of values. Returns the same table by default.
-- @param provide_name_lookup Optional, whether to allow for lookup by name. True by default.
-- @param should_preprocess whether to run the array portion as functions operating on the table. True by default.
-- TODO Eventually sort out conflicting names via virtual paths.
function M.type_create(--[[Optional]] resource_creation_function, --[[Optional]] provide_name_lookup, --[[Optional]] should_preprocess)
    -- Convert nil -> true for provide_name_lookup & should_preprocess.
    provide_name_lookup = (provide_name_lookup == nil) or provide_name_lookup
    should_preprocess = (should_preprocess == nil) or should_preprocess

    local R = nilprotect {} -- Resource table
    local next_id = 1
    R.resource_list = {}
    R.resource_name_table = provide_name_lookup and {} or nil

    function R.define(table)
    	if should_preprocess then
    		M.resolve_preprocess_functions(table)
    	end
    	-- Call the resource creation function, if one was provided.
        local entry = resource_creation_function and resource_creation_function(table) or table
        entry.id = #R.resource_list + 1
        R.resource_list[entry.id] = entry
        if provide_name_lookup then
            R.resource_name_table[entry.name] = entry
        end
        return entry
    end

    function R.lookup(key)
        local t = type(key)
        if t == "string" then
            if provide_name_lookup then return R.resource_name_table[key] end
            error("Attempt to lookup entry with name '" .. key .. "' for resource type that does not have a name.")
        elseif t == "number" then
            return R.resource_list[key]
        else
            error("Attempt to lookup entry with unsupported key type '" .. t .. "'. Key was " .. pretty_tostring(key))
        end
    end

    return R
end

--- Resolves a resource argument table, by taking a table, calling its array portion as closures, then removing the array portion.
-- @param args the table with an embedded list of closures. The table is updated by this operation.
function M.resolve_preprocess_functions(args)
    for i=1,#args do args[i](args) end
    for i=#args,1 do args[i] = nil end
end

return M