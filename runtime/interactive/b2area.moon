import Area, Polygon
    from require "maps.MapElements"

import MapArea, MapAreaSet
    from require "maps.MapAreas"

SourceMap = require "core.SourceMap"
DebugUtil = require "maps.DebugUtil"
GenerateUtils = require "maps.GenerateUtils"
B2ShapeSet = require "maps.B2ShapeSet"

rng = require('mtwist').create(os.time())

make_polygon_points = (rng, w, h, n_points) ->
    return GenerateUtils.skewed_ellipse_points(rng, {0,0}, {30, 30}, rng\random(4,12))

-- AreaTemplate -> MapAreaSet -> Map
area = Area {
    name: 'root'
    subareas: for i=1,3
        Polygon {
            name: 'main'
            points: make_polygon_points(rng, 10, 10, 4)
        }
    connection_scheme: 'direct'
}

pretty(area)

_element_method = (args) -> (area, ...) ->
    t = getmetatable(area)
    f = args[t] or args.default
    return f(area, ...)

-- Create a resolved map topology
generate_map_area = _element_method {
    [Area]: () =>
        subareas = for subarea in @subareas
            generate_map_area(subarea)
        
        switch @connection_scheme
            when "direct"

            else
                error("Unexpected")
        MapAreaSet @name, subareas, conv

    [Polygon]: () =>
    default: () =>
        error("Unexpected")
}

--DebugUtil.visualize_source_map(map, 8, 8) 
--DebugUtil.visualize_source_map(map, 8, 8) 
