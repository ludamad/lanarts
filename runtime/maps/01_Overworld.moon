----
-- Generates the game maps, starting with high-level details (places that will be in the game)
-- and then generating actual tiles.
----
import map_place_object, ellipse_points,
    LEVEL_PADDING, Region, RVORegionPlacer,
    random_rect_in_rect, random_ellipse_in_ellipse,
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func,
    towards_region_delta_func,
    rectangle_points,
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

import MapRegion from require "maps.MapRegion"

NewMaps = require "maps.NewMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
import make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
    from MapUtils

MapSequence = require "maps.MapSequence"
{:MapLinker} = require "maps.MapLink"
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

M = nilprotect {} -- Module

OVERWORLD_VISION_RADIUS = 8

local underdungeon_create

create_overworld_scheme = (tileset) -> nilprotect {
    floor1: Tile.create(tileset.floor, false, true, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {FLAG_OVERWORLD})
    wall1: Tile.create(tileset.wall, true, true, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false)
}

create_dungeon_scheme = (tileset) -> nilprotect {
    floor1: Tile.create(tileset.floor, false, true, {}, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {}, {FLAG_OVERWORLD})
    wall1: Tile.create(tileset.wall, true, false, {}, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false, {}, {FLAG_OVERWORLD})
}

OVERWORLD_TILESET = create_overworld_scheme(Tilesets.grass)
DUNGEON_TILESET = create_dungeon_scheme(Tilesets.pebble)

OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE = 300, 300

TEST_CONF = {
    map_label: os.getenv("LANARTS_TESTCASE")
    is_overworld: true
    size: {100,100}
    number_regions: 0
    floor1: OVERWORLD_TILESET.floor1
    floor2: OVERWORLD_TILESET.floor2
    wall1: OVERWORLD_TILESET.wall1
    wall2: OVERWORLD_TILESET.wall2
    rect_room_num_range: {0,0}
    rect_room_size_range: {10,15}
    rvo_iterations: 0
    n_stairs_down: 0
    n_stairs_up: 0
    connect_line_width: () -> 1
    region_delta_func: spread_region_delta_func
    room_radius: 1
    n_statues: 0
}

OVERWORLD_CONF = (rng) ->
    type = rng\random_choice {
        {
            number_regions: rng\random(25, 30)
            region_delta_func: ring_region_delta_func
            room_radius: () ->
                r = 7
                bound = rng\random(1,20)
                for j=1,rng\random(0,bound) do r += rng\randomf(0, 1)
                return r
        }
        {
            number_regions: 4
            arc_chance: 1
            region_delta_func: spread_region_delta_func --center_region_delta_func
            room_radius: () ->
                return rng\random(10, 20)
        }
        {
            number_regions: rng\random(15, 22)
            region_delta_func: ring_region_delta_func
            arc_chance: 1
            room_radius: () ->
                return rng\random(5, 10)
        }
    }
    return {
        map_label: "Plain Valley"
        is_overworld: true
        size: if rng\random(0,2) == 0 then {85, 105} else {85, 105}
        number_regions: type.number_regions
        floor1: OVERWORLD_TILESET.floor1
        floor2: OVERWORLD_TILESET.floor2
        wall1: OVERWORLD_TILESET.wall1
        wall2: OVERWORLD_TILESET.wall2
        rect_room_num_range: {0,0}
        rect_room_size_range: {10,15}
        rvo_iterations: 150
        n_stairs_down: 0
        n_stairs_up: 0
        connect_line_width: () -> rng\random(2,6)
        region_delta_func: type.region_delta_func
        arc_chance: type.arc_chance or 0
        room_radius: type.room_radius
        -- Dungeon objects/features
        n_statues: 4
    }

