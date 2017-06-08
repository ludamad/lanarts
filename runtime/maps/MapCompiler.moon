-- Box2D generation utilities
B2GenerateUtils = require "maps.B2GenerateUtils"
GenerateUtils = require "maps.GenerateUtils"
GeometryUtils = require "maps.GeometryUtils"
DebugUtils = require "maps.DebugUtils"
SourceMap = require "core.SourceMap"
MapRegionShapes = require("maps.MapRegionShapes")
Map = require "core.Map"

import MapRegion, combine_map_regions, map_regions_bbox
    from require("maps.MapRegion")

import ConnectedRegions, FilledRegion
    from require "maps.MapElements"

make_polygon_points = (rng, w, h, n_points) ->
    return GenerateUtils.skewed_ellipse_points(rng, {0,0}, {30, 30}, rng\random(4,12))

MapCompiler = newtype {
    init: (args) =>
        -- Result memoizing table 
        @result = {}
        @operators = {}
        @root_node = assert args.root
        @rng = assert args.rng
        @instances = {}
        -- Maps from node -> data
        @_children = {}
        @_regions = {}
        @_combined_region = {}
        @_prepare(args)
    add: (selector, operator) =>
        append @operators, {:selector, :operator}
 
    get_map_region: (node) =>
        assert @_regions[node], "Must run compile_map_topology on node first!"
        regions = for child in *@_children[node]
            region = @get_map_region(child)
        combine_map_regions()

    _generate_shape: (scheme, x, y, w, h) =>
        switch scheme
            when 'deformed_ellipse'
                points = @rng\random(5, 15)
                polygon = GenerateUtils.skewed_ellipse_points(@rng, {x,y}, {w, h}, points)
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
        assert scheme == 'box2d'
        B2GenerateUtils.spread_map_regions {
            rng: @rng
            :regions 
            fixed_polygons: @_generate_shape('deformed_ellipse', 0,0,4,4)
            n_iterations: 10
            mode: 'towards_fixed_shapes'
            clump_once_near: true
        }

    _connect_regions: (scheme, regions) =>
        assert scheme == 'direct'
        B2GenerateUtils.connect_map_regions {
            rng: @rng
            :regions 
            n_connections: #regions * 2
        }

    -- Sets node_children and node_regions
    _prepare_map_topology: (node) =>
        map_regions = {}
        children = {}
        combined_region = nil
        switch getmetatable(node)
            when ConnectedRegions
                {:name, :regions, :connection_scheme, :spread_scheme} = node
                child_regions = for region_node in *regions
                    append children, region_node
                    @_prepare_map_topology(region_node)
                    -- TODO smarter spreading
                    x, y = @rng\random(-20, 20), @rng\random(-20,20)
                    -- Can translate entire combined region; translates all subregions as well:
                    combined_region = @_combined_region[region_node]
                    combined_region\translate(x, y)
                    combined_region
                @_spread_regions(spread_scheme, child_regions)
                @_connect_regions(connection_scheme, child_regions)
                combined_region = combine_map_regions(child_regions)
                DebugUtils.visualize_map_regions {regions: child_regions, title: "After connection / spread"}

            when FilledRegion
                {:name, :shape, :size} = node
                {w, h} = size
                combined_region = MapRegion.create @_generate_shape(shape,0,0,w,h)
                append map_regions, combined_region
                DebugUtils.visualize_map_regions {regions: map_regions, title: "FilledRegion"}
            else
                error("Unknown node")
        -- Regions 'owned' by this node
        @_regions[node] = map_regions
        @_children[node] = children
        @_combined_region[node] = assert combined_region, "Need combined_region ~= nil!"
    get_node_owned_regions: (node) => @_regions[node]
    get_node_children: (node) => @_children[node]
    get_node_total_region: (node) => @_combined_region[node]

    apply: (node, args) =>
        assert not args.map, "Passing map here redundant!"
        args.map = @map
        for region in *@_regions[node]
            region\apply(args)

    for_all_nodes: (func) =>
        recurse = (node) ->
            func(@, node)
            for child in * @_children[node]
                recurse(child)
        recurse(@root_node)

    _prepare_source_map: (label, padding, content) => 
        -- Correct map topology:
        bbox = map_regions_bbox({@_combined_region[@root_node]})
        total_region = @get_node_total_region(@root_node)
        total_region\translate(-bbox[1] + padding, -bbox[2] + padding)
        -- TODO remove this when confident about the code
        -- Assert that our polygons fit within our created source map bounds:
        w, h = bbox[3] - bbox[1], bbox[4] - bbox[2] 
        for polygon in *total_region.polygons
            for {x, y} in *polygon
                assert x >= padding - 0.1 and x <= w+padding +0.1, "pad=#{padding}, #{x}, #{w}"
                assert y >= padding - 0.1 and y <= h+padding +0.1 , "pad=#{padding}, #{y}, #{h}"
        @map = SourceMap.map_create {
            rng: @rng
            :label
            size: {w + padding*2, h + padding*2}
            flags: SourceMap.FLAG_SOLID
            :content 
        }

    -- Creates @map, ready to be filled
    _prepare: (args) => 
        label = assert args.label, "Should have map 'label'"
        padding = args.padding or 10
        content = args.content or 0
        @_prepare_map_topology(@root_node)
        return @_prepare_source_map(label, padding, content)
    -- Creates a game map
    compile: () =>
        return Map.create {
            map: @map
            label: @map.label
            instances: @instances
            wandering_enabled: true
        }
}

main = (raw_args) ->
    -- AreaTemplate -> MapAreaSet -> Map
    area = ConnectedRegions {
        regions: for i=1,3
            FilledRegion {
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
