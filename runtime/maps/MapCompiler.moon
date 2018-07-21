-- Box2D generation utilities
B2GenerateUtils = require "maps.B2GenerateUtils"
GenerateUtils = require "maps.GenerateUtils"
DebugUtils = require "maps.DebugUtils"
SourceMap = require "core.SourceMap"
MapRegionShapes = require("maps.MapRegionShapes")
Map = require "core.Map"
MapUtils = require "maps.MapUtils"
World = require "core.World"
Vaults = require "maps.Vaults"

import MapRegion, combine_map_regions, map_region_bbox, map_regions_bbox
    from require("maps.MapRegion")

import Spread, Shape
    from require "maps.MapElements"

make_polygon_points = (rng, w, h, n_points) ->
    return GenerateUtils.skewed_ellipse_points(rng, {0,0}, {30, 30}, n_points or rng\random(4,8))

expand_if_nested = (value) ->
    if type(value) == "table" and type(value.type) == "string"
        return value.type, value
    else
        return value, {}

MapCompiler = newtype {
    init: (args) =>
        -- Result memoizing table
        @result = {}
        @operators = {}
        @post_poned = {}
        @label = assert (args.label or @label)
        @root_node = assert (args.root or @root_node) -- Take from base class
        @rng = assert args.rng
        -- Maps from node -> data
        @_children = {}
        @_regions = {}
        @_group_sets = {}
        @_combined_region = {}
        @_bboxes = {}
        @_spawn_players = args.spawn_players
        @_next_group_id = 1
    add: (selector, operator) =>
        append @operators, {:selector, :operator}

    with_map: (f) =>
        append @post_poned, f
    -- Override to handle explicitly set spawn points

    random_player_square: (player) => MapUtils.random_square(@map, nil)

    get_player_spawn_points: () =>
        log_verbose "get_player_spawn_points #{@label} #{#World.players}"
        spawn_points = for player in *World.players
            log_verbose "get_player_spawn_points #{player.name}"
            sqr = nil
            for i=1,40
                sqr = @random_player_square(player)
                if sqr then break
            if sqr == nil
                return nil
            {x, y} = sqr
            {x*32+16,y*32+16}
        return spawn_points

    get_map_region: (node) =>
        assert @_regions[node], "Must run compile_map_topology on node first!"
        regions = for child in *@_children[node]
            @get_map_region(child)
        return combine_map_regions(regions)

    _recalculate_perimeter: () =>
        -- Detect the perimeter, important for the winding-tunnel algorithm.
        SourceMap.perimeter_apply {map: @map,
            candidate_selector: {matches_all: SourceMap.FLAG_SOLID}, inner_selector: {matches_none: SourceMap.FLAG_SOLID}
            operator: {add: SourceMap.FLAG_PERIMETER}
        }
        SourceMap.perimeter_apply {map: @map,
            candidate_selector: {matches_none: {SourceMap.FLAG_SOLID}},
            inner_selector: {matches_all: {SourceMap.FLAG_PERIMETER, SourceMap.FLAG_SOLID}}
            operator: {add: Vaults.FLAG_INNER_PERIMETER}
        }
    _generate_shape: (scheme, x, y, w, h) =>
        scheme, scheme_args = expand_if_nested(scheme)
        switch scheme
            when 'deformed_ellipse'
                points = scheme_args.n_points or @rng\random(5, 8)
                polygon = GenerateUtils.skewed_ellipse_points(@rng, {x,y}, {w, h}, points)
                return {polygon}
            when 'ellipse'
                points = scheme_args.n_points or @rng\random(5, 8)
                polygon = GenerateUtils.ellipse_points_0(@rng, {x,y}, {w, h}, points)
                return {polygon}
            when 'rectangle'
                -- Half chance of being rotated:
                if @rng\randomf() > .5
                    w, h = h, w
                polygon = GenerateUtils.ellipse_points_0(@rng, {x,y}, {w, h}, 4)
                return {polygon}
            else
                parts = scheme\split(':')
                name = parts[1]
                if parts[2] == 'convex'
                    polygon = MapRegionShapes.get_shape_convex_hull(name, x, y, w, h)
                    return {polygon}
                else
                    polygons = MapRegionShapes.get_shape_polygons(name, x, y, w, h)
                    return polygons
    _spread_regions: (scheme, regions) =>
        timer = timer_create()

        -- Can translate entire combined region; translates all subregions as well:
        -- TODO smarter spreading
        if type(scheme) == "function"
            return scheme(@, node)

        @scatter_regions(regions)
        scheme, scheme_args = expand_if_nested(scheme)
        switch scheme
            when 'box2d'
                B2GenerateUtils.spread_map_regions {
                    rng: @rng
                    :regions
                    n_iterations: 100
                    mode: 'towards_center'
                    clump_once_near: true
                }
            when 'box2d_solid_center'
                B2GenerateUtils.spread_map_regions {
                    rng: @rng
                    :regions
                    fixed_polygons: @_generate_shape('deformed_ellipse', 0,0,4,4)
                    n_iterations: 100
                    mode: 'towards_fixed_shapes'
                    clump_once_near: true
                }
            when 'rvo_spread', 'rvo_center', 'rvo_ring'
                spreader = GenerateUtils.RVORegionPlacer.create()
                adapters = for r in *regions
                    GenerateUtils.MapRegionAdapter.create(r)
                velocity_func = switch scheme
                    when 'rvo_spread' then GenerateUtils.spread_region_delta_func
                    when 'rvo_center' then GenerateUtils.center_region_delta_func
                    when 'rvo_ring' then GenerateUtils.ring_region_delta_func
                for r in *adapters
                    spreader\add(r, velocity_func(nil, @rng, {center: () => 0,0}))
                for i=1,scheme_args.iters or 1000
                    spreader\step() -- TODO customizable
                for i, r in ipairs regions
                    {:x, :ox, :y, :oy} = adapters[i]
                    r\translate(x - ox, y - oy)
            else
                error("Unexpected")
        log_verbose "Spread regions time: #{timer\get_milliseconds()}ms"

    property: (node, property, default=nil) =>
        if node.properties[property] == nil
            return default
        return ode.properties[property]
    connect_map_regions: (regions, n_connections=2) =>
        B2GenerateUtils.connect_map_regions {
            rng: @rng
            :regions
            :n_connections
        }
    _connect_regions: (scheme, regions) =>
        scheme, scheme_args = expand_if_nested(scheme)
        switch scheme
            when 'direct_light'
                timer = timer_create()
                @connect_map_regions(regions, 2)
                log_verbose "Connect regions time: #{timer\get_milliseconds()}ms"
            when 'direct'
                timer = timer_create()
                @connect_map_regions(regions, #regions * 2)
                log_verbose "Connect regions time: #{timer\get_milliseconds()}ms"
            when 'minimum_spanning_arc_and_line'
                @with_map (map) ->
                    adapters = for r in *regions
                        GenerateUtils.MapRegionAdapter.create(r)
                    edges = GenerateUtils.subregion_minimum_spanning_tree adapters, () ->
                        r1 = @rng\random(scheme_args.acceptable_dist or 12)
                        r2 = @rng\random(scheme_args.acceptable_dist or 12)
                        return r1 + r2
                    map.arc_chance = 0.2
                    NewMaps = require "maps.NewMaps"
                    NewMaps.connect_edges map, @rng, {
                        floor1: @tileset.floor,
                        floor2: @tileset.floor_alt
                        connect_line_width: () ->
                            if @rng\randomf() < 0.2
                                return 2
                            return 1
                    }, {1,1, map.size[1] - 1, map.size[2] - 1}, edges
            when 'random_tunnels'
                error("Unexpected")
            when 'none'
                nil
            else
                error("Unexpected")

    _flatten: (l, accum={}) =>
        if getmetatable(l)
            append accum, l
        else
            for elem in *l
                @_flatten(elem, accum)
        return accum
    -- Sets node_children and node_regions

    scatter_regions: (regions,  scatter_x=20, scatter_y=20) =>
        for region in *regions
            x, y = @rng\random(-scatter_x, scatter_x), @rng\random(-scatter_y,scatter_y)
            region\translate(x, y)
        return nil

    _prepare_children_topology: (parent, regions, children_set) =>
        for region_node in *@_flatten(regions)
            append children_set, region_node
            @_prepare_map_topology(region_node, parent)

    _prepare_map_topology: (node, parent=nil) =>
        map_regions = {}
        children_set = {}
        combined_region = nil
        node.parent = parent
        node.properties or= {}
        if parent ~= nil
            for k,v in pairs(parent.properties)
                if node.properties[k] ~= nil
                    continue
                -- Only inherit properties if we have not set them explicitly
                node.properties[k] = v
        switch getmetatable(node)
            when Spread
                {:name, :regions, :connection_scheme, :spread_scheme} = node
                @_prepare_children_topology(node, regions, children_set)
                child_regions = [@_combined_region[child] for child in *node.regions]
                -- Handle node spreading
                if type(spread_scheme) == "function"
                    val = node.spread_scheme(@, node)
                    return false if val == false
                else
                    val = @_spread_regions(spread_scheme, child_regions)
                    return false if val == false
                -- Handle node connection
                if type(connection_scheme) == "function"
                    val = node.connection_scheme(@, node)
                    return false if val == false
                else
                    val = @_connect_regions(connection_scheme, child_regions)
                    return false if val == false
                combined_region = combine_map_regions(child_regions)
                DebugUtils.visualize_map_regions {regions: child_regions, title: "After connection / spread"}
            when Shape
                {:name, :shape, :size, :spread_scheme} = node
                {w, h} = size
                shape = @_generate_shape(shape,-w/2,-h/2,w,h)
                region = MapRegion.create(shape)
                append map_regions, region
                if node.regions
                    scatter_x, scatter_y = math.min(20, w / 10), math.min(20, h / 10)
                    @_prepare_children_topology(node, node.regions, children_set)
                    child_regions = [@_combined_region[child] for child in *node.regions]
                    @scatter_regions(child_regions, scatter_x, scatter_y)
                    B2GenerateUtils.spread_map_regions {
                        rng: @rng
                        regions: child_regions
                        fixed_polygons: table.reversed(shape)
                        n_iterations: 100
                        mode: 'towards_center'
                        clump_once_near: true
                    }
                    combined_region = combine_map_regions(child_regions)
                    combined_region = combine_map_regions({region, combined_region})
                else
                    combined_region = region
                DebugUtils.visualize_map_regions {regions: map_regions, title: "Shape"}
            else
                error("Unknown node")

        -- Regions 'owned' by this node
        @_regions[node] = map_regions
        @_children[node] = children_set
        @_combined_region[node] = assert combined_region, "Need combined_region ~= nil!"

    as_owned_regions: (node) => @_regions[node]
    as_children: (node) => @_children[node]
    as_region: (node) => @_combined_region[node]
    as_size: (node) =>
        bbox = @as_bbox node
        w,h = (bbox[3] - bbox[1]), (bbox[4] - bbox[2])
        return {w, h}
    as_bbox: (node, padding=0) =>
        --if not @_bboxes[node]
        @_bboxes[node] = map_region_bbox(@_combined_region[node])
        if padding ~= 0
            {w,h} = @map.size
            {x1, y1, x2, y2} = @_bboxes[node]
            return {
                math.max(0, math.min(x1 - padding, w))
                math.max(0, math.min(y1 - padding, h))
                math.max(0, math.min(x2 + padding, w))
                math.max(0, math.min(y2 + padding, h))
            }
        return @_bboxes[node]
    as_group_set: (node) => @_group_sets[node]

    fill_unconnected: () =>
        {w,h} = @map.size
        SourceMap.area_fill_unconnected {
            map: @map
            seed: {w/2, h/2}
            unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
            mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
            marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
            fill_operator: {content: @tileset.wall, add: SourceMap.FLAG_SOLID, remove: SourceMap.FLAG_SEETHROUGH}
        }
    apply: (node, args, tunnel_args) =>
        assert not args.map, "Passing map here redundant!"
        args.map = @map
        if tunnel_args then tunnel_args.map = @map
        for region in *@_regions[node]
            region\tunnel_apply(tunnel_args or args)
            region\inner_apply(args)

    -- Depth-first traversal of node tree
    for_all_nodes: (func) =>
        recurse = (node) ->
            for child in * @_children[node]
                recurse(child)
            func(@, node)
        recurse(@root_node)

    tile_paint: (node, floor_tile, tunnel_tile) =>
        if not floor_tile
            floor_tile = (if node.group%2 == 1 then @tileset.floor else @tileset.floor_alt)
        if not tunnel_tile
            tunnel_tile = (if node.group%2 == 1 then @tileset.floor_alt else @tileset.floor)
        @apply node, {
            operator: {
                remove: {SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL}
                add: SourceMap.FLAG_SEETHROUGH
                content: floor_tile, group: node.group
            }
        }, {
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: {SourceMap.FLAG_SEETHROUGH, SourceMap.FLAG_TUNNEL}
                content: tunnel_tile, group: node.group
            }
        }


    tile_paint_perimeter: (node, wall_tile=nil) =>
        if not wall_tile
            wall_tile = (if node.group%2 == 1 then @tileset.wall else @tileset.wall_alt)
        SourceMap.perimeter_apply {map: @map,
            candidate_selector: {matches_all: SourceMap.FLAG_SOLID},
            inner_selector: {matches_none: SourceMap.FLAG_SOLID, matches_group: @_group_sets[node]}
            operator: {
                content: wall_tile
            }
        }

    paint_children: (node) =>
        for child in *@_children[node]
            val = @paint_node(child)
            return false if val == false
        return true

    paint_node: (node) =>
        -- Determine root (lowest) value in group range
        node.group = SourceMap.group_create(@map, {0,0,0,0}, 0)
        -- Paint the node
        if node.paint
            val = node.paint(@, node)
            return false if val == false
        else
            @tile_paint(node, node.properties.floor_tile, node.properties.tunnel_tile)
        if not @paint_children(node)
            return false
        -- Determine highest value in group range
        group_child_max = SourceMap.map_num_groups(@map) - 1
        -- Set group range
        @_group_sets[node] = {node.group, group_child_max}
        -- Fill perimeter with wall tile
        @tile_paint_perimeter(node, node.properties.wall_tile)
        return true

    _shuffle: (tbl) =>
      for i=#tbl,1,-1 do
          rand = @rng\random(1, #tbl)
          tbl[i], tbl[rand] = tbl[rand], tbl[i]
      return tbl

    node_match: (node, args, shuffle=true) =>
        args.map = @map
        args.area = @as_bbox(node, args.padding)
        args.selector.matches_group = @as_group_set node
        squares = SourceMap.rectangle_match(args)
        if shuffle then @_shuffle squares
        return squares

    place_children_objects: (node) =>
        for child in *@_children[node]
            val = @place_node_objects(child)
            return false if val == false
        return true

    place_node_objects: (node) =>
        -- Determine root (lowest) value in group range
        -- Create the node's objects
        if node.place_objects
            val = node.place_objects(@, node)
            return false if val == false
        return @place_children_objects(node)

    _prepare_source_map: (label, padding, content) =>
        -- Correct map topology:
        bbox = map_region_bbox @_combined_region[@root_node]
        -- Assert that our polygons fit within our created source map bounds:
        w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
        -- TODO DEBUG-ONLY CHECK Really make sure we are drawing in correct bounds

        @_combined_region[@root_node]\translate(-bbox[1] + padding, -bbox[2] + padding)

        @for_all_nodes (node) =>
            for polygon in *@_combined_region[node].polygons
                for {x, y} in *polygon
                    assert x >= padding - 0.1 and x <= w+padding +0.1, "pad=#{padding}, #{x}, #{w}"
                    assert y >= padding - 0.1 and y <= h+padding +0.1 , "pad=#{padding}, #{y}, #{h}"
            for {:polygon} in *@_combined_region[node].tunnels
                for {x, y} in *polygon
                    assert x >= padding - 0.1 and x <= w+padding +0.1, "pad=#{padding}, #{x}, #{w}"
                    assert y >= padding - 0.1 and y <= h+padding +0.1 , "pad=#{padding}, #{y}, #{h}"

            --{x1, y1, x2, y2} = @as_bbox(node)
            --assert x1 >= padding - 0.1 and x2 < w + padding + 0.1
            --assert y1 >= padding - 0.1 and y2 < h + padding + 0.1

        --print("WIDTH", w, "HEIGHT", h)
        @map = SourceMap.map_create {
            rng: @rng
            :label
            size: {w + padding*2, h + padding*2}
            flags: SourceMap.FLAG_SOLID
            instances: {}
            :content
        }
        if not @paint_node(@root_node)
            return false
        @_recalculate_perimeter()
        if not @place_node_objects(@root_node)
            return false

        if @_spawn_players
            spawn_points = @get_player_spawn_points()
            if not spawn_points
                return false
            @_player_spawn_points = spawn_points
        for callback in *@post_poned
            callback(@map)
        SourceMap.erode_diagonal_pairs {map: @map, rng: @rng, selector: {matches_all: SourceMap.FLAG_SOLID}}
        if not SourceMap.area_fully_connected {
            map: @map,
            unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
            mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
            marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
        }
            log_verbose("ABORT: connection check failed")
            return false
        @_recalculate_perimeter()
        return true

    random_square: (area = nil) =>
        return MapUtils.random_square(@map, area, {matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
    random_square_not_near_wall: (area = nil) =>
        return MapUtils.random_square(@map, area, {matches_none: {Vaults.FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})

    -- Creates @map, ready to be filled
    prepare: (args={}) =>
        MAX_TRIES = 1000
        for i=1,MAX_TRIES
            table.clear @_children
            table.clear @_regions
            table.clear @_group_sets
            table.clear @_combined_region
            table.clear @_bboxes
            padding = args.padding or 10
            content = args.content or (if rawget(self, 'tileset') then @tileset.wall else 0)
            val = @_prepare_map_topology(@root_node)
            if val == false
                continue
            success = @_prepare_source_map(@label, padding, content)
            if success
                return true
        return false
    -- Creates a game map
    compile: (args) =>
        gmap = Map.create {
            map: @map
            label: @map.label
            instances: @map.instances
            wandering_enabled: true
        }

        if @_spawn_players
            World = require "core.World"
            World.players_spawn gmap, @_player_spawn_points
            if os.getenv "LANARTS_XP"
                for p in *World.players
                    p.instance\gain_xp(tonumber(os.getenv "LANARTS_XP"))
        return gmap
}

main = (raw_args) ->
    -- AreaTemplate -> MapAreaSet -> Map
    area = Spread {
        regions: for i=1,3
            Shape {
                shape: 'deformed_ellipse'
                size: {(i+5) * 10, (i+5) * 10}
            }
        connection_scheme: 'direct'
        spread_scheme: 'box2d'
    }
    rng = require('mtwist').create(os.time())
    compiler = MapCompiler.create {
        label: "Demo"
        :rng,
        root: area
    }

    i = 1
    compiler\for_all_nodes (node) =>
        @apply node, {
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: SourceMap.FLAG_SEETHROUGH
                content: i
            }
        }
        i += 1
    DebugUtils.enable_visualization(800, 600)
    DebugUtils.debug_show_source_map(compiler.map)

return {:MapCompiler, :main}
