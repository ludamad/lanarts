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
    for polygon in polygons
        for point in polygon
            _tinsert points, point

    local p = #points

    _lexicographical_point_sort(points)

    local lower = {}
    for i = 1, p
        while (#lower >= 2 and cross(lower[#lower - 1], lower[#lower], points[i]) <= 0)
            _tremove(lower, #lower)

        _tinsert(lower, points[i])

    local upper = {}
    for i = p, 1, -1
        while (#upper >= 2 and cross(upper[#upper - 1], upper[#upper], points[i]) <= 0)
            _tremove(upper, #upper)
        _tinsert(upper, points[i])

    _tremove(upper, #upper)
    _tremove(lower, #lower)
    for _, point in ipairs(lower)
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

return {:convex_hull, :scale_polygon}
