local GameMap = import "core.GameMap"
local GameState = import "core.GameState"
local MonsterType = import "@MonsterType"
local MapGen = import "core.MapGeneration"
local Display = import "core.Display"

local PlayerObject = import "@objects.PlayerObject"
local MonsterObject = import "@objects.MonsterObject"

local MapUtils = import "lanarts.maps.MapUtils"
local TileSets = import "lanarts.tiles.Tilesets"

local SimulationMap = newtype()

function SimulationMap:init(map, gmap)
    self.map = map
    self.gmap = gmap
    self.monsters = {}
    self.players = {}
end

function SimulationMap:add_player(name, race, class)
    local tile_xy = MapUtils.random_square(self.map)
    local player_object = PlayerObject.create {
        name = name,
        race = race,
        class = class,
        xy = MapUtils.from_tile_xy(tile_xy),
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
        xy = MapUtils.from_tile_xy(tile_xy),
        do_init = true,
        map = self.gmap
    }
    table.insert(self.monsters, monster_object)
    return monster_object
end

function SimulationMap:step()
    Display.view_follow(self.players[1].xy)
    GameMap.map_step({map = self.gmap})
end

function SimulationMap:draw()
    GameMap.map_draw({map = self.gmap})
end

return SimulationMap