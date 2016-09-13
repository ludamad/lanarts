local Serial = require "core.Serialization"

local M = nilprotect {} -- Submodule

local tconcat, type, pairs = table.concat, type, pairs

local function name_subobjects(t, to_object, to_name, parts, depth)
    if to_name[t] then return end -- Already named
    local name = tconcat(parts, ';')
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
    name_subobjects(t, to_object, to_name, parts, #parts + 1)
end

function M.import_fallback(_context, str)
    if str:sub(1,3) == "_I:" then
        local mname = str:match("^_I%:[^%;]+")
        local t = require(mname:sub(4))
        M.name_subobjects(t, Serial.index_object_dictionary, Serial.object_index_dictionary, mname)
    elseif str:sub(1,3) == "_R:" then
        local mname = str:match("^_R%:[^%;]+")
        local t =  require(mname:sub(4))
        M.name_subobjects(t, Serial.index_object_dictionary, Serial.object_index_dictionary, mname)
    end
    return Serial.index_object_dictionary[str]
end

function M.install_import_fallback()
    Serial.set_name_resolution_fallback(M.import_fallback)
end

function M.name_global_data(--[[Optional]] to_object, --[[Optional]] to_name)
    assert((to_object == nil) == (to_name == nil), "Specify both or none.")
    to_object = to_object or Serial.index_object_dictionary
    to_name = to_name or Serial.object_index_dictionary
    for k,v in pairs(_INTERNAL_IMPORTED) do
        M.name_subobjects(v, to_object, to_name, "_I:"..k)
    end
    for k,v in pairs(_IMPORTED) do
        if #v > 0 then
            M.name_subobjects(v[1], to_object, to_name, "_I:"..k)
        end
    end
    -- For 'require' compatibility
    for k,v in pairs(_LOADED) do
        M.name_subobjects(v, to_object, to_name, "_R:"..k)
    end
    M.name_subobjects(_G, to_object, to_name, "_G:")
end

return M
