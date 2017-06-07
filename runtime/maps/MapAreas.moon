GeometryUtils = require "maps.GeometryUtils"
import Shape, spread_shapes
    from require "maps.B2ShapeSet"

polygon_bbox = (points) -> 
    x1,y1 = math.huge, math.huge
    x2,y2 = -math.huge, -math.huge
    for {x, y} in *points
        x1, y1 = math.min(x1, x), math.min(y1, y)
        x2, y2 = math.max(x2, x), math.max(y2, y)
    return {x1, y1, x2, y2}

-- A resolved polygonal area on a map.
-- Considered immutable once returned from generate_map_area.
-- TODO make debug helpers to ensure classes are used immutably.
-- TODO or in C++?.
MapArea = newtype {
    -- NOTE: Topology should be finalized upon construction
    init: (name, subareas = {}, polygon = false) =>
        @name = name
        @subareas = subareas
        @background_polygon = polygon
        for area in @subareas
            assert getmetatable(area) == MapArea
        -- Post area generation objects
        -- Have a generate() method
        @after_area_generate = {}
        -- Post full level generation objects
        -- Have a generate() method
        -- TODO is this needed?
        --@after_level_generate = {}
    bbox: () =>
        return polygon_bbox(@polygon)
    to_shape: () =>
        if #@subareas > 0
        return (
}

spread_areas = () ->
    return spread_shapes {
        :rng
        shapes: make_shapes(4)
        fixed_shapes: {Shape.create({make_polygon(8, 8)})}
        n_iterations: 50
        --visualize: true
        mode: 'towards_fixed_shapes'
        clump_once_near: true
        connect_after: true
        return_compound_shape: true
        --scale: {1.1, 1.1}
    }

MapAreaSet = newtype {
    init: (root) =>
        assert getmetatable(root) == MapArea
        @root = root
}

return {:MapArea, :MapAreaSet}
