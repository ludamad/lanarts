MapCompiler = require "maps.MapCompiler"
import Spread, Shape
    from require "maps.MapElements"

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
            local oper
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

generate_game_map = (map, place_object, place_monsters) ->
    M = Map.create {
        map: map
        label: assert(map.map_label)
        instances: map.instances
        wandering_enabled: map.wandering_enabled
    }
    return M

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
        --if rng\random(4) < 2 
         --   fapply = p1.line_connect 
        --else 
        if chance(map.arc_chance)
            fapply = p1.arc_connect
        else
            fapply = p1.line_connect
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

generate_subareas = (template, map, rng, regions, starting_edges = {}) ->
    conf = template.outer_conf
    -- Generate the polygonal rooms, connected with lines & arcs
    for region in *regions
        generate_area map, rng, region.conf, region, template.shell, starting_edges

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

map_try_create = (template_f) ->
    seed = random(0, 2 ^ 30)
    rng = require("mtwist").create(seed)
    event_log("(RNG #%d) Attempting map_try_create with seed %d\n", rng\amount_generated(), seed)
    template = template_f(rng)
    {PW,PH} = LEVEL_PADDING
    mw,mh = template.w, template.h
    outer = Region.create(1+PW,1+PH,mw-PW,mh-PH)
    -- Generate regions in a large area, crop them later
    rect = {{1+PW, 1+PH}, {mw-PW, 1+PH}, {mw-PW, mh-PH}, {1+PW, mh-PH}}
    rect2 = {{1+PW, mh-PH}, {mw-PW, mh-PH}, {mw-PW, 1+PH}, {1+PW, 1+PH}}
    major_regions = RVORegionPlacer.create {rect2}
    event_log("(RNG #%d) before SourceMap.map_create\n", rng\amount_generated())
    map = SourceMap.map_create { 
        rng: rng
        size: {mw, mh}
        content: template.default_wall.id
        flags: {SourceMap.FLAG_SOLID, if template.seethrough then SourceMap.FLAG_SEETHROUGH else 0}
        map_label: template.map_label,
        instances: {}
        arc_chance: template.arc_chance or 0
        door_locations: {}
        post_maps: {}
        rectangle_rooms: {}
        wandering_enabled: template.wandering_enabled
        -- For the overworld, created by dungeon features we add later:
        player_candidate_squares: {}
    }
    event_log("(RNG #%d) after SourceMap.map_create\n", rng\amount_generated())

    for subconf in *assert(template.subtemplates, "Must have subtemplates!")
        {w,h} = subconf.size
        -- Takes region parameters, region placer, and region outer ellipse bounds:
        assert(w and h, "Subconf must have size")
        r = random_region_add rng, w, h, 20, center_region_delta_func(map, rng, outer), 0,
            major_regions, outer\bbox()
        if r == nil
            print("ABORT: region could not be added")
            return nil
        {rx1,ry1,rx2,ry2} = r\bbox()
        event_log("(RNG #%d) created region at (%d,%d,%d,%d)\n", rng\amount_generated(), rx1, ry1, rx2, ry2)
        r.max_speed = 8
        r.conf = subconf
    starting_edges = {}
    -- No rvo for now
    major_regions\steps(1500)

    -- Apply the regions:
    for r in *major_regions.regions
        {rx1,ry1,rx2,ry2} = r\bbox()
        event_log("(RNG #%d) after RVO, have region at (%d,%d,%d,%d)\n", rng\amount_generated(), rx1, ry1, rx2, ry2)
        r._points = false
        r\apply {:map, operator: (tile_operator r.conf.wall1)}

    generate_subareas(template, map, rng, major_regions.regions, starting_edges)
    map.regions = major_regions.regions

    if not template\on_create_source_map(map)
        print("ABORT: on_create_source_map")
        return nil

    -- Reject levels that are not fully connected:
    if not SourceMap.area_fully_connected {
        :map, 
        unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
        mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
        marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
    }
        print("ABORT: connection check failed")
        return nil
    for _, map_gen_func in ipairs(map.post_maps)
        map_gen_func()

    game_map = generate_game_map(map)
    template\on_create_game_map(game_map)
    return game_map

map_create = (template) ->
    for i=1,1000
        map = map_try_create(template)
        if map
            return map
        print "** MAP GENERATION ATTEMPT " .. i .. " FAILED, RETRYING **"
    error("Could not generate a viable map in 1000 tries!")

return {:map_create, :generate_door_candidates, :connect_edges}
