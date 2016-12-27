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
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

TileSets = require "tiles.Tilesets"
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

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE, 
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

OVERWORLD_VISION_RADIUS = 8

create_overworld_scheme = (tileset) -> {
    floor1: Tile.create(tileset.floor, false, true, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {FLAG_OVERWORLD}) 
    wall1: Tile.create(tileset.wall, true, true, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false)
}

create_dungeon_scheme = (tileset) -> {
    floor1: Tile.create(tileset.floor, false, true, {}, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {}, {FLAG_OVERWORLD}) 
    wall1: Tile.create(tileset.wall, true, false, {}, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false, {}, {FLAG_OVERWORLD})
}

OVERWORLD_TILESET = create_overworld_scheme(TileSets.grass)

OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE = 300, 300
SHELL = 50

SMALL_OVERWORLD_CONF = (rng) -> {
    map_label: "Plain Valley"
    is_small_overworld: true
    size: {30, 30}
    number_regions: 1
    floor1: OVERWORLD_TILESET.floor2 
    floor2: OVERWORLD_TILESET.floor1
    wall1: OVERWORLD_TILESET.wall1
    wall2: OVERWORLD_TILESET.wall2
    rect_room_num_range: {0,0}
    rect_room_size_range: {10,15}
    rvo_iterations: 150
    n_stairs_down: 0
    n_stairs_up: 0
    connect_line_width: () -> rng\random(2,6)
    region_delta_func: ring_region_delta_func
    room_radius: () ->
        return 30
    -- Dungeon objects/features
    monster_weights: () -> {["Giant Rat"]: 0, ["Chicken"]: 0, ["Cloud Elemental"]: 1, ["Turtle"]: 8, ["Spriggan"]: 2}
    n_statues: 4
}
OVERWORLD_CONF = (rng) -> {
    map_label: "Plain Valley"
    is_overworld: true
    size: if rng\random(0,2) == 0 then {85, 105} else {85, 105} 
    number_regions: rng\random(25,30)
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
    region_delta_func: ring_region_delta_func
    room_radius: () ->
        r = 7
        bound = rng\random(1,20)
        for j=1,rng\random(0,bound) do r += rng\randomf(0, 1)
        return r
    -- Dungeon objects/features
    monster_weights: () -> {["Giant Rat"]: 0, ["Chicken"]: 0, ["Cloud Elemental"]: 1, ["Turtle"]: 8, ["Spriggan"]: 2}
    n_statues: 4
}

