local M = nilprotect {} -- Submodule

local sound_cache = {}

function M.lookup(name)
    if not sound_cache[name] then 
        sound_cache[name] = sound_optional_load(path_resolve(name) .. ".ogg")
    end
    return sound_cache[name]
end

return M