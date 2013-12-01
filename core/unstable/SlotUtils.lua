local rawget,rawset,type = _G.rawget,_G.rawset,_G.type -- Cache
local M = nilprotect {} -- Submodule

-- Shared by spells and items
M.METATABLE = {}

function M.METATABLE:__copy(obj)
    local type = self.type
    self.type = nil
    table.deep_copy(self, obj, --[[Do not invoke meta]] false)
    self.type, obj.type = type, type
end

function M.METATABLE:__index(k)
    -- Look in slot type
    local v = rawget(self, 'type')[k]
    if type(v) == 'function' then
        if k:find("on_") == 1 then -- Decide based on naming convention
            return v -- It was an 'on_' function
        else
            return v(self)
        end
    end
    return v
end

function M.clear_cache(self)
    table.clear(self._cache)
end

return M