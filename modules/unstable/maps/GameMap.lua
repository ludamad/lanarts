local Map = import "core.Map"
local GameState = import "core.GameState"
local MonsterType = import "@MonsterType"
local SourceMap = import "core.SourceMap"
local Display = import "core.Display"

local PlayerObject = import "@objects.PlayerObject"
local MonsterObject = import "@objects.MonsterObject"

local MapUtils = import "lanarts.maps.MapUtils"
local TileSets = import "lanarts.tiles.Tilesets"
    
local ColAvoid = import "core.CollisionAvoidance"

local GameMap = newtype()

function GameMap:init(source_map, map)
    self.source_map = source_map
    self.map = map
    self.collision_group = ColAvoid.collision_group_create()
    self.monsters = {}
    self.players = {}
end

function GameMap:add_player(name, race, class)
    local xy = MapUtils.random_square(self.source_map)
    local player_object = PlayerObject.create {
        name = name,
        race = race,
        class = class,
        collision_group = self.collision_group,
        xy = MapUtils.from_tile_xy(xy),
        is_local_player = function() return true end,
        do_init = true,
        map = self.map
    }
    SourceMap.rectangle_apply {
        map=self.source_map, area = {xy[1]-3,xy[2]-3,xy[1]+3,xy[2]+3},
        fill_operator = {add = SourceMap.FLAG_HAS_OBJECT}
    }
    table.insert(self.players, player_object)
    return player_object
end

function GameMap:add_monster(monster)
    local tile_xy = MapUtils.random_square(self.source_map)
    local monster_object = MonsterObject.create {
        monster_type = monster,
        xy = MapUtils.from_tile_xy(tile_xy),
        collision_group = self.collision_group,
        do_init = true,
        map = self.map
    }
    table.insert(self.monsters, monster_object)
    return monster_object
end

function GameMap:step()
    local objects = Map.objects_list(self.map)
    Display.view_follow(self.players[1].xy)
    for _, obj in ipairs(objects) do
        if obj.on_prestep then obj:on_prestep() end
    end
    for _, obj in ipairs(objects) do
        obj:on_step()
    end
    self.collision_group:step()
    Map.map_step({map = self.map})
end

function GameMap:draw()
    local objects = Map.objects_list(self.map)
    Map.tiles_predraw(self.map)
    for _, obj in ipairs(objects) do
        if obj.on_predraw then obj:on_predraw() end
    end
    for _, obj in ipairs(objects) do
        obj:on_draw()
    end
    Map.instances_draw(self.map)
    Map.tiles_postdraw(self.map)
end

return GameMap