DUNGEON_CONF = (rng) -> 
    -- Brown layout or blue layout?
    tileset = TileSets.pebble
    C = create_dungeon_scheme(tileset)
    -- Rectangle-heavy or polygon-heavy?
    switch 1 -- rng\random(3)
        when 0
            -- Few, big, rooms?
            C.number_regions = rng\random(5,10)
            C.room_radius = () ->
                r = 4
                for j=1,rng\random(0,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {2,3}
            C.rect_room_size_range = {10,15}
        when 1
            -- Mix?
            C.number_regions = rng\random(15,20)
            C.room_radius = () ->
                r = 2
                for j=1,rng\random(0,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {2,10}
            C.rect_room_size_range = {7,15}
        when 2
            -- Mostly rectangular rooms?
            C.number_regions = rng\random(2,7)
            C.room_radius = () ->
                r = 2
                for j=1,rng\random(0,10) do r += rng\randomf(0, 1)
                return r
            C.rect_room_num_range = {10,15}
            C.rect_room_size_range = {7,15}

    return table.merge C, {
        map_label: "A Dungeon"
        size: {65,65} 
        rvo_iterations: 20
        n_stairs_down: 3
        n_stairs_up: 0
        connect_line_width: () -> 2 + (if rng\random(5) == 4 then 1 else 0)
        region_delta_func: default_region_delta_func
        -- Dungeon objects/features
        monster_weights: () -> {["Giant Rat"]: 8, ["Cloud Elemental"]: 1, ["Chicken"]: 1}
        n_statues: 4
    }


make_rooms_with_tunnels = (map, rng, conf, area) ->
    oper = SourceMap.random_placement_operator {
        size_range: conf.rect_room_size_range
        rng: rng, :area
        amount_of_placements_range: conf.rect_room_num_range
        create_subgroup: false
        child_operator: (map, subgroup, bounds) ->
            --Purposefully convoluted for test purposes
            queryfn = () ->
                query = make_rectangle_criteria()
                return query(map, subgroup, bounds)
            oper = make_rectangle_oper(conf.floor2.id, conf.wall2.id, conf.wall2.seethrough, queryfn)
            if oper(map, subgroup, bounds)
                append map.rectangle_rooms, bounds
                --place_instances(rng, map, bounds)
                return true
            return false
    }
 
    oper map, SourceMap.ROOT_GROUP, area 
    tunnel_oper = make_tunnel_oper(rng, conf.floor1.id, conf.wall1.id, conf.wall1.seethrough)

    tunnel_oper map, SourceMap.ROOT_GROUP, area--{1,1, map.size[1]-1,map.size[2]-1}
    return map

connect_edges = (map, rng, conf, area, edges) ->
    for {p1, p2} in *edges
        tile = conf.floor1
        flags = {}
        rad1,rad2 = math.max(p1.w, p1.h)/2, math.max(p2.w, p2.h)/2
        line_width = conf.connect_line_width()
        if line_width <= 2 and p1\ortho_dist(p2) > (rng\random(3,6)+rad1+rad2)
            append flags, SourceMap.FLAG_TUNNEL
        if p2.id%5 <= 3 
            tile = conf.floor2
            append flags, FLAG_ALTERNATE
        fapply = nil 
        if rng\random(4) < 2 
            fapply = p1.line_connect 
        else 
            fapply = p1.arc_connect
        fapply p1, {
            :map, :area, target: p2, :line_width
            operator: (tile_operator tile, {matches_none: FLAG_ALTERNATE, matches_all: SourceMap.FLAG_SOLID, add: flags})
        }

make_rect_points = (x1,y1,x2,y2) ->
    return {{x1, y2}, {x2, y2}, {x2, y1}, {x1, y1}}

generate_area = (map, rng, conf, outer, padding, starting_edges = {}) ->
    size = conf.size
    R = RVORegionPlacer.create {outer.points}-- {make_rect_points outer.x, outer.y, outer.x+outer.w,outer.x}

    for i=1,conf.number_regions
        -- Make radius of the circle:
        r, n_points, angle = conf.room_radius(),rng\random(3,10) ,rng\randomf(0, math.pi)
        {x1, y1, x2, y2} = outer\bbox()
        r = random_region_add rng, r*2,r*2, n_points, conf.region_delta_func(map, rng, outer), angle, R, {x1 + padding, y1 + padding, x2 - padding, y2 - padding}, true
        if r then outer\add(r)

    R\steps(conf.rvo_iterations)

    for region in *R.regions
        tile = (if rng\random(4) ~= 1 then conf.floor1 else conf.floor2)
        region\apply {
            map: map, area: outer\bbox(), operator: (tile_operator tile, {add: FLAG_ROOM})
        }

    -- Connect all the closest region pairs:
    edges = region_minimum_spanning_tree(R.regions)
    add_edge_if_unique = (p1,p2) ->
        for {op1, op2} in *edges
            if op1 == p1 and op2 == p2 or op2 == p1 and op1 == p2
                return
        append edges, {p1, p2}
    for {p1, p2} in *starting_edges 
        add_edge_if_unique(p1, p2)

    -- Append all < threshold in distance
    for i=1,#R.regions
        for j=i+1,#R.regions do if rng\random(0,3) == 1
            p1, p2 = R.regions[i], R.regions[j]
            dist = math.sqrt( (p2.x-p1.x)^2+(p2.y-p1.y)^2)
            if dist < rng\random(5,15)
                add_edge_if_unique p1, p2
    connect_edges map, rng, conf, outer\bbox(), edges

generate_subareas = (map, rng, regions, starting_edges = {}) ->
    conf = OVERWORLD_CONF(rng)
    -- Generate the polygonal rooms, connected with lines & arcs
    for region in *regions
        generate_area map, rng, region.conf, region, SHELL, starting_edges

    edges = subregion_minimum_spanning_tree(regions, () -> rng\random(12) + rng\random(12))
    connect_edges map, rng, conf, nil, edges

    -- Diagonal pairs are a bit ugly. We can see through them but not pass them. Just open them up.
    SourceMap.erode_diagonal_pairs {:map, :rng, selector: {matches_all: SourceMap.FLAG_SOLID}}

    -- Detect the perimeter, important for the winding-tunnel algorithm.
    SourceMap.perimeter_apply {:map,
        candidate_selector: {matches_all: SourceMap.FLAG_SOLID}, inner_selector: {matches_none: SourceMap.FLAG_SOLID}
        operator: {add: SourceMap.FLAG_PERIMETER}
    }

    for region in *regions
        SourceMap.perimeter_apply {:map,
            area: region\bbox()
            candidate_selector: {matches_all: SourceMap.FLAG_SOLID}, inner_selector: {matches_all: FLAG_ALTERNATE, matches_none: SourceMap.FLAG_SOLID}
            operator: tile_operator region.conf.wall2 
        }

        -- Generate the rectangular rooms, connected with winding tunnels
    for region in *regions
        make_rooms_with_tunnels map, rng, region.conf, region\bbox() 

generate_door_candidates = (map, rng, regions) ->
    SourceMap.perimeter_apply {:map
        candidate_selector: {matches_none: {SourceMap.FLAG_SOLID}}, 
        inner_selector: {matches_all: {SourceMap.FLAG_PERIMETER, SourceMap.FLAG_SOLID}}
        operator: {add: FLAG_INNER_PERIMETER}
    }
    for region in *regions
        if region.conf.is_overworld or region.conf.is_small_overworld
            for subregion in *region.subregions
                region\apply {:map
                    operator: {remove: SourceMap.FLAG_TUNNEL}
                }
    -- Make sure doors dont get created in the overworld components:
    --SourceMap.rectangle_apply {:map, fill_operator: {matches_all: FLAG_OVERWORLD, remove: SourceMap.FLAG_TUNNEL}}
    SourceMap.perimeter_apply {:map,
        candidate_selector: {matches_all: {SourceMap.FLAG_TUNNEL}, matches_none: {FLAG_ROOM, SourceMap.FLAG_SOLID}}, 
        inner_selector: {matches_all: {FLAG_ROOM}, matches_none: {FLAG_DOOR_CANDIDATE, SourceMap.FLAG_SOLID}}
        operator: {add: FLAG_DOOR_CANDIDATE}
    }

    filter_door_candidates = (x1,y1,x2,y2) ->
        SourceMap.rectangle_apply {:map
            fill_operator: {remove: FLAG_DOOR_CANDIDATE}, area: {x1, y1, x2, y2}
        }
    filter_random_third = (x1,y1,x2,y2) ->
        w,h = (x2 - x1), (y2 - y1)
        if rng\random(0,2) == 0 
            filter_door_candidates(x2 + w/3, y1-1, x2+1, y2+1)
        if rng\random(0,2) == 0 
            filter_door_candidates(x1-1, y1-1, x1 + w/3, y2+1)
        if rng\random(0,2) == 0 
            filter_door_candidates(x1-1, y1+h/3, x2+1, y2+1)
        if rng\random(0,2) == 0 
            filter_door_candidates(x1-1, y1-1, x2+1, y2 - h/3)
    for region in *regions
        -- Unbox the region:
        for {:x,:y,:w,:h} in *region.subregions
            filter_random_third(x,y,x+w,y+h)
    for {x1,y1,x2,y2} in *map.rectangle_rooms
        -- Account for there already being a perimeter -- don't want to remove tunnels too far, get weird artifacts.
        filter_random_third(x1+1,y1+1,x2-1,y2-1)

generate_game_map = (map, place_object, place_monsters) ->
    M = Map.create {
        map: map
        label: assert(map.map_label)
        instances: map.instances
        wandering_enabled: map.wandering_enabled
    }
    return M

-- Returns a post-creation callback to be called on game_map

overworld_spawns = (map) ->
    gen_feature = (sprite, solid, seethrough = true) -> (px, py) -> 
        Feature.create M, {x: px*32+16, y: py*32+16, :sprite, :solid, :seethrough}

    for region in *map.regions
        area = region\bbox()
        conf = region.conf
        for xy in *SourceMap.rectangle_match {:map, selector: {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}, matches_all: {FLAG_DOOR_CANDIDATE}}}
            MapUtils.spawn_door(map, xy)
        for i=1,conf.n_statues
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
            MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0,17))
        for i=1,10 do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
            MapUtils.spawn_item(map, item.type, item.amount, sqr) 
        -- for i=1,conf.n_shops
        --     sqr = MapUtils.random_square(map, area, {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
        --     if not sqr
        --         break
        --     Region1.generate_store(map, sqr)
        if conf.is_overworld
            es1 = OldMaps.generate_from_enemy_entries(map, OldMaps.medium_animals, 8, area, {matches_none: {SourceMap.FLAG_SOLID, FLAG_NO_ENEMY_SPAWN}})
            es2 = OldMaps.generate_from_enemy_entries(map, OldMaps.medium_enemies, 5, area, {matches_none: {SourceMap.FLAG_SOLID, FLAG_NO_ENEMY_SPAWN}})
            es3 = OldMaps.generate_from_enemy_entries(map, OldMaps.fast_enemies, 5, area, {matches_none: {SourceMap.FLAG_SOLID, FLAG_NO_ENEMY_SPAWN}})
            -- Respawn overworld units:
            for es in *{es1, es2, es3}
                for enemy_obj in *es
                    enemy_obj.__table.respawns_on_death = true
        else
            OldMaps.generate_from_enemy_entries(map, OldMaps.hard_enemies, 10, area, {matches_none: {SourceMap.FLAG_SOLID, FLAG_NO_ENEMY_SPAWN}})
            OldMaps.generate_from_enemy_entries(map, OldMaps.fast_enemies, 10, area, {matches_none: {SourceMap.FLAG_SOLID, FLAG_NO_ENEMY_SPAWN}})

place_feature = (map, template, region_filter) ->
   -- Function to try a single placement, returns success:
   attempt_placement = (template) ->
       orient = map.rng\random_choice {
           SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y,
           SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
       }
       for r in *map.regions
           if not region_filter(r)
               continue
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
           if attempt_placement(template)
               return true
       return false
   -- Try to create the template object using our placement routines:
   if attempt_placement_n_times(template, 100)
       -- Exit, as we have handled the first overworld component
       return true
   return false

safe_portal_spawner = (tileset) -> (map, map_area, sprite, callback, frame) ->
    portal_holder = {}
    portal_placer = (map, xy) -> 
        portal_holder[1] = MapUtils.spawn_portal(map, xy, sprite, callback, frame)
    vault = SourceMap.area_template_create(Vaults.small_item_vault_multiway {rng: map.rng, item_placer: portal_placer, :tileset})
    -- TODO allow passing areas to use map_area, not critical for now
    if not place_feature(map, vault, (r) -> true)
        return MapUtils.random_portal(map, map_area, sprite, callback, frame)
    assert(portal_holder[1])
    return portal_holder[1]

overworld_features = (map) ->
    OldMapSeq1 = MapSequence.create {preallocate: 1}
    OldMapSeq1b = MapSequence.create {preallocate: 1}
    OldMapSeq2 = MapSequence.create {preallocate: 1}
    OldMapSeq3 = MapSequence.create {preallocate: 1}
    OldMapSeq4 = MapSequence.create {preallocate: 1}
    post_poned = {}

    ------------------------- 
    -- Place ridges: --
    place_outdoor_ridges = () ->
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy)
        item_placer = (map, xy) ->
            item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
            MapUtils.spawn_item(map, item.type, item.amount, xy)
        vault = SourceMap.area_template_create(Vaults.ridge_dungeon {dungeon_placer: item_placer, :door_placer, tileset: TileSets.pebble})
        if not place_feature(map, vault, (r) -> r.conf.is_overworld)
            return false -- Dont reject -- true
    for i=1,4
        if place_outdoor_ridges() then return nil
    ------------------------- 

    ------------------------- 
    -- Place easy dungeon: --
    place_easy = () ->
        inner_encounter_template = {
           {
                layout: {{size: {60,40}, rooms: {padding:0,amount:40,size:{3,7}},tunnels:{padding:0, width:{1,3},per_room: 5}}}
                content: {
                    items: {amount: 12, group: ItemGroups.basic_items} 
                    enemies: {
                        wandering: true
                        amount: 0
                        generated: {
                          {enemy: "Ogre Mage",         guaranteed_spawns: 3}
                          {enemy: "Orc Warrior",       guaranteed_spawns: 3}
                          {enemy: "Adder",             guaranteed_spawns: 5}
                        }
                    }
                }
            }
        }
        templates = OldMaps.Dungeon1
        -------------------------------------------------
        -- Inner dungeon generation for dungeon 1      --
        on_generate_dungeon = (map, floor) ->
            if floor == #templates
                ------------------------- 
                -- Place key vault     --
                for i=1,2 
                    item_placer = (map, xy) ->
                        item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
                        MapUtils.spawn_item(map, item.type, item.amount, xy)
                    tileset = TileSets.pebble

                    vault = SourceMap.area_template_create(Vaults.small_item_vault {rng: map.rng, :item_placer, :tileset})
                    if not place_feature(map, vault, (r) -> true)
                        return nil

                area = {0,0,map.size[1],map.size[2]}
                for i=1,4
                    sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
                    if not sqr
                        return nil
                    Region1.generate_store(map, sqr)

                sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
                if not sqr
                    return nil
                MapUtils.spawn_enemy(map, "Red Dragon", sqr)
                ------------------------- 

            else if floor == 1
                ------------------------- 
                -- Place inner dungeon vault     --
                on_generate_dungeon = (map, floor) ->
                    ---------------------------------------------------------- 
                    for item in *{"Scroll of Experience", "Scroll of Experience", "Scroll of Experience"} 
                        item_placer = (map, xy) -> MapUtils.spawn_item(map, item, 1, xy)
                        tileset = TileSets.snake
                        vault = SourceMap.area_template_create(Vaults.small_item_vault {rng: map.rng, :item_placer, :tileset})
                        if not place_feature(map, vault, (r) -> true)
                            return nil
                    ---------------------------------------------------------- 
                    return true
                on_generate_dungeon = nil
                spawn_portal = safe_portal_spawner(TileSets.snake)
                dungeon = {label: 'Ogre Lair', tileset: TileSets.snake, templates: inner_encounter_template, on_generate: on_generate_dungeon, :spawn_portal, sprite_out: Region1.stair_kinds_index(5, 7)}
                gold_placer = (map, xy) ->
                    MapUtils.spawn_item(map, "Gold", random(2,10), xy)
                door_placer = (map, xy) ->
                    -- nil is passed for the default open sprite
                    MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
                place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq1b, dungeon)
                vault = SourceMap.area_template_create(Vaults.sealed_dungeon {dungeon_placer: place_dungeon, tileset: TileSets.snake, :door_placer, :gold_placer})
                if not place_feature(map, vault, (r) -> true)
                    return nil
                --vault = SourceMap.area_template_create(Vaults.dungeon_tunnel {tileset: TileSets.snake, :door_placer, :gold_placer})
                --if not place_feature(map, vault, (r) -> true)
                --    return nil
                ------------------------- 
            ---- Ensure connectivity because we messed with dungeon features:
            --if not SourceMap.area_fully_connected {
            --    :map, 
            --    unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
            --    mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
            --    marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
            --}
            --    return nil
            return true
        -------------------------------------------------
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy)
        on_placement = (map) -> OldMapSeq1b\slot_resolve(1, map)
        dungeon = {label: 'Hideaway', tileset: TileSets.pebble, :templates, on_generate: on_generate_dungeon, :on_placement}
        place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq1, dungeon)
        vault = SourceMap.area_template_create(Vaults.ridge_dungeon {dungeon_placer: place_dungeon, :door_placer, tileset: TileSets.pebble})
        if not place_feature(map, vault, (r) -> r.conf.is_overworld)
            return true
    if place_easy() then return nil
    ------------------------- 

    -----------------------------
    -- Place player spawn area: --
    --place_player_spawn_area = () ->
    --    door_placer = (map, xy) ->
    --        -- nil is passed for the default open sprite
    --        MapUtils.spawn_door(map, xy)
    --    vault = SourceMap.area_template_create(Vaults.ridge_dungeon {dungeon_placer: do_nothing, :door_placer, player_spawn_area: true, tileset: TileSets.pebble}, door_match_content: OVERWORLD_TILESET.floor2)
    --    if not place_feature(map, vault, (r) -> r.conf.is_overworld)
    --        return true
    --if place_player_spawn_area() then return nil
    -------------------------------

    -----------------------------
    -- Place medium dungeon 1: --
    place_medium1 = () ->
        local templates 
        templates = OldMaps.Dungeon2
        on_generate_dungeon = (map, floor) ->
            if floor == #templates
                ---------------------------------------------------------- 
                -- Place key vault, along with some gold vaults --
                for item in *{"Azurite Key", "Gold", "Gold"} 
                    item_placer = (map, xy) -> MapUtils.spawn_item(map, item, (if item == "Gold" then 10 else 1), xy)
                    tileset = TileSets.snake
                    vault = SourceMap.area_template_create(Vaults.small_item_vault {rng: map.rng, :item_placer, :tileset})
                    if not place_feature(map, vault, (r) -> true)
                        return nil
                ---------------------------------------------------------- 
            return true
        dungeon = {label: 'Temple', tileset: TileSets.temple, :templates, on_generate: on_generate_dungeon}
        gold_placer = (map, xy) ->
            -- Dont need gold here
            -- MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            return nil
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._closed_door_crypt)
        place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq2, dungeon)
        vault = SourceMap.area_template_create(Vaults.sealed_dungeon {dungeon_placer: place_dungeon, tileset: TileSets.temple, :door_placer, :gold_placer, player_spawn_area: true})
        if not place_feature(map, vault, (r) -> r.conf.is_overworld)
            return true
    if place_medium1() then return nil
    -----------------------------

    -----------------------------
    -- Place medium dungeon 2: --
    place_medium2 = () ->
        templates = OldMaps.Dungeon3
        on_generate_dungeon = (map, floor) ->
            if floor == #templates
                ------------------------- 
                -- Place key vault     --
                item_placer = (map, xy) -> MapUtils.spawn_item(map, "Dandelite Key", 1, xy)
                tileset = TileSets.snake
                vault = SourceMap.area_template_create(Vaults.small_item_vault {rng: map.rng, :item_placer, :tileset})
                if not place_feature(map, vault, (r) -> true)
                    return nil
                ------------------------- 
            return true
        gold_placer = (map, xy) ->
            MapUtils.spawn_item(map, "Gold", random(2,10), xy)
        dungeon = {label: "Gragh's Lair", tileset: TileSets.snake, :templates, on_generate: on_generate_dungeon}
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
        place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq3, dungeon)
        vault = SourceMap.area_template_create(Vaults.sealed_dungeon {dungeon_placer: place_dungeon, tileset: TileSets.snake, :door_placer, :gold_placer})
        if not place_feature(map, vault, (r) -> not r.conf.is_overworld)
            return true
    if place_medium2() then return nil
    -----------------------------

    ------------------------- 
    -- Place hard dungeon: --
    place_hard = () ->
        tileset = TileSets.snake
        dungeon = {label: 'Zin\'s Palace', :tileset, templates: OldMaps.Dungeon4, spawn_portal: safe_portal_spawner(tileset)}
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
        enemy_placer = (map, xy) ->
            enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
            MapUtils.spawn_enemy(map, enemy, xy)
        place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq4, dungeon)
        vault = SourceMap.area_template_create(Vaults.skull_surrounded_dungeon {dungeon_placer: place_dungeon, :enemy_placer, :door_placer, :tileset})
        if not place_feature(map, vault, (r) -> not r.conf.is_overworld)
            return true
    if place_hard() then return nil
    ------------------------- 

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
                    item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
                    MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            tileset = TileSets.snake
            vault = SourceMap.area_template_create(Vaults.small_random_vault {rng: map.rng, item_placer, :enemy_placer, :gold_placer, :store_placer, :tileset, :door_placer, :tileset})
            if not place_feature(map, vault, (r) -> true)
                return false -- Dont reject
    if place_small_vaults() then return nil
    ------------------------- 

    ---------------------------------
    -- Place big vaults            --
    place_big_vaults = () ->
        for template in *{Vaults.big_encounter1}--, Vaults.big_encounter2}
            enemy_placer = (map, xy) ->
                enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
                MapUtils.spawn_enemy(map, enemy, xy)
            item_placer = (map, xy) ->
                item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
                MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                if map.rng\chance(.7) 
                    MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
            vault = SourceMap.area_template_create(template {:enemy_placer, :item_placer, :gold_placer, :door_placer})
            if not place_feature(map, vault, (r) -> true)
                return true
    --if place_big_vaults() then return nil
    ---------------------------------

    ---------------------------------
    -- Place centaur challenge     --
    place_centaur_challenge = () ->
        enemy_placer = (map, xy) ->
            enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
            MapUtils.spawn_enemy(map, enemy, xy)
        boss_placer = (map, xy) ->
            MapUtils.spawn_enemy(map, "Dark Centaur", xy)
        item_placer = (map, xy) ->
            item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
            MapUtils.spawn_item(map, item.type, item.amount, xy)
        gold_placer = (map, xy) ->
            if map.rng\chance(.7) 
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
        vault = SourceMap.area_template_create(Vaults.anvil_encounter {:enemy_placer, :boss_placer, :item_placer, :gold_placer, :door_placer})
        if not place_feature(map, vault, (r) -> true)
            return true
    if place_centaur_challenge() then return nil
    ---------------------------------

    ---------------------------------
    place_tunnel = () ->
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
                item = ItemUtils.item_generate ItemGroups.basic_items, 1 --Randart power level
                MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy)
            vault = SourceMap.area_template_create(template {:enemy_placer, :store_placer, :item_placer, :gold_placer, :door_placer})
            if not place_feature(map, vault, (r) -> r.conf.is_overworld)
                -- Dont reject
                continue
                -- return true
    if place_tunnel() then return nil
    ---------------------------------

    -- Return the post-creation callback:
    return (game_map) ->
        OldMapSeq1\slot_resolve(1, game_map)
        OldMapSeq2\slot_resolve(1, game_map)
        OldMapSeq3\slot_resolve(1, game_map)
        OldMapSeq4\slot_resolve(1, game_map)
        for f in *post_poned
            f(game_map)