DUNGEON_CONF = (rng, tileset = Tilesets.pebble, schema = 1, n_regions=nil) ->
    C = create_dungeon_scheme(tileset)
    -- Rectangle-heavy or polygon-heavy?
    switch schema -- rng\random(3)
        when 3
            -- Few, bigger, rooms?
            C.number_regions = n_regions or rng\random(15,20)
            C.room_radius = () ->
                r = 8
                for j=1,rng\random(5,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {2,2}
            C.rect_room_size_range = {7,15}
        when 4
            -- Few, big, rooms?
            C.number_regions = n_regions or rng\random(13,18)
            C.room_radius = () ->
                r = 5
                for j=1,rng\random(5,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {10,10}
            C.rect_room_size_range = {7,15}
        when 0
            -- Few, big, rooms?
            C.number_regions = n_regions or rng\random(15,20)
            C.room_radius = () ->
                r = 4
                for j=1,rng\random(5,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {10,10}
            C.rect_room_size_range = {7,15}
        when 1
            -- Mix?
            C.number_regions = n_regions or rng\random(15,20)
            C.room_radius = () ->
                r = 2
                for j=1,rng\random(0,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {2,10}
            C.rect_room_size_range = {7,15}
        when 2
            -- Mostly rectangular rooms?
            C.number_regions = n_regions or rng\random(2,7)
            C.room_radius = () ->
                r = 2
                for j=1,rng\random(0,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {10,15}
            C.rect_room_size_range = {7,15}

    return table.merge C, {
        map_label: "A Dungeon"
        size: if schema == 4 then {85, 85} else {65,65}
        rvo_iterations: 20
        n_stairs_down: 3
        n_stairs_up: 0
        connect_line_width: () -> 2 + (if rng\random(5) == 4 then 1 else 0)
        region_delta_func: default_region_delta_func
        -- Dungeon objects/features
        n_statues: 4
    }

place_doors_and_statues = (region_set) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        conf = region.conf
        for xy in *SourceMap.rectangle_match {:map, selector: {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}, matches_all: {FLAG_DOOR_CANDIDATE}}}
            MapUtils.spawn_door(map, xy)
        for i=1,conf.n_statues
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
            MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0,17))

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

overworld_items_and_enemies = (region_set) ->
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

        OldMaps.generate_from_enemy_entries(map, OldMaps.medium_animals, 8, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        --OldMaps.generate_from_enemy_entries(map, OldMaps.medium_enemies, 5, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        --OldMaps.generate_from_enemy_entries(map, OldMaps.fast_enemies, 5, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})

place_feature = (map, template, regions=map.regions) ->
   event_log("(RNG #%d) placing feature", map.rng\amount_generated())
   -- Function to try a single placement, returns success:
   attempt_placement = (template) ->
       orient = map.rng\random_choice {
           SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y,
           SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
       }
       event_log("(RNG #%d) orient=%d", map.rng\amount_generated(), orient)
       for r in *regions
           {w, h} = template.size
           -- Account for rotation in size:
           if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180
               w, h = h, w
           {x1, y1, x2, y2} = r\bbox()
           -- Expand just outside the bounds of the region:
           x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
           -- Ensure we are within the bounds of the world map:
           x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
           top_left_xy = MapUtils.random_square(map, {x1, y1, x2, y2})
           apply_args = {:map, :top_left_xy, orientation: orient }
           if template\matches(apply_args)
               template\apply(apply_args)
               return true
       return false
   -- Function to try placement n times, returns success:
   attempt_placement_n_times = (template, n) ->
       for i=1,n
           event_log("(RNG #%d) placement=%d", map.rng\amount_generated(), i)
           if attempt_placement(template)
               return true
       return false
   -- Try to create the template object using our placement routines:
   if attempt_placement_n_times(template, 100)
       -- Exit, as we have handled the first overworld component
       return true
   return false

place_vault_in = (region_set, vault) ->
    template = SourceMap.area_template_create(vault)
    return place_feature(region_set.map, template, region_set.regions)

place_vault = (map, vault) ->
    template = SourceMap.area_template_create(vault)
    return place_feature(map, template)

safe_portal_spawner = (tileset) -> (map, map_area, sprite, callback, frame) ->
    portal_holder = {}
    portal_placer = (map, xy) ->
        portal_holder[1] = MapUtils.spawn_portal(map, xy, sprite, callback, frame)
    vault = SourceMap.area_template_create(Vaults.small_item_vault_multiway {rng: map.rng, item_placer: portal_placer, :tileset})
    -- TODO allow passing areas to use map_area, not critical for now
    if not place_feature(map, vault)
        return MapUtils.random_portal(map, map_area, sprite, callback, frame)
    assert(portal_holder[1])
    return portal_holder[1]

place_underdungeon_vault = (region_set) ->
    {:map, :regions} = region_set
    underdungeon = MapLinker.create {map_label: "Underdungeon", generate: (backwards) => underdungeon_create(backwards)}
    return place_vault_in region_set, Vaults.sealed_dungeon {
        rng: map.rng,
        tileset: Tilesets.snake
        door_placer: (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, 'Dandelite Key')
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_vaults_open")
            underdungeon\link_portal(portal, "spr_gates.exit_dungeon")
    }

place_hive = (region_set) ->
    {:map, :regions} = region_set
    entrance = require("map_descs.HiveEntrance")\linker()
    -- Link entrance to further depths
    depths = require("map_descs.HiveDepths")\linker()
    for i=1,3
        entrance\link_linker(depths, "spr_gates.enter_lair", "spr_gates.exit_lair")
    -- Link to hive entrance
    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.hive
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.hive_portal")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
    }

place_snake_pit = (region_set) ->
    {:map, :regions} = region_set
    entrance = require("map_descs.SnakePitEntrance")\linker()
    -- Link entrance to further depths
    depths = require("map_descs.SnakePitDepths")\linker()
    for i=1,3
        entrance\link_linker(depths, "spr_gates.enter", "spr_gates.return")
    -- Link to snake pit entrance
    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.snake
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_snake")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
        player_spawn_area: true
    }

OGRE_LAIR = OldMaps.create_map_desc {
    tileset: Tilesets.snake
    label: "Ogre Lair"
    layout: {{size: {60,40}, rooms: {padding:0,amount:40,size:{3,7}},tunnels:{padding:0, width:{1,3},per_room: 5}}}
    content: {
        items: {amount: 8, group: ItemGroups.enchanted_items}
        enemies: {
            wandering: true
            amount: 0
            generated: {
              {enemy: "Ogre Mage",         guaranteed_spawns: 5}
              {enemy: "Orc Warrior",       guaranteed_spawns: 3}
              {enemy: "Adder",             guaranteed_spawns: 5}
            }
        }
    }
    on_generate: (map) ->
        ----------------------------------------------------------
        for i=1,3
            item = ItemUtils.randart_generate(1)
            if not place_vault map, Vaults.small_item_vault {
                rng: map.rng
                item_placer: (map, xy) -> MapUtils.spawn_item(map, item.type, item.amount, xy)
                tileset: Tilesets.snake
            }
                return false
        ----------------------------------------------------------
        return true
}

outpost_map_descs = () ->
    last_floor_generate = (map) ->
        -------------------------
        -- Place key vault     --
        for i=1,2
            if not place_vault map, Vaults.small_item_vault {
                rng: map.rng
                item_placer: (map, xy) ->
                    item = ItemUtils.item_generate ItemGroups.basic_items
                    MapUtils.spawn_item(map, item.type, item.amount, xy)
                tileset: Tilesets.pebble
            }
                return false

        area = {0,0,map.size[1],map.size[2]}
        for i=1,4
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                return false
            Region1.generate_store(map, sqr)

        sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
        if not sqr
            return false
        return true
    return for floor,base in ipairs(OldMaps.Dungeon1)
        template = table.merge base, {
            tileset: Tilesets.pebble
            label: ({"Outpost Entrance", "Outpost Stockroom"})[floor]
            on_generate: if floor == #OldMaps.Dungeon1 then last_floor_generate else nil
        }
        OldMaps.create_map_desc(template)

{OUTPOST_ENTRANCE, OUTPOST_STOCKROOM} = outpost_map_descs()

place_outpost = (region_set) ->
    {:map, :regions} = region_set
    entrance, stockroom = OUTPOST_ENTRANCE\linker(), OUTPOST_STOCKROOM\linker()
    ogre_lair = OGRE_LAIR\linker()

    -- TODO hack, want better linking in linkers here
    OUTPOST_ENTRANCE.on_generate = (map) ->
        return place_vault map, Vaults.sealed_dungeon {
            tileset: Tilesets.snake
            gold_placer: (map, xy) ->
                MapUtils.spawn_item(map, "Gold", map.rng\random(2,10), xy)
            door_placer: (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
            dungeon_placer: callable_once (map, xy) ->
                portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_orc")
                ogre_lair\link_portal(portal, "spr_gates.exit_orc")
        }

    -- Link entrance & stockroom
    for i=1,3
        entrance\link_linker(stockroom, "spr_gates.enter", "spr_gates.return")

    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.snake
        gold_placer: (map, xy) -> nil -- dont need gold here
        door_placer: (map, xy) -> MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.desolation_portal")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
    }

temple_map_descs = () ->
    return for floor,base in ipairs(OldMaps.Dungeon2)
        template = table.merge base, {
            tileset: Tilesets.temple
            label: ({"Temple Entrance", "Temple Chamber", "Temple Inner Sanctum"})[floor]
            on_generate: (floor == 3) and (map) ->
                ---------------------------------------------------------------------
                -- Place key vault, along with a gold vault and 2 regular items.   --
                for type in *{"Azurite Key", "Scroll of Experience", false}
                    vault = SourceMap.area_template_create Vaults.small_item_vault {
                        rng: map.rng
                        item_placer: (map, xy) ->
                            amount = 1
                            if not type
                                {:type, :amount} = ItemUtils.item_generate ItemGroups.enchanted_items
                            MapUtils.spawn_item(map, type, amount, xy)
                        tileset: Tilesets.snake
                    }
                    if not place_feature(map, vault)
                        return false
                return true
        }
        OldMaps.create_map_desc(template)

{TEMPLE_ENTRANCE, TEMPLE_CHAMBER, TEMPLE_SANCTUM} = temple_map_descs()

place_temple = (region_set) ->
    {:map, :regions} = region_set
    entrance, chamber, sanctum = TEMPLE_ENTRANCE\linker(), TEMPLE_CHAMBER\linker(), TEMPLE_SANCTUM\linker()
    for i=1,3
        entrance\link_linker(chamber, "spr_gates.enter", "spr_gates.return")
        chamber\link_linker(sanctum, "spr_gates.enter", "spr_gates.return")

    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.temple
        gold_placer: (map, xy) -> nil -- dont need gold here
        door_placer: (map, xy) -> MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_temple")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
        player_spawn_area: true
    }

