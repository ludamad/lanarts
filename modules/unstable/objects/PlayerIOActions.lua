local GameMap = import "core.Map"
local Actions = import "@Actions"
local SUtils = import "@serialization.SerializationUtils"

local M = nilprotect {} -- Submodule

M.MoveIOAction = {
    _types = {SUtils.XY},
    check = function(player, xy)
        return not GameMap.object_solid_check(player, xy)
    end,
    apply = function(player, xy)
        player.xy = xy
    end
}

M.action_types = {
    M.MoveIOAction
}

for i,action in ipairs(M.action_types) do
    action.id = i
end

function M.can_use_io_action(player, action, ...)
    return action.check(player, ...)
end

function M.use_io_action(player, action, ...)
    action.apply(player, ...)
end

return M