local Relations = import "@objects.Relations"
local Keys = import "core.keyboard"
local Projectiles = import "@objects.Projectiles"
local DungeonFeatures = import "@objects.DungeonFeatures"

local function player_on_step(self)
    local single_player = (settings.connection_type == net.NONE)
    if single_player and key_pressed('L') then
       Projectiles.TestProjectile.create(self, self.xy)
    end
end

-- Return submodule
return {
    player_init = function(self)
        self.team = Relations.TEAM_PLAYER_DEFAULT
        self.on_step = player_on_step
    end
}