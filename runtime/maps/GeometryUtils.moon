-- Cache for performance
_tsort, _tinsert, _tremove = table.sort, table.insert, table.remove

_lexicographical_point_compare = (a, b) ->
    return a[1] == b[1] and a[2] > b[2] or a[1] > b[1]

_lexicographical_point_sort = (points) ->
    _tsort points, _lexicographical_point_compare

_cross_product = (p, q, r) ->
    return (q[2] - p[2]) * (r[1] - q[1]) - (q[1] - p[1]) * (r[2] - q[2])

-- Moonscript port of 'https://gist.github.com/sixFingers/ee5c1dce72206edc5a42b3246a52ce2e'
-- From there:
--   Andrew's monotone chain convex hull algorithm
--   https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
--   Direct port from Javascript version
convex_hull = (polygons) ->
    points = {}
    for polygon in *polygons
        for point in *polygon
            _tinsert points, point

    p = #points

    _lexicographical_point_sort(points)

    lower = {}
    for i = 1, p
        while (#lower >= 2 and _cross_product(lower[#lower - 1], lower[#lower], points[i]) <= 0)
            _tremove(lower, #lower)

        _tinsert(lower, points[i])

    upper = {}
    for i = p, 1, -1
        while (#upper >= 2 and _cross_product(upper[#upper - 1], upper[#upper], points[i]) <= 0)
            _tremove(upper, #upper)
        _tinsert(upper, points[i])

    _tremove(upper, #upper)
    _tremove(lower, #lower)
    for point in *lower
        _tinsert(upper, point)

    return upper

scale_polygon = (points, scale_x, scale_y) ->
    cx,cy = 0,0
    for {x, y} in *points
        cx, cy = cx + x, cy + y
    cx,cy = cx/#points, cy/#points
    return for {x, y} in *points
        x = (x - cx)*scale_x + cx
        y = (y - cy)*scale_y + cy
        {x, y}

polygon_bbox = (polygon) -> 
    x1,y1 = math.huge, math.huge
    x2,y2 = -math.huge, -math.huge
    for {x, y} in *polygon
        x1, y1 = math.min(x1, x), math.min(y1, y)
        x2, y2 = math.max(x2, x), math.max(y2, y)
    return {x1, y1, x2, y2}

polygon_set_bbox = (polygons) ->
    x1,y1 = math.huge, math.huge
    x2,y2 = -math.huge, -math.huge
    for polygon in *polygons
        for {x, y} in *polygon

            x1, y1 = math.min(x1, x), math.min(y1, y)
            x2, y2 = math.max(x2, x), math.max(y2, y)
    return {x1, y1, x2, y2}

return {:convex_hull, :scale_polygon, :polygon_set_bbox, :polygon_bbox}
