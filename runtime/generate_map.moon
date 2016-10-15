----
-- Generates the game maps, starting with high-level details (places that will be in the game)
-- and then generating actual tiles.
----
import map_place_object, ellipse_points, 
    LEVEL_PADDING, Region, RVORegionPlacer, 
    random_rect_in_rect, random_ellipse_in_ellipse, 
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree
    Tile, tile_operator
        from require "@generate_util"

import MapUtils

import print_map, make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
    from require "@map_util"

import TileMap from require "core"

-- Generation constants and data
FLAG_ALTERNATE = TileMap.FLAG_CUSTOM1
FLAG_INNER_PERIMETER = TileMap.FLAG_CUSTOM2
FLAG_DOOR_CANDIDATE = TileMap.FLAG_CUSTOM3
FLAG_OVERWORLD = TileMap.FLAG_CUSTOM4
FLAG_ROOM = TileMap.FLAG_CUSTOM5

OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE = 150, 150
OVERWORLD_CONF = (rng) -> {
    map_label: "Plain Valley"
    size: {45,45}--if rng\random(0,2) == 0 then {135, 85} else {85, 135} 
    number_regions: rng\random(35,40)
    floor1: Tile.create('grass1', false, true)
    floor2: Tile.create('grass2', false, true) 
    wall1: Tile.create('tree', true, true, {FLAG_OVERWORLD})
    wall2: Tile.create('dungeon_wall', true, false)
    rect_room_num_range: {0,0}
    rect_room_size_range: {10,15}
    rvo_iterations: 150
    n_shops: rng\random(2,4)
    n_stairs_down: 0
    n_stairs_up: 0
    connect_line_width: () -> rng\random(2,6)
    region_delta_func: ring_region_delta_func
    room_radius: () ->
        r = 2
        bound = rng\random(1,10)
        for j=1,rng\random(0,bound) do r += rng\randomf(0, 1)
        return r
    -- Dungeon objects/features
    monster_weights: () -> {["Giant Rat"]: 0, ["Chicken"]: 0, ["Cloud Elemental"]: 1, ["Turtle"]: 8, ["Spriggan"]: 2}
    n_statues: 4
}

DUNGEON_CONF = (rng) -> 
    -- Brown layout or blue layout?
    C = {}
    switch rng\random(3)
        when 0
            C.floor1 = Tile.create('grey_floor', false, true)
            C.floor2 = Tile.create('reddish_grey_floor', false, true) 
            C.wall1 = Tile.create('dungeon_wall', true, false, {}, {FLAG_OVERWORLD})
            C.wall2 = Tile.create('crypt_wall', true, false, {}, {FLAG_OVERWORLD})
        when 1
            C.floor1 = Tile.create('crystal_floor1', false, true)
            C.floor2 = Tile.create('crystal_floor2', false, true) 
            C.wall1 = Tile.create('crystal_wall', true, false, {}, {FLAG_OVERWORLD})
            C.wall2 = Tile.create('crystal_wall2', true, false, {}, {FLAG_OVERWORLD})
        when 2
            C.floor1 = Tile.create('pebble_floor1', false, true)
            C.floor2 = Tile.create('pebble_floor2', false, true) 
            C.wall1 = Tile.create('pebble_wall1', true, false, {}, {FLAG_OVERWORLD})
            C.wall2 = Tile.create('pebble_wall3', true, false, {}, {FLAG_OVERWORLD})
    -- Rectangle-heavy or polygon-heavy?
    switch rng\random(3)
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
            C.number_regions = rng\random(5,20)
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
        size: {85, 85}--if rng\random(0,2) == 0 then {135, 85} else {85, 135} 
        rvo_iterations: 20
        n_shops: rng\random(2,4)
        n_stairs_down: 3
        n_stairs_up: 0
        connect_line_width: () -> 2 + (if rng\random(5) == 4 then 1 else 0)
        region_delta_func: default_region_delta_func
        -- Dungeon objects/features
        monster_weights: () -> {["Giant Rat"]: 8, ["Cloud Elemental"]: 1, ["Chicken"]: 1}
        n_statues: 4
    }


make_rooms_with_tunnels = (map, rng, conf, area) ->
    oper = TileMap.random_placement_operator {
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
 
    oper map, TileMap.ROOT_GROUP, area 
    tunnel_oper = make_tunnel_oper(rng, conf.floor1.id, conf.wall1.id, conf.wall1.seethrough)

    tunnel_oper map, TileMap.ROOT_GROUP, area--{1,1, map.size[1]-1,map.size[2]-1}
    return map

connect_edges = (map, rng, conf, area, edges) ->
    for {p1, p2} in *edges
        tile = conf.floor1
        flags = {}
        rad1,rad2 = math.max(p1.w, p1.h)/2, math.max(p2.w, p2.h)/2
        line_width = conf.connect_line_width()
        if line_width <= 2 and p1\ortho_dist(p2) > (rng\random(3,6)+rad1+rad2)
            append flags, TileMap.FLAG_TUNNEL
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
            operator: (tile_operator tile, {matches_none: FLAG_ALTERNATE, add: flags})
        }

make_rect_points = (x1,y1,x2,y2) ->
    return {{x1, y2}, {x2, y2}, {x2, y1}, {x1, y1}}

generate_area = (map, rng, conf, outer) ->
    size = conf.size
    R = RVORegionPlacer.create {outer.points}-- {make_rect_points outer.x, outer.y, outer.x+outer.w,outer.x}

    for i=1,conf.number_regions
        -- Make radius of the circle:
        r, n_points, angle = conf.room_radius(),rng\random(3,10) ,rng\randomf(0, math.pi)
        r = random_region_add rng, r*2,r*2, n_points, conf.region_delta_func(map, rng, outer), angle, R, outer\bbox(), true
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

    -- Append all < threshold in distance
    for i=1,#R.regions
        for j=i+1,#R.regions do if rng\random(0,3) == 1
            p1, p2 = R.regions[i], R.regions[j]
            dist = math.sqrt( (p2.x-p1.x)^2+(p2.y-p1.y)^2)
            if dist < rng\random(5,15)
                add_edge_if_unique p1, p2
    connect_edges map, rng, conf, outer\bbox(), edges

