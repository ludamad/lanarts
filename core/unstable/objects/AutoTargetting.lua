local ObjectUtils = import "lanarts.objects.ObjectUtils"

local M = nilprotect {} -- Submodule

-- These functions operate on an ActionResolver and return a target obj
-- Return nil if no viable option (except NONE, which always returns nil)
local kinds = {
    NONE = do_nothing,
    CLOSEST_HOSTILE_XY = function(resolver, obj)
        return ObjectUtils.find_closest_hostile(obj).xy
    end,
    CLOSEST_HOSTILE_OBJ = function(resolver, obj)
        return ObjectUtils.find_closest_hostile(obj)
    end
}

return M