PIXULLOCHIA_ENTRANCE = OldMaps.create_map_desc table.merge OldMaps.Dungeon4[1], {
    tileset: Tilesets.pixulloch
    label: "Pixullochia Entrance"
}

place_pixullochia = (region_set) ->
    {:map, :regions} = region_set
    entrance = PIXULLOCHIA_ENTRANCE\linker()
    sanctum = require("map_descs.PixullochiaDepths")\linker()
    for i=1,3
        entrance\link_linker(sanctum, "spr_gates.enter", "spr_gates.return")

    return place_vault_in region_set, Vaults.hell_dungeon {
        tileset: Tilesets.pixulloch
        enemy_placer: (map, xy) ->
            enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
            MapUtils.spawn_enemy(map, enemy, xy)
        door_placer: (map, xy) -> MapUtils.spawn_lanarts_door(map, xy)
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.zig_portal")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
    }

-----------------------------
-- Gragh's lair            --
place_graghs_lair = (region_set) ->
    {:map, :regions} = region_set
    lair = require("map_descs.GraghsLair")\linker()

    return place_vault_in region_set, Vaults.graghs_lair_entrance {
        tileset: Tilesets.lair
        dungeon_placer: (map, xy) ->
            -- Make portal
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_slime")
            lair\link_portal(portal, 'spr_gates.exit_dungeon')
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
        enemy_placer: (map, xy) ->
            enemy = OldMaps.enemy_generate({{enemy: "Sheep", chance: 100}})
            MapUtils.spawn_enemy(map, enemy, xy)
    }
