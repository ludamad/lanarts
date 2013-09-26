local Actions = import "@Actions"
local GameMap = import "@GameMap"
local SUtils = import "@serialization.SerializationUtils"

local M = nilprotect {} -- Submodule

M.MoveIOAction = {
    _types = {SUtils.XY},
    prerequisite = function(player, xy)
        return not GameMap.object_solid_check(player)
    end,
    apply = function(player, xy)
        player.xy = xy
    end
}

M.AutotargetObjectIOAction = {
    _types = {SUtils.NUM},
    prerequisite = function(player, target_id)
        return not GameMap.object_solid_check(player)
    end,
    apply = function(player, xy)
        player.xy = xy
    end
}

M.action_types = {
    M.MoveAction
}

for i,action in ipairs(M.action_types) do
    action.id = i
end

return M