test_determinism = () ->
    do return
    create_test_map = (rng) ->
        conf = OVERWORLD_CONF(rng)
        {PW,PH} = LEVEL_PADDING
        mw,mh = nil,nil
        if rng\random(0,2) == 1
            mw, mh = OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE
        else 
            mw, mh = OVERWORLD_DIM_MORE, OVERWORLD_DIM_LESS
        outer = Region.create(1+PW,1+PH,mw-PW,mh-PH)
        -- Generate regions in a large area, crop them later
        rect = {{1+PW, 1+PH}, {mw-PW, 1+PH}, {mw-PW, mh-PH}, {1+PW, mh-PH}}
        rect2 = {{1+PW, mh-PH}, {mw-PW, mh-PH}, {mw-PW, 1+PH}, {1+PW, 1+PH}}
        major_regions = RVORegionPlacer.create {rect2}
        map = SourceMap.map_create { 
            rng: rng
            size: {mw, mh}
            content: conf.wall1.id
            flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
            map_label: conf.map_label,
            instances: {}
            door_locations: {}
            rectangle_rooms: {}
            wandering_enabled: true
            -- For the overworld, created by dungeon features we add later:
            player_candidate_squares: {}
        }

        local overworld_region
        for subconf in *{DUNGEON_CONF(rng), OVERWORLD_CONF(rng)}--, SMALL_OVERWORLD_CONF(rng)}
            {w,h} = subconf.size
            -- Takes region parameters, region placer, and region outer ellipse bounds:
            r = random_region_add rng, w, h, 20, center_region_delta_func(map, rng, outer), 0,
                major_regions, outer\bbox()
            if subconf.is_overworld
                overworld_region = r
            --if r == nil
            --    return nil
            r.max_speed = 32
            r.conf = subconf
            delta_func = center_region_delta_func(map, rng, outer)
        --for i in *{1,3}
        --    major_regions.regions[i].velocity_func = towards_region_delta_func(map, rng, major_regions[2])
        --major_regions.regions[2].velocity_func = towards_region_delta_func(map, rng, major_regions[3])
     
        -- No rvo for now
        major_regions\steps(150)

        -- Apply the regions:
        for r in *major_regions.regions
            r._points = false
            r\apply {:map, operator: (tile_operator r.conf.wall1)}

        generate_subareas(map, rng, major_regions.regions)
        map.regions = major_regions.regions
        
        post_creation_callback = overworld_features(map)
        --if not post_creation_callback
        --    return nil
        generate_door_candidates(map, rng, major_regions.regions)
        overworld_spawns(map)

        -- Reject levels that are not fully connected:
        --if not SourceMap.area_fully_connected {
        --    :map, 
        --    unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
        --    mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
        --    marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
        --}
        --    return nil
        return map
    
    create_test_maps = (seed) ->
        rng = require("mtwist").create(seed)
        random_seed(seed)
        return for i=1,100 
            print "Creating map #{seed} : #{i}"
            create_test_map(rng) -- List in moonscript
    for i=1,10
        file = io.open("test_maps/map-set #{i}", "w")
        print "Comparing with seed #{i}"
        maps = create_test_maps(i)
        for j=1,100
            file\write(SourceMap.map_dump(maps[i]))
        file\close()
