local GameMap = import "core.GameMap"
local Actions = import "@Actions"
local SUtils = import "@serialization.SerializationUtils"

local M = nilprotect {} -- Submodule

M.MoveIOAction = {
    _types = {SUtils.XY},
    check = function(player, xy)
        return not GameMap.object_solid_check(player)
    end,
    apply = function(player, xy)
        player.xy = xy
    end
}

M.AutotargetObjectIOAction = {
    _types = {SUtils.NUM},
    check = function(player, target_id)
        return not GameMap.object_solid_check(player)
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
    action:check(player, ...)
end

function M.use_io_action(player, action, ...)
    action:apply(player, ...)
end

return M