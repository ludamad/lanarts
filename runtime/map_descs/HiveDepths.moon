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
{:place_feature} = require "maps.01_Overworld"
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

node_dungeon_room_template = () =>
    area_temp = SourceMap.area_template_create {
        data_file: @map.rng\random_choice {"maps/dungeon1room1a.txt", "maps/dungeon1room1b.txt", "maps/dungeon1room1c.txt"}
        legend: {
            'x': {add: SourceMap.FLAG_SOLID, content: Tilesets.hive.wall}
            '.': {add: SourceMap.FLAG_SEETHROUGH, content: if chance .5 then Tilesets.hive.floor else Tilesets.hive.floor_alt}
        }
    }
    orientation = @map.rng\random_choice {
        SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y
        SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
    }
    {cx, cy} = center @region_set
    {w, h} = area_temp.size
    if orientation == SourceMap.ORIENT_TURN_90 or orientation == SourceMap.ORIENT_TURN_270
        w, h = h, w
    area_temp\apply {
        map: @map,
        top_left_xy: {cx - w/2, cy - h/2}
        :orientation
    }
    return true

node_fill_dungeon_room = () =>
    rbbox = bbox @region_set
    for back_link in *@desc.back_links
        xy = MapUtils.random_square(@map, nil, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        if not xy
            return false
        back_link(@map, xy)
    for forward_link in *@desc.forward_links
        xy = MapUtils.random_square(@map, nil, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        if not xy
            return false
        forward_link(@map, xy)
    spawn_enemies = (enemies) ->
        selector = {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}}
        monster_focus_point = MapUtils.random_square(@map, rbbox, selector)
        if not monster_focus_point
            return false
        for type in *enemies
            enemy = MapUtils.random_enemy(@map, type, rbbox, selector)
            if not enemy
                return false
            enemy.monster_wander_position = () => monster_focus_point
        return true
    spawn_item_vaults = () ->
        for type in *{"Dandelite Key", false, false}
            item_placer = (map, xy) ->
                amount = 1
                if not type
                    {:type, :amount} = ItemUtils.item_generate ItemGroups.enchanted_items
                MapUtils.spawn_item(map, type, amount, xy)
            tileset = Tilesets.hive
            vault = SourceMap.area_template_create(Vaults.small_item_vault {rng: @map.rng, :item_placer, :tileset})
            if not place_feature(@map, vault, @region_set.regions)
                return false
        return true
    if not spawn_enemies {
        "Queen Bee"
        "Giant Bee"
        "Giant Bee"
        "Giant Bee"
        "Giant Bee"
        "Mouther"
        "Mouther"
        "Waxanarian"
    }
        return false
    if not spawn_item_vaults()
        return false
    return true

return MapDesc.create {
    map_label: "Hive Depths"
    size: {130, 130}
    default_content: Tilesets.hive.wall
    children: {
        MapNode.create {
            place: node_dungeon_room_template
            children: {
                MapNode.create {
                    place: node_fill_dungeon_room
                }
            }
        }
    }
}
