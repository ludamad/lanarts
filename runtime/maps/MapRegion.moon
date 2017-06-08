-- Box2D generation utilities
B2GenerateUtils = require "maps.B2GenerateUtils"
GenerateUtils = require "maps.GenerateUtils"
GeometryUtils = require "maps.GeometryUtils"
DebugUtils = require "maps.DebugUtils"
SourceMap = require "core.SourceMap"

-- Define a map region in terms of polygons
-- Should be within (0, 0) to (map.w, map.h)
MapRegion = newtype {
    init: (polygons) =>
        @polygons = assert polygons
        @tunnels = {}
    apply: (args) =>
        {:map, :area, :operator} = args
        for polygon in *@polygons
            pretty(polygon)
            SourceMap.polygon_apply {
                :map, :area, :operator
                points: polygon
            }
        for {:polygon} in *@tunnels
            pretty(polygon)
            SourceMap.polygon_apply {
                :map, :area, :operator
                points: polygon
            }
        return nil
    translate: (x, y) =>
        for polygon in *@polygons
            for point in *polygon
                point[1] += x
                point[2] += y
        for {:polygon} in *@tunnels
            for point in *polygon
                point[1] += x
                point[2] += y
        return nil
}

-- Main is used if the module is run directly:
main = (raw_args) ->
    argparse = require "argparse"
    parser = argparse("", "")
    parser\option("-S --shape", "Give a shape description")\count("*")
    args = parser\parse(raw_args)

    MapRegionShapes = require("maps.MapRegionShapes")
    rng = require("mtwist").create(os.time())
    make_polygon = (x, y, w, h, points) ->
        return GenerateUtils.skewed_ellipse_points(rng, {x,y}, {w, h}, points)
    map_regions_bbox = (regions) ->
        x1,y1 = math.huge, math.huge
        x2,y2 = -math.huge, -math.huge
        for {:polygons} in *regions
            for polygon in *polygons
                for {x, y} in *polygon
                    x1, y1 = math.min(x1, x), math.min(y1, y)
                    x2, y2 = math.max(x2, x), math.max(y2, y)
        return {x1, y1, x2, y2}

    sample_shape = () ->
        make_map_regions = (n) ->
            regions = {}
            for i, shape_desc in ipairs args.shape
                x,y = rng\random(-200, 200), rng\random(-200,200)
                w, h = (i+5) * 10, (i+5) * 10
                polygons = if shape_desc == 'random'
                    -- Start at random locations:
                    polygon = make_polygon x, y, w, h, (i+5) * math.random() + 3
                    append regions, MapRegion.create {polygon} -- 1 polygon for 'random'
                else
                    parts = shape_desc\split(':')
                    name = parts[1]
                    if parts[2] == 'convex'
                        polygon = MapRegionShapes.get_shape_convex_hull(name, x, y, w, h)
                        append regions, MapRegion.create {polygon} -- 1 polygon for 'random'
                    else
                        polygons = MapRegionShapes.get_shape_polygons(name, x, y, w, h)
                        for polygon in *polygons
                            append regions, MapRegion.create {polygon} -- 1 polygon for 'random'
            return regions
        regions = make_map_regions(4)
        --B2GenerateUtils.spread_map_regions {
        --    :rng
        --    :regions 
        --    fixed_polygons: {make_polygon(0, 0, 8, 8)}
        --    n_iterations: 50
        --    mode: 'towards_fixed_shapes'
        --    clump_once_near: true
        --}
        DebugUtils.enable_visualization(800, 600)
        if #regions > 1
            B2GenerateUtils.connect_map_regions {
                :rng
                :regions 
                fixed_polygons: {make_polygon(0, 0, 8, 8)}
                n_connections: 5
            }
        DebugUtils.visualize_map_regions {:regions, title: "After generation"}
        return regions

    SourceMap = require "core.SourceMap"
    regions = sample_shape()
    bbox = map_regions_bbox(regions)
    w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
    padding = 8
    -- Make the top y=0 and left x=0
    for region in *regions
        region\translate(-bbox[1] + padding, -bbox[2] + padding)
    assert w > 0 and h > 0, "w, h should be > 0, #{w}, #{h}"
    for {:polygons} in *regions
        for polygon in *polygons
            for {x, y} in *polygon
                assert x >= padding and x <= w+padding, "0, #{x}, #{w}"
                assert y >= padding and y <= h+padding, "0, #{y}, #{h}"
    map = SourceMap.map_create {
        :rng
        label: "Test"
        size: {w + padding*2, h + padding*2}
        flags: SourceMap.FLAG_SOLID
    }

    timer = timer_create()
    for i=1,#regions
        regions[i]\apply {
            :map
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: SourceMap.FLAG_SEETHROUGH
                content: i
            }
        }
    print "Time spent rasterizing polygons: ", timer\get_milliseconds()

    DebugUtils.debug_show_source_map(map, 1, 1)

    timer = timer_create()
    SourceMap.area_fill_unconnected {
        :map, 
        seed: {w/2, h/2}
        unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
        mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
        marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
        fill_operator: {content: 1} --{content: 0, add: SourceMap.FLAG_SOLID, remove: SourceMap.FLAG_SEETHROUGH}
    }
    print "Time spent filling unconnected: ", timer\get_milliseconds()

    --timer = timer_create()
    -- SourceMap.erode_diagonal_pairs {:map, :rng, selector: {matches_all: SourceMap.FLAG_SOLID}}
    --print "Time spent eroding : ", timer\get_milliseconds()

    DebugUtils.debug_show_source_map(map, 1, 1)

return {:MapRegion, :main}