generate_subareas = (map, rng, regions) ->
    conf = OVERWORLD_CONF(rng)
    -- Generate the polygonal rooms, connected with lines & arcs
    for region in *regions
        generate_area map, rng, region.conf, region

    edges = subregion_minimum_spanning_tree(regions, () -> rng\random(12) + rng\random(12))
    connect_edges map, rng, conf, nil, edges

    -- Diagonal pairs are a bit ugly. We can see through them but not pass them. Just open them up.
    TileMap.erode_diagonal_pairs {:map, :rng, selector: {matches_all: TileMap.FLAG_SOLID}}

    -- Detect the perimeter, important for the winding-tunnel algorithm.
    TileMap.perimeter_apply {:map,
        candidate_selector: {matches_all: TileMap.FLAG_SOLID}, inner_selector: {matches_none: TileMap.FLAG_SOLID}
        operator: {add: TileMap.FLAG_PERIMETER}
    }

    for region in *regions
        TileMap.perimeter_apply {:map,
            area: region\bbox()
            candidate_selector: {matches_all: TileMap.FLAG_SOLID}, inner_selector: {matches_all: FLAG_ALTERNATE, matches_none: TileMap.FLAG_SOLID}
            operator: tile_operator region.conf.wall2 
        }

        -- Generate the rectangular rooms, connected with winding tunnels
    for region in *regions
        make_rooms_with_tunnels map, rng, region.conf, region\bbox() 

    TileMap.perimeter_apply {:map
        candidate_selector: {matches_none: {TileMap.FLAG_SOLID}}, 
        inner_selector: {matches_all: {TileMap.FLAG_PERIMETER, TileMap.FLAG_SOLID}}
        operator: {add: FLAG_INNER_PERIMETER}
    }
    for region in *regions
        for subregion in *region.subregions
            subregion\apply {:map
                operator: {remove: TileMap.FLAG_TUNNEL}
            }
    -- Make sure doors dont get created in the overworld components:
    TileMap.rectangle_apply {:map, fill_operator: {matches_all: FLAG_OVERWORLD, remove: TileMap.FLAG_TUNNEL}}
    TileMap.perimeter_apply {:map,
        candidate_selector: {matches_all: {TileMap.FLAG_TUNNEL}, matches_none: {FLAG_ROOM, TileMap.FLAG_SOLID}}, 
        inner_selector: {matches_all: {FLAG_ROOM}, matches_none: {FLAG_DOOR_CANDIDATE, TileMap.FLAG_SOLID}}
        operator: {add: FLAG_DOOR_CANDIDATE}
    }

    filter_door_candidates = (x1,y1,x2,y2) ->
        TileMap.rectangle_apply {:map
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

generate_overworld = (rng) ->
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
    map = TileMap.map_create { 
        size: {mw, mh}
        content: conf.wall1.id
        flags: conf.wall1.add_flags
        map_label: conf.map_label,
        door_locations: {}
        rectangle_rooms: {}
    }

    for subconf in *{DUNGEON_CONF(rng), OVERWORLD_CONF(rng), OVERWORLD_CONF(rng), OVERWORLD_CONF(rng), OVERWORLD_CONF(rng), DUNGEON_CONF(rng)}
        {w,h} = subconf.size
        -- Takes region parameters, region placer, and region outer ellipse bounds:
        r = random_region_add rng, w, h, 20, spread_region_delta_func(map, rng, outer), 0,
            major_regions, outer\bbox()
        if r ~= nil
            r.max_speed = 10
            r.conf = subconf
    -- No rvo for now

    for r in *major_regions.regions
        r._points = false
        r\apply {:map, operator: (tile_operator r.conf.wall1)}

    generate_subareas(map, rng, major_regions.regions)
    map.regions = major_regions.regions

    return map

generate_game_map = (G, map, place_object, place_monsters) ->
    import map_state from require "core"
    import Feature from require '@map_object_types'
    
    M = map_state.create_map_state(G, 1, G.rng, map.map_label, map)
    gen_feature = (sprite, solid, seethrough = true) -> (px, py) -> 
        Feature.create M, {x: px*32+16, y: py*32+16, :sprite, :solid, :seethrough}

    for region in *map.regions
        area = region\bbox()
        conf = region.conf
        for {x,y} in *TileMap.rectangle_match {:map, selector: {matches_none: {TileMap.FLAG_HAS_OBJECT, TileMap.FLAG_SOLID}, matches_all: {FLAG_DOOR_CANDIDATE}}}
            gen_feature('door_closed', true, false)(x, y)
        for i=1,conf.n_shops do place_object M, gen_feature('shops', false), area, {
            matches_none: {TileMap.FLAG_HAS_OBJECT, TileMap.FLAG_SOLID}
        }

        for i=1,conf.n_stairs_down do place_object M, gen_feature('stairs_down', false), area, {
            matches_none: {TileMap.FLAG_HAS_OBJECT, TileMap.FLAG_SOLID}
        }

        for i=1,conf.n_stairs_up do place_object M, gen_feature('stairs_up', false), area, {
            matches_none: {TileMap.FLAG_HAS_OBJECT, TileMap.FLAG_SOLID}
        }
        place_monsters M, conf.monster_weights(), area
    return M

return {
    :generate_overworld, :generate_game_map
}
