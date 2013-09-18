local GameMap = import "core.GameMap"
local MonsterType = import "@MonsterType"
local MapGen = import "core.MapGeneration"
local Display = import "core.Display"

local PlayerObject = import "@objects.PlayerObject"
local MonsterObject = import "@objects.MonsterObject"

local MapUtils = import "lanarts.maps.MapUtils"
local TileSets = import "lanarts.tiles.Tilesets"

local SimulationMap = newtype()

function SimulationMap:init()
    self.map = MapUtils.area_template_to_map("LanartsExampleLevel", path_resolve "test_content/test-map.txt", 0, {
        ['.'] =  { add = MapGen.FLAG_SEETHROUGH, content = TileSets.pebble.floor },
        ['x'] =  { add = {MapGen.FLAG_SEETHROUGH,MapGen.FLAG_SOLID}, content = TileSets.pebble.wall }
    })
	self.gmap = GameMap.create { map = self.map }
    self.monsters = {}
    self.players = {}
end

function SimulationMap:add_player(name, race, class)
    local tile_xy = MapUtils.random_square(self.map)
    local player_object = PlayerObject.create {
        name = name,
        race = race,
        class = class,
        xy = vector_scale(tile_xy, 32),
        is_local_player = function() return true end,
        do_init = true,
        map = self.gmap
    }
    table.insert(self.players, player_object)
    return player_object
end

function SimulationMap:add_monster(monster)
    local tile_xy = MapUtils.random_square(self.map)
    local monster_object = MonsterObject.create {
        monster_type = monster,
        xy = vector_scale(tile_xy, 32),
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
    GameMap.map_draw({map = self.gmap, reveal_all = true})
end

return SimulationMap