-----------------------------

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

    ---------------------------
    ---- Place hard dungeon: --
    --place_hard = () ->
    --    tileset = Tilesets.pixulloch
    --    dungeon = {label: 'Pixullochia', :tileset, templates: OldMaps.Dungeon4, spawn_portal: safe_portal_spawner(tileset)}
    --    door_placer = (map, xy) ->
    --        -- nil is passed for the default open sprite
    --        MapUtils.spawn_lanarts_door(map, xy)
    --    enemy_placer = (map, xy) ->
    --        enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
    --        MapUtils.spawn_enemy(map, enemy, xy)
    --    place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq4, dungeon)
    --    vault = SourceMap.area_template_create(Vaults.skull_surrounded_dungeon {dungeon_placer: place_dungeon, :enemy_placer, :door_placer, :tileset, player_spawn_area: false})
    --    if not place_feature(map, vault, regions)
    --        return true
    --if place_hard()
    --    print "RETRY: place_hard()"
    --    return nil
    ---------------------------
    if not place_pixullochia(region_set)
        return false

    append map.post_maps, () ->
        overdungeon_items_and_enemies(region_set)

    return true

overworld_features = (region_set) ->
    {:map, :regions} = region_set
    OldMapSeq3 = MapSequence.create {preallocate: 1}

    append map.post_game_map, (game_map) ->
        OldMapSeq3\slot_resolve(1, game_map)

    -------------------------
    -- Place ridges: --
    place_outdoor_ridges = () ->
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy)
        item_placer = (map, xy) ->
            item = ItemUtils.item_generate ItemGroups.basic_items
            MapUtils.spawn_item(map, item.type, item.amount, xy)
        vault = SourceMap.area_template_create(Vaults.ridge_dungeon {dungeon_placer: item_placer, :door_placer, tileset: Tilesets.pebble})
        if not place_feature(map, vault, regions)
            return false -- Dont reject -- true
    for i=1,4
        if place_outdoor_ridges()
            print "RETRY: place_outdoor_ridges()"
            return nil
    -------------------------

    if not map.rng\random_choice({place_snake_pit, place_temple})(region_set)
        return nil
    -----------------------------

    -------------------------------
    -- Place hive: --
    if not place_hive(region_set)
        print "RETRY: place_hive()"
        return nil
    -------------------------------

    -------------------------------
    -- Place underdungeon: --
    if not place_underdungeon_vault(region_set)
        print "RETRY: place_hive()"
        return nil
    -------------------------------

    -------------------------
    -- Place small_random_vaults:  --
    place_small_vaults = () ->
        for i=1,map.rng\random(2,3)
            enemy_placer = (map, xy) ->
                enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
                MapUtils.spawn_enemy(map, enemy, xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy, nil, xy)
            store_placer = (map, xy) ->
                Region1.generate_store(map, xy)
            item_placer = (map, xy) ->
                if map.rng.chance(.1)
                    MapUtils.spawn_item(map, "Scroll of Experience", 1, xy)
                else
                    item = ItemUtils.item_generate ItemGroups.basic_items
                    MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            tileset = Tilesets.snake
            vault = SourceMap.area_template_create(Vaults.small_random_vault {rng: map.rng, item_placer, :enemy_placer, :gold_placer, :store_placer, :tileset, :door_placer, :tileset})
            if not place_feature(map, vault)
                return false -- Dont reject
    if place_small_vaults()
        print "RETRY: place_small_vaults()"
        return nil
    -------------------------

    ---------------------------------
    -- Place big vaults            --
    place_big_vaults = () ->
        for template in *{Vaults.big_encounter1}--, Vaults.big_encounter2}
            enemy_placer = (map, xy) ->
                enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
                MapUtils.spawn_enemy(map, enemy, xy)
            item_placer = (map, xy) ->
                item = ItemUtils.item_generate ItemGroups.basic_items
                MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                if map.rng\chance(.7)
                    MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
            vault = SourceMap.area_template_create(template {:enemy_placer, :item_placer, :gold_placer, :door_placer})
            if not place_feature(map, vault)
                return true
    -- if place_big_vaults() then return nil
    ---------------------------------

    ---------------------------------
    -- Place centaur challenge     --
    place_centaur_challenge = () ->
        enemy_placer = (map, xy) ->
            enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
            MapUtils.spawn_enemy(map, enemy, xy)
        boss_placer = (map, xy) ->
            --if map.rng\randomf() < .5
            --    enemy = OldMaps.enemy_generate(OldMaps.strong_hell)
            --    MapUtils.spawn_enemy(map, enemy, xy)
            --else
            MapUtils.spawn_enemy(map, "Centaur Hunter", xy)
        n_items_placed = 0
        item_placer = (map, xy) ->
            item = ItemUtils.item_generate ItemGroups.basic_items, false, 1, (if n_items_placed == 0 then 100 else 2)
            MapUtils.spawn_item(map, item.type, item.amount, xy)
            n_items_placed += 1
        gold_placer = (map, xy) ->
            if map.rng\chance(.7)
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
        vault = SourceMap.area_template_create(Vaults.anvil_encounter {:enemy_placer, :boss_placer, :item_placer, :gold_placer, :door_placer})
        if not place_feature(map, vault, regions)
            return true
    -- if place_centaur_challenge() then return nil
    ---------------------------------

    ---------------------------------
    place_mini_features = () ->
        i = 0
        for template in *{Vaults.cavern, Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge}--, Vaults.stone_henge, Vaults.stone_henge}
            i += 1
        --for template in *{Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge}
            enemy_placer = (map, xy) ->
                enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
                MapUtils.spawn_enemy(map, enemy, xy)
            store_placer = (map, xy) ->
                Region1.generate_store(map, xy)
            if i ~= 1 and i ~= 4
                store_placer = do_nothing
            -- TODO evaluate if I want the 4 stores:
            if i == 4
                store_placer = (map, xy) ->
                    MapUtils.spawn_item(map, "Gold", random(2,10), xy)

            item_placer = (map, xy) ->
                item = ItemUtils.item_generate ItemGroups.basic_items
                MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy)
            vault = SourceMap.area_template_create(template {:enemy_placer, :store_placer, :item_placer, :gold_placer, :door_placer})
            if not place_feature(map, vault, regions)
                -- Dont reject
                continue
                -- return true
    if place_mini_features()
        print "RETRY: place_mini_features()"
        return nil
    ---------------------------------

    append map.post_maps, () ->
        overworld_items_and_enemies(region_set)

    return true

