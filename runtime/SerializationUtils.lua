local Serialization = require "core.Serialization"

local M = nilprotect {} -- Submodule

local tconcat, type, pairs = table.concat, type, pairs

local function name_subobjects(t, to_object, to_name, parts, depth)
    if to_name[t] then return end -- Already named
    local name = tconcat(parts, ';')
    -- print("NAMING ", name)
    to_object[name], to_name[t] = t, name
    if type(t) == "table" then
        for k, v in pairs(t) do
            if type(k) == "string" then
                parts[depth] = k
                name_subobjects(v, to_object, to_name, parts, depth + 1)
            end
        end
        parts[depth] = nil
    end
end

function M.name_subobjects(t, to_object, to_name, --[[Optional]] base_string)
    local parts = {base_string}
    if t ~= package then
        name_subobjects(t, to_object, to_name, parts, #parts + 1)
    end
end

function M.require_fallback(_context, str)
    if str:sub(1,3) == "_R:" then
        local mname = str:match("^_R%:[^%;]+")
        local t =  require(mname:sub(4))
        -- print(mname, Serialization.index_object_dictionary[mname])
        M.name_subobjects(t, Serialization.index_object_dictionary, Serialization.object_index_dictionary, "_R:"..mname:sub(4))
    end
    return Serialization.index_object_dictionary[str]
end

function M.install_require_fallback()
    Serialization.set_name_resolution_fallback(M.require_fallback)
end

function M.name_global_data(--[[Optional]] to_object, --[[Optional]] to_name)
    assert((to_object == nil) == (to_name == nil), "Specify both or none.")
    to_object = to_object or Serialization.index_object_dictionary
    to_name = to_name or Serialization.object_index_dictionary
    for mname,v in pairs(package.loaded) do
        if mname ~= "_G" and mname ~= "core.GlobalData" then
            M.name_subobjects(v, to_object, to_name, "_R:"..mname)
        end
    end
    local old_package = package
    M.name_subobjects(_G, to_object, to_name, "_G:")
end

return M
