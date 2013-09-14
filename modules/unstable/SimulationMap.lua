local GameMap = import "core.GameMap"
local MonsterType = import "@MonsterType"
local MapGen = import "core.MapGeneration"

local PlayerObject = import "@objects.PlayerObject"
local MonsterObject = import "@objects.MonsterObject"

local SimulationMap = newtype()

function SimulationMap:init()
	self.gmap = GameMap.create { 
        map = MapGen.map_create { 
            label = "TestLevel", 
            size = {256,256},            
            content = Data.tile_create { 
                images = {
                    image_cached_load (path_resolve "test_content/test_tile.png")
                }
            }
        }
    }
    self.monsters = {}
    self.players = {}
end

function SimulationMap:add_player(name, race, class, xy)
    local player_object = PlayerObject.create {
        name = name,
        race = race,
        class = class,
        sprite = {}, -- TODO
        xy = xy,
        is_local_player = function() return true end,
        do_init = true,
        map = self.gmap
    }
    table.insert(self.players, player_object)
    return player_object
end

function SimulationMap:add_monster(monster, xy)
    local monster_object = MonsterObject.create {
        monster_type = monster,
        xy = xy,
        do_init = true,
        map = self.gmap
    }
    table.insert(self.monsters, monster_object)
    return monster_object
end

function SimulationMap:step()
    GameMap.map_step({map = self.gmap})
end

function SimulationMap:draw()
    GameMap.map_draw({map = self.gmap})
end

return SimulationMap