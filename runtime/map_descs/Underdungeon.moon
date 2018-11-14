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

{:generate_map_node} = require "maps.01_Overworld"

overdungeon_items_and_enemies = (region_set) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        conf = region.conf
        for i=1,OldMaps.adjusted_item_amount(10) do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.item_generate ItemGroups.basic_items
            MapUtils.spawn_item(map, item.type, item.amount, sqr)

        OldMaps.generate_from_enemy_entries(map, OldMaps.hard_enemies, 10, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        OldMaps.generate_from_enemy_entries(map, OldMaps.fast_enemies, 10, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})

overdungeon_features = (region_set) ->
    {:map, :regions} = region_set

    OldMapSeq4 = MapSequence.create {preallocate: 1}

    append map.post_game_map, (game_map) ->
        OldMapSeq4\slot_resolve(1, game_map)

    -----------------------------
    -- Purple Dragon lair            --
    place_purple_dragon_lair = () ->
        name = "Purple Dragon Lair"
        create_compiler_context = () ->
            cc = MapCompilerContext.create()
            cc\register(name, Places.DragonLair)
            append map.post_game_map, (game_map) ->
                cc\register("root", () -> game_map)
            return cc
        cc = create_compiler_context()
        return place_vault_in region_set, Vaults.graghs_lair_entrance {
            tileset: Tilesets.hell
            dungeon_placer: (map, xy) ->
                -- Make portal
                portal = MapUtils.spawn_portal(map, xy, "spr_gates.volcano_portal")
                other_portal = cc\add_pending_portal name, (feature, compiler) ->
                    MapUtils.random_portal(compiler.map, nil, "spr_gates.volcano_exit")
                other_portal\connect {feature: portal, label: "root"}
                portal.on_player_interact = make_on_player_interact(cc, other_portal)
                return portal
            door_placer: (map, xy) ->
                MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
            enemy_placer: (map, xy) ->
                enemy = OldMaps.enemy_generate({{enemy: "Fire Bat", chance: 100}})
                MapUtils.spawn_enemy(map, enemy, xy)
        }
    if not place_purple_dragon_lair()
        return false

    if not place_outpost(region_set)
        return false
    -----------------------------
    -- Place optional dungeon 2, the crypt: --
    place_crypt = () ->
        crypt = require("map_descs.Crypt")\linker()
        hell = require("map_descs.Hell")\linker()
        crypt\link_linker(hell, 'spr_gates.enter_hell3', 'spr_gates.return_hell')
        return place_vault_in region_set, Vaults.crypt_dungeon {
            tileset: Tilesets.crypt
            door_placer: (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
            dungeon_placer: (map, xy) ->
                portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_crypt")
                crypt\link_portal(portal, 'spr_gates.exit_dungeon')
            enemy_placer: (map, xy) ->
                enemy = OldMaps.enemy_generate(OldMaps.strong_undead)
                MapUtils.spawn_enemy(map, enemy, xy)
        }
    if not place_crypt()
        print "RETRY: place_crypt()"
        return false
    -----------------------------

    if not place_graghs_lair(region_set)
        return false

    if not place_pixullochia(region_set)
        return false

    append map.post_maps, () ->
        overdungeon_items_and_enemies(region_set)

    return true

pebble_overdungeon = (rng) ->
    -- CONFIG
    arc_chance = 0.05
    size = rng\random_choice {{125, 85}, {85, 125}}
    number_regions = rng\random(5, 7)
    connect_line_width = () -> rng\random(2, 6)
    default_wall = Tile.create(Tilesets.pebble.wall, true, true, {SourceMap.FLAG_SOLID})
    room_radius = () -> rng\random(5,10)

    return generate_map_node () ->
        map = NewMaps.source_map_create {
            :rng
            size: {OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE}
            default_content: default_wall.id
            default_flags: {SourceMap.FLAG_SOLID}
            map_label: "Underdungeon"
            :arc_chance
        }
        template = nilprotect {
            :default_wall
            subtemplates: {nilprotect {
                is_overworld: true
                :size
                :number_regions
                floor1: DUNGEON_TILESET.floor1
                floor2: DUNGEON_TILESET.floor2
                wall1: DUNGEON_TILESET.wall1
                wall2: DUNGEON_TILESET.wall2
                rect_room_num_range: {4,8} -- disable
                rect_room_size_range: {14,20}
                rvo_iterations: 100
                :connect_line_width
                :room_radius
                region_delta_func: spread_region_delta_func
                -- Dungeon objects/features
                n_statues: 4
            }}
            outer_conf: nilprotect {
                floor1: DUNGEON_TILESET.floor1
                floor2: DUNGEON_TILESET.floor2
                :connect_line_width
            }
            shell: 25
        }
        if not NewMaps.map_try_create(map, rng, template)
            return nil
        full_region_set = {:map, regions: map.regions}
        append map.post_maps, () ->
            place_doors_and_statues(full_region_set)
        if not overdungeon_features(full_region_set)
            return nil
        return map


return MapDesc.create {
    size: {20, 20}
    children: {
        MapNode.create {
            place: node_place_easy_overworld_rooms
        }
    }
}

underdungeon_create = (links) ->
    {:map} = pebble_overdungeon NewMaps.new_rng()
    for link in *links
        xy = MapUtils.random_square(map, nil, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        if not xy
            return false
        link(map, xy)
    game_map = NewMaps.generate_game_map(map)
    for f in *map.post_game_map
        f(game_map)
    return game_map


