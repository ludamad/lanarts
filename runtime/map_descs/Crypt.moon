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
{:place_feature, :place_vault, :DUNGEON_CONF} = require "maps.01_Overworld"
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

make_template = (rng, back_links, forward_links) -> {
    map_label: "Crypt"
    w: 110, h: 100
    seethrough: false
    outer_conf: DUNGEON_CONF(rng, Tilesets.crypt, nil, 10)
    subtemplates: {DUNGEON_CONF(rng, Tilesets.crypt, 4, 10)}
    shell: 10
    default_wall: Tile.create(Tilesets.crypt.wall, true, true, {})
    _create_encounter_rooms: (map) =>
        _items_placed = 0
        for i =1,2
            if not place_vault map, Vaults.crypt_encounter_vault {
                rng: map.rng
                enemy_placer: (map, xy) ->
                    enemy = OldMaps.enemy_generate(OldMaps.weak_undead)
                    MapUtils.spawn_enemy(map, enemy, xy)
                door_placer: (map, xy) ->
                    -- nil is passed for the default open sprite
                    MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
                store_placer: (map, xy) ->
                    Region1.generate_epic_store(map, xy)
                item_placer: (map, xy) ->
                    local item
                    if _items_placed >= 1
                        item = ItemUtils.item_generate ItemGroups.enchanted_items
                    else
                        item = {type: "Tomb Lanart", amount: 1}
                    MapUtils.spawn_item(map, item.type, item.amount, xy)
                    _items_placed += 1
                tileset: Tilesets.crypt
            }
                return false
        return true

    -----------------------------
    -- Place optional dungeon 3, hell: --
    --_place_hell: (map) =>
    --    Seq = MapSequence.create {preallocate: 1}
    --    door_placer = (map, xy) ->
    --        -- nil is passed for the default open sprite
    --        MapUtils.spawn_door(map, xy, nil, Vaults._door_magentite, "Magentite Key")
    --    next_dungeon = {1}
    --    place_dungeon = (map, xy) ->
    --        portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_hell1")
    --        c = (Seq\forward_portal_add 1, portal, next_dungeon[1], () -> hell_create(Seq, 2))
    --        if World.player_amount > 1
    --            append map.post_maps, c
    --        next_dungeon[1] += 1
    --    enemy_placer = (map, xy) ->
    --        enemy = OldMaps.enemy_generate(OldMaps.strong_undead)
    --        MapUtils.spawn_enemy(map, enemy, xy)
    _place_down_chambers: () ->
        for link in *forward_links
            if not place_vault map, Vaults.hell_dungeon {
                dungeon_placer: link
                tileset: Tilesets.hell
                door_placer: (map, xy) ->
                    -- nil is passed for the default open sprite
                    MapUtils.spawn_door(map, xy, nil, Vaults._magentite_door, "Magentite Key")
                enemy_placer: (map, xy) ->
                    enemy = OldMaps.enemy_generate(OldMaps.strong_undead)
                    MapUtils.spawn_enemy(map, enemy, xy)
            }
                return false
        return true
    _create_stairs_up: (map) =>
        for link in *back_links
            if place_vault map, Vaults.crypt_entrance_vault {
                rng: map.rng
                item_placer: link
                door_placer: (map, xy) -> MapUtils.spawn_door(map, xy)
                tileset: Tilesets.crypt
            }
                return false
        return true
    _spawn_enemies: (map) =>
        area = {0,0,map.size[1],map.size[2]}
        OldMaps.generate_from_enemy_entries(map, OldMaps.strong_undead, 25, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        return true
    _spawn_items: (map) =>
        area = {0,0,map.size[1],map.size[2]}
        for group in *{{ItemGroups.enchanted_items, 5}, {{item: "Scroll of Experience", chance: 100}, 1}}
            for i=1,group[2] do
                sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
                if not sqr
                    break
                map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
                item = ItemUtils.item_generate group[1]
                MapUtils.spawn_item(map, item.type, item.amount, sqr)
        return true
    on_create_source_map: (map) =>
        if not @_place_down_chambers(map)
            print("Chamber fail")
            return false
        if not @_create_encounter_rooms(map)
            print("Encounter fail")
            return false
        if not @_create_stairs_up(map)
            print("Stairs up fail")
            return false
        if not @_spawn_items(map)
            print("Items fail")
            return false
        if not @_spawn_enemies(map)
            print("Enemies fail")
            return false
        NewMaps.generate_door_candidates(map, rng, map.regions)
        return true
}

return MapDesc.create {
    map_label: "Crypt"
    size: {130, 130}
    default_content: Tilesets.crypt.wall
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