--        maps1, maps2 = create_test_maps(i), create_test_maps(i)
--        for j=1,100
--            print "Comparing with seed #{i} : #{j}"
--            assert SourceMap.maps_equal(maps1[j], maps2[j])
--
overworld_try_create = (rng) ->
    rng = rng or require("mtwist").create(random(0, 2 ^ 31))
    conf = OVERWORLD_CONF(rng)
    {PW,PH} = LEVEL_PADDING
    mw,mh = nil,nil
    if rng\random(0,2) == 1
        mw, mh = OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE
    else 
        mw, mh = OVERWORLD_DIM_MORE, OVERWORLD_DIM_LESS
    outer = Region.create(1+PW,1+PH,mw-PW,mh-PH)
    -- Generate regions in a large area, crop them later
    rect = {{1+PW, 1+PH}, {mw-PW, 1+PH}, {mw-PW, mh-PH}, {1+PW, mh-PH}}
    rect2 = {{1+PW, mh-PH}, {mw-PW, mh-PH}, {mw-PW, 1+PH}, {1+PW, 1+PH}}
    major_regions = RVORegionPlacer.create {rect2}
    map = SourceMap.map_create { 
        rng: rng
        size: {mw, mh}
        content: conf.wall1.id
        flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
        map_label: conf.map_label,
        instances: {}
        door_locations: {}
        rectangle_rooms: {}
        wandering_enabled: true
        -- For the overworld, created by dungeon features we add later:
        player_candidate_squares: {}
    }

    for subconf in *{DUNGEON_CONF(rng), OVERWORLD_CONF(rng)}--, SMALL_OVERWORLD_CONF(rng)}
        {w,h} = subconf.size
        -- Takes region parameters, region placer, and region outer ellipse bounds:
        r = random_region_add rng, w, h, 20, center_region_delta_func(map, rng, outer), 0,
            major_regions, outer\bbox()
        if r == nil
            return nil
        r.max_speed = 8
        r.conf = subconf
    --for i in *{1,3}
    --    major_regions.regions[i].velocity_func = towards_region_delta_func(map, rng, major_regions.regions[2])
    --major_regions.regions[2].velocity_func = towards_region_delta_func(map, rng, major_regions.regions[3])
    starting_edges = {}
    -- No rvo for now
    major_regions\steps(1500)

    -- Apply the regions:
    for r in *major_regions.regions
        r._points = false
        r\apply {:map, operator: (tile_operator r.conf.wall1)}

    generate_subareas(map, rng, major_regions.regions, starting_edges)
    map.regions = major_regions.regions
 
    post_creation_callback = overworld_features(map)
    if not post_creation_callback
        return nil
    generate_door_candidates(map, rng, major_regions.regions)
    overworld_spawns(map)

    -- Reject levels that are not fully connected:
    if not SourceMap.area_fully_connected {
        :map, 
        unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
        mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
        marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
    }
        return nil
    -- player_spawn_points = for i=1,2 do MapUtils.random_square(map, {0,0,map.size[1],map.size[2]}, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
    player_spawn_points = MapUtils.pick_player_squares(map, map.player_candidate_squares)
    if not player_spawn_points
        return nil

    game_map = generate_game_map(map)
    post_creation_callback(game_map)
    World.players_spawn(game_map, player_spawn_points)
    Map.set_vision_radius(game_map, OVERWORLD_VISION_RADIUS)
    return game_map

overworld_create = () ->
    for i=1,1000
        map = overworld_try_create()
        if map
            return map
        print "** MAP GENERATION ATTEMPT " .. i .. " FAILED, RETRYING **"
    error("Could not generate a viable overworld in 1000 tries!")

return {
    :overworld_create, :generate_game_map, :test_determinism
}
