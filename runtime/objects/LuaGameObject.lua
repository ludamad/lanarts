local GameObject = require "core.GameObject"

local assert, do_nothing, tmerge, setmetatable, rawget = assert, do_nothing, table.merge_into, setmetatable, rawget
local getmetatable = getmetatable

local M = {}

function M.type_create(T)
    T = T or {}
    base = T.base or GameObject.Base
    T.base = nil
    tmerge(T, base)

    function T.is_instance(obj, T)
        return (getmetatable(obj).parents[T] ~= nil)
    end

    function T.create(...)
        local val = setmetatable({}, T)
        T.init(val, ...)
        return val
    end

    local base_index = base.__index
    function T:__index(k)
        local v = rawget(T, k)
        if v ~= nil then return v end
        return base_index(self, k)
    end

    T.parent_init = assert(base.init, "Base object type does not define 'init'!")
    T.parent_on_step = base.on_step or do_nothing
    T.parent_on_draw = base.on_draw or do_nothing

    -- Ensure something
    T.on_step = T.on_step or do_nothing
    T.on_draw = T.on_draw or do_nothing

    T.parents = {[base] = true}
    if base.parents then
        tmerge(T.parents, base.parents)
    end

    return T
end

return M