local generate_map_node

--initial_overworld_conf = (rng) -> {
--    is_overworld: true
--    size: rng\random_choice {{65, 45}, {45, 65}}
--    number_regions: rng\random(5, 7)
--    floor1: OVERWORLD_TILESET.floor1
--    floor2: OVERWORLD_TILESET.floor2
--    wall1: OVERWORLD_TILESET.wall1
--    wall2: OVERWORLD_TILESET.wall2
--    rect_room_num_range: {0,0} -- disable
--    rect_room_size_range: {10,15}
--    rvo_iterations: 100
--    connect_line_width: () -> rng\random(2,6)
--    region_delta_func: spread_region_delta_func
--    arc_chance: 0.05
--    room_radius: () -> rng\random(5,10)
--    -- Dungeon objects/features
--    n_statues: 4
--}

    --template_f = (rng) ->
    --    event_log("(RNG #%d) Attempting overworld generation", rng\amount_generated())
    --    conf or= initial_overworld_conf(rng) -- OVERWORLD_CONF(rng)
    --    return {
    --        :rng
    --        subtemplates: {conf}
    --        seethrough: true
    --        outer_conf: conf
    --        shell: 50
    --    }

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

grassy_overworld = (rng) ->
    -- CONFIG
    arc_chance = 0.05
    size = rng\random_choice {{65, 45}, {45, 65}}
    number_regions = rng\random(5, 7)
    connect_line_width = () -> rng\random(2, 6)
    default_wall = Tile.create(Tilesets.grass.wall, true, true, {FLAG_OVERWORLD})
    room_radius = () -> rng\random(5,10)

    return generate_map_node () ->
        map = NewMaps.source_map_create {
            :rng
            size: {OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE}
            default_content: default_wall.id
            default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
            map_label: "Plain Valley"
            :arc_chance
        }
        template = nilprotect {
            :default_wall
            subtemplates: {nilprotect {
                is_overworld: true
                :size
                :number_regions
                floor1: OVERWORLD_TILESET.floor1
                floor2: OVERWORLD_TILESET.floor2
                wall1: OVERWORLD_TILESET.wall1
                wall2: OVERWORLD_TILESET.wall2
                rect_room_num_range: {0,0} -- disable
                rect_room_size_range: {10,15}
                rvo_iterations: 100
                :connect_line_width
                :room_radius
                region_delta_func: spread_region_delta_func
                -- Dungeon objects/features
                n_statues: 4
            }}
            outer_conf: nilprotect {
                floor1: OVERWORLD_TILESET.floor1
                floor2: OVERWORLD_TILESET.floor2
                :connect_line_width
            }
            shell: 25
        }
        if not NewMaps.map_try_create(map, rng, template)
            return nil
        full_region_set = {:map, regions: map.regions}
        overworld_region = {:map, regions: table.filter(map.regions, (r) -> r.conf.is_overworld)}
        overdungeon_region = {:map, regions: table.filter(map.regions, (r) -> not r.conf.is_overworld)}
        append map.post_maps, () ->
            place_doors_and_statues(full_region_set)
        if not overworld_features(overworld_region)
            return nil
        return map

