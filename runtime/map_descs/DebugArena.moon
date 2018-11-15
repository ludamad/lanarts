import map_place_object, ellipse_points,
    LEVEL_PADDING, Region, RVORegionPlacer,
    random_rect_in_rect, random_ellipse_in_ellipse,
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func,
    towards_region_delta_func,
    rectangle_points,
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

GeometryUtils = require "maps.GeometryUtils"
MapRegionShapes = require("maps.MapRegionShapes")
{:MapRegion, :combine_map_regions, :from_bbox} = require "maps.MapRegion"
{:MapNode, :MapDesc} = require "maps.MapDesc"

PolyPartition = require "core.PolyPartition"
DebugUtils = require "maps.DebugUtils"
NewMaps = require "maps.NewMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
import make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
    from MapUtils

MapSequence = require "maps.MapSequence"
Vaults = require "maps.Vaults"
World = require "core.World"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

{:MapCompilerContext, :make_on_player_interact} = require "maps.MapCompilerContext"
Places = require "maps.Places"

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

{:center, :find_bbox, :find_square, :selector_filter, :selector_map} = require "maps.MapRegionUtils"

PADDING = 10

event_log = (...) -> print string.format(...)

shrink = (bbox, x, y) ->
    {x1,y1,x2,y2} = bbox
    return {
        x1+x
        y1+y
        x2-x
        y2-y
    }

node_place_easy_overworld_rooms = () =>
    n_rooms = 1
    event_log("(RNG #%d) generating %d rooms", @rng\amount_generated(), n_rooms)
    regions = {}
    for i=1,n_rooms
        size = {10, 10}
        result = find_bbox(@region_set, size)
        if not result
            return false
        {:bbox} = result
        group = @new_group()
        region = from_bbox(unpack(bbox))
        region.inner_bbox = shrink(bbox, 1, 1)
        region.selector = {matches_group: {group, group}}
        append regions, region
        SourceMap.rectangle_apply {
            map: @map
            area: bbox
            fill_operator: {
                :group
                add: SourceMap.FLAG_SEETHROUGH
                remove: SourceMap.FLAG_SOLID
                content: Tilesets.pebble.floor
            }
            perimeter_width: 0
            perimeter_operator: {
                add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER}
                remove: SourceMap.FLAG_SEETHROUGH
                content: Tilesets.pebble.wall
            }
        }
        for type in *@desc.monster_spawns
            enemy = MapUtils.random_enemy(@map, type, region\bbox(), region.selector)
            if not enemy
                return false
    event_log("(RNG #%d) after generating %d rooms", @rng\amount_generated(), n_rooms)
    @region_set = {map: @map, :regions}
    return true

DebugArena = MapDesc.create {
    size: {20, 20}
    children: {
        MapNode.create {
            place: node_place_easy_overworld_rooms
        }
    }
}

MAX_MAP_TRIES = 100
generate = (portal_spawns, monster_spawns) ->
    DebugArena.monster_spawns = monster_spawns
    map = NewMaps.try_n_times MAX_MAP_TRIES, () -> DebugArena\compile(portal_spawns)
    game_map = NewMaps.generate_game_map(map)
    {x, y} = MapUtils.random_square(map, nil, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
    append map.player_candidate_squares, {x*32+16,y*32+16}
    player_spawn_points = MapUtils.pick_player_squares(map, map.player_candidate_squares)
    for post_poned in *map.post_game_map
        post_poned(game_map)
    return game_map, player_spawn_points

DebugArena.generate_and_spawn = (monster_spawns) =>
    game_map, player_spawn_points = generate({}, monster_spawns)
    World.players_spawn(game_map, player_spawn_points)
    return game_map

return DebugArena
