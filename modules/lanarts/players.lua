local Relations = import "@objects.Relations"
local Keys = import "core.Keyboard"
local Projectiles = import "@objects.Projectiles"
local DungeonFeatures = import "@objects.DungeonFeatures"

local PLAYER_TRAIT = "player"

local function player_on_step(self)
    local single_player = (settings.connection_type == net.NONE)
end

-- Return submodule
return {
    player_init = function(player)
        player.team = Relations.TEAM_PLAYER_DEFAULT
        player.on_step = player_on_step
        player.traits = {PLAYER_TRAIT}
    end,
    is_player = function(player)
        return table.contains(player.traits, PLAYER_TRAIT)
    end,
    is_local_player = function(player)
        return table.contains(player.traits, PLAYER_TRAIT) and player:is_local_player()
    end
}