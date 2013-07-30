-- Convenience functions for dealing with resources.
-- A resource must have a field named 'id'.
-- Name and description are other typical fields for which convenience functions are provided.
-- Resources are defined using a table of values, and a list of closures which modify this table before it is used.
-- This allows for succint definitions of traits that modify multiple values.
-- TODO: Test resource persisting

require "utils"

local M = {} -- submodule

--- Create functions which share 'global' tables
function M.resource_table_create(resource_type, --[[Optional]] provide_name_lookup)
    local resource_list, next_id = {}, 1
    local rtable = {}
    local resource_name_table = provide_name_lookup and {} or nil

    function rtable.entry_create(table)
        local entry = resource_type.create(table)
        table.insert(resource_list, entry)
        if provide_name_lookup then 
            resource_name_table[entry.name] = entry
        end
    end

    function rtable.entry_lookup(key)
        local t = type(key)
        if t == "string" then
            if resource_name_table then return resource_name_table[key] end
            error("Attempt to lookup entry with name '" .. key .. "' for resource type that does not have a name.")
        elseif t == "number" then
            return resource_list[key]
        else
            error("Attempt to lookup entry with unsupported key type '" .. t .. "'. Key was " .. pretty_tostring(key))
        end
    end
end

--- Resolves a resource argument table, by taking a table, calling its array portion as closures, then removing the array portion.
function M.resource_arguments_resolve(args)
    for i=1,#args do args[i](args) end
    for i=#args,1 do args[i] = nil end
    return args
end

return M
