local Relations = import "@objects.relations"

-- Return submodule
return {
    player_init = function(self)
        self.team = Relations.TEAM_PLAYER_DEFAULT
    end
}