MAX_GENERATE_ITERS = 1000
generate_map_node = (create_map) -> NewMaps.try_n_times MAX_GENERATE_ITERS, () ->
    map = create_map()
    if not map
        return nil

    -- Reject levels that are not fully connected:
    if not NewMaps.check_connection(map)
        print("ABORT: connection check failed")
        return nil

    NewMaps.generate_door_candidates(map, map.rng, map.regions)

    for f in *map.post_maps
        if f() == 'reject' -- TODO consistently use this pattern?
            return nil
    return {
        :map
        static_area: MapRegion.create({})
        editable_area: MapRegion.create({
            rectangle_points(0, 0, map.size[1], map.size[2])
        })
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

overworld_create = () ->
    {:map} = grassy_overworld NewMaps.new_rng()
    --{x, y} = MapUtils.random_square(map, nil, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
    --append map.player_candidate_squares, {x*32+16,y*32+16}
    player_spawn_points = MapUtils.pick_player_squares(map, map.player_candidate_squares)
    assert player_spawn_points, "Could not pick player spawn squares!"

    game_map = NewMaps.generate_game_map(map)
    for f in *map.post_game_map
        f(game_map)
    World.players_spawn(game_map, player_spawn_points)
    Map.set_vision_radius(game_map, OVERWORLD_VISION_RADIUS)
    return game_map

return nilprotect {
    :overworld_create
    :place_feature
    :place_vault, :place_vault_in
    test_determinism: () -> nil
    :generate_map_node
    :TEMPLE_ENTRANCE, :TEMPLE_CHAMBER, :TEMPLE_SANCTUM
    :OGRE_LAIR
    :OUTPOST_ENTRANCE, :OUTPOST_STOCKROOM
    :underdungeon_create
    :DUNGEON_CONF
    :PIXULLOCHIA_ENTRANCE
}
