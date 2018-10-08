B2GenerateUtils = require "maps.B2GenerateUtils"
GenerateUtils = require "maps.GenerateUtils"
GeometryUtils = require "maps.GeometryUtils"

json = require "json"
success, shape_data = json.parse(file_as_string "maps/shapes.json")
assert success, "Could not load maps/shapes.json as a JSON object!"

-- Cache our transform of the shape's polygon points
get_shape_polygons = memoized (name) ->
    return for {:shape} in *shape_data[name]
        for i=1,#shape,2
            {shape[i], shape[i+1]}

-- Cache our transform of the shape's polygon points
get_shape_convex_hull = memoized (name) ->
    return GeometryUtils.convex_hull get_shape_polygons(name)

-- Shape bbox should be part of the metadata but isn't right now; at least cache it
get_shape_bbox = memoized (name) ->
    bbox = GeometryUtils.polygon_set_bbox get_shape_polygons(name)
    return bbox

transform_point = (point, x, y, width, height, bbox) ->
    -- Get bbox width and height
    bw, bh = bbox[3] - bbox[1], bbox[4] - bbox[2]
    {px, py} = point
    -- Move coordinates to between (0,0) to (bw,bh)
    px, py = px - bbox[1], py - bbox[2]
    -- Move coordinates to between (-0.5,-0.5) to (0.5,0.5)
    px, py = px / bw - 0.5, py / bh - 0.5
    -- Return coordinates between (x - 0.5 * width, y - 0.5 * height) to (x + 0.5 * width, y + 0.5 * height)
    return {x + px * width, y + py * height}

return {
    :transform_point
    -- Get a list of convex polygons situated around x, y
    get_shape_polygons: (name, x, y, width, height) ->
        polygons = get_shape_polygons(name)
        bbox = get_shape_bbox(name)
        return for polygon in *polygons
            for p in *polygon
                transform_point(p, x, y, width, height, bbox)
    get_shape_convex_hull: (name, x, y, width, height) ->
        polygon = get_shape_convex_hull(name)
        bbox = get_shape_bbox(name)
        return for p in *polygon
            transform_point(p, x, y, width, height, bbox)
}

