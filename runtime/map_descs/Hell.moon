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

PolyPartition = require "core.PolyPartition"
DebugUtils = require "maps.DebugUtils"
NewMaps = require "maps.NewMaps"
NewDungeons = require "maps.NewDungeons"
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
{:place_feature, :place_vault} = require "maps.01_Overworld"
Map = require "core.Map"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

{:MapCompilerContext, :make_on_player_interact} = require "maps.MapCompilerContext"
{:MapNode, :MapDesc} = require "maps.MapDesc"
Places = require "maps.Places"

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

{:center, :bbox, :find_bbox, :find_square, :selector_filter, :selector_map} = require "maps.MapRegionUtils"

{:load_map_polys, :node_paint_group} = require "maps.MapNodeFills"

tileset = Tilesets.hell
hell_create = (rng, back_links, forward_links) -> {
    map_label: "Hell"
    subtemplates: {DUNGEON_CONF(rng, tileset, 3)}
    w: 200, h: 200
    seethrough: false
    outer_conf: DUNGEON_CONF(rng, tileset)
    shell: 10
    default_wall: Tile.create(tileset.wall, true, true, {})
    post_poned: {}
    _create_stairs_up: (map) =>
        for link in *table.tconcat(back_links, forward_links)
            if not place_vault map, Vaults.hell_entrance_vault {
                rng: map.rng
                item_placer: link
                door_placer: (map, xy) ->
                    -- nil is passed for the default open sprite
                    MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
                :tileset
            }
                return false
        return true
    _spawn_enemies: (map) =>
        area = {0,0,map.size[1],map.size[2]}
        OldMaps.generate_from_enemy_entries(map, OldMaps.strong_hell, 25, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        return true
    _spawn_items: (map) =>
        area = {0,0,map.size[1],map.size[2]}
        for group in *{{ItemGroups.basic_items,20}, {ItemGroups.enchanted_items, 10}, {{item: "Scroll of Experience", chance: 100}, 2}}
            for i=1,group[2] do
                sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
                if not sqr
                    break
                map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
                item = ItemUtils.item_generate group[1]
                MapUtils.spawn_item(map, item.type, item.amount, sqr)
        -- ~8 level 1 randarts:
        for i=1,OldMaps.adjusted_item_amount(8) do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.randart_generate(1) -- power level 1
            MapUtils.spawn_item(map, item.type, item.amount, sqr)
        -- ~2 level 2 randarts:
        for i=1,OldMaps.adjusted_item_amount(2) do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.randart_generate(2) -- power level 1
            MapUtils.spawn_item(map, item.type, item.amount, sqr)
        return true
    on_create_source_map: (map) =>
        if not @_create_stairs_up(map)
            return nil
        if not @_spawn_items(map)
            return nil
        if not @_spawn_enemies(map)
            return nil
        NewMaps.generate_door_candidates(map, rng, map.regions)
        return true
    on_create_game_map: (game_map) =>
        for f in *@post_poned
            f(game_map)
        Map.set_vision_radius(game_map, 6)
}

return MapDesc.create {
    map_label: "Hell"
    size: {200, 200}
    default_content: Tilesets.hive.wall
    children: {
        MapNode.create {
            place: () =>
                template = make_template(@map.rng, @desc.back_links, @desc.forward_links)
                if not NewMaps.map_try_create(@map, @map.rng, template)
                    return false
                if not template\on_create_source_map(@map)
                    return false
                return true
        }
    }
}

