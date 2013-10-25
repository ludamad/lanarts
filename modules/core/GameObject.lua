-- Extension to internal 'core.GameObject'
local M = import_internal "core.GameObject"

local assert, do_nothing, tmerge, setmetatable = assert, do_nothing, table.merge, setmetatable
local getmetatable = getmetatable

function M.is_type(obj, T)
    return getmetatable(obj).parents[T]
end

function M.type_create(base)
    base = base or M.Base
    local T = {}
    function T.create(...)
        local val = setmetatable({}, T)
        T.init(val, ...)
        return val
    end

    tmerge(T, base)

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
