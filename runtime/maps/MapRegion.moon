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
    apply: (args) =>
        {:map, :area, :operator} = args
        for polygon in *@polygons
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
        return nil
}

__visualize = () ->
    DebugUtils.enable_visualization(800, 600)
    rng = require("mtwist").create(os.time())
    make_polygon = (x, y, w, h, points) ->
        x, y= 0,0
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
            return for i=4,3 + n 
                -- Start at random locations:
                polygon = make_polygon rng\random(-200, 200), 
                    rng\random(-200,200), 
                    i * 10, -- w
                    i * 10, -- h
                    i * math.random() + 3
                MapRegion.create {polygon}
        regions = make_map_regions(4)
        B2GenerateUtils.spread_map_regions {
            :rng
            :regions 
            fixed_polygons: {make_polygon(0, 0, 8, 8)}
            n_iterations: 50
            mode: 'towards_fixed_shapes'
            clump_once_near: true
        }
        DebugUtils.visualize_map_regions {:regions, title: "After generation"}
        return regions

    SourceMap = require "core.SourceMap"
    regions = sample_shape()
    pretty('REIGONS', #regions)
    bbox = map_regions_bbox(regions)
    w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
    -- Make the top y=0 and left x=0
    for region in *regions
        region\translate(-bbox[1], -bbox[2])
    assert w > 0 and h > 0, "w, h should be > 0, #{w}, #{h}"
    for {:polygons} in *regions
        for polygon in *polygons
            for {x, y} in *polygon
                assert x >= 0 and x <= w, "0, #{x}, #{w}"
                assert y >= 0 and y <= h, "0, #{y}, #{h}"
    map = SourceMap.map_create {
        :rng
        label: "Test"
        size: {w, h}
        flags: SourceMap.FLAG_SOLID
    }

    timer = timer_create()
    for i=1,#regions
        regions[i]\apply {
            :map
            area: {2,2, w-2, h-2}
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: SourceMap.FLAG_SEETHROUGH
                content: i
            }
        }
    print "Time spent rasterizing polygons: ", timer\get_milliseconds()

    DebugUtils.debug_show_source_map(map, 1, 1)

__visualize()
