local _tsort, _tinsert, _tremove = table.sort, table.insert, table.remove
local _lexicographical_point_compare
_lexicographical_point_compare = function(a, b)
  return a[1] == b[1] and a[2] > b[2] or a[1] > b[1]
end
local _lexicographical_point_sort
_lexicographical_point_sort = function(points)
  return _tsort(points, _lexicographical_point_compare)
end
local _cross_product
_cross_product = function(p, q, r)
  return (q[2] - p[2]) * (r[1] - q[1]) - (q[1] - p[1]) * (r[2] - q[2])
end
local convex_hull
convex_hull = function(polygons)
  local points = { }
  for _index_0 = 1, #polygons do
    local polygon = polygons[_index_0]
    for _index_1 = 1, #polygon do
      local point = polygon[_index_1]
      _tinsert(points, point)
    end
  end
  local p = #points
  _lexicographical_point_sort(points)
  local lower = { }
  for i = 1, p do
    while (#lower >= 2 and _cross_product(lower[#lower - 1], lower[#lower], points[i]) <= 0) do
      _tremove(lower, #lower)
    end
    _tinsert(lower, points[i])
  end
  local upper = { }
  for i = p, 1, -1 do
    while (#upper >= 2 and _cross_product(upper[#upper - 1], upper[#upper], points[i]) <= 0) do
      _tremove(upper, #upper)
    end
    _tinsert(upper, points[i])
  end
  _tremove(upper, #upper)
  _tremove(lower, #lower)
  for _index_0 = 1, #lower do
    local point = lower[_index_0]
    _tinsert(upper, point)
  end
  return upper
end
local scale_polygon
scale_polygon = function(points, scale_x, scale_y)
  local cx, cy = 0, 0
  for _index_0 = 1, #points do
    local _des_0 = points[_index_0]
    local x, y
    x, y = _des_0[1], _des_0[2]
    cx, cy = cx + x, cy + y
  end
  cx, cy = cx / #points, cy / #points
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #points do
      local _des_0 = points[_index_0]
      local x, y
      x, y = _des_0[1], _des_0[2]
      x = (x - cx) * scale_x + cx
      y = (y - cy) * scale_y + cy
      local _value_0 = {
        x,
        y
      }
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end
local polygon_bbox
polygon_bbox = function(polygon)
  local x1, y1 = math.huge, math.huge
  local x2, y2 = -math.huge, -math.huge
  for _index_0 = 1, #polygon do
    local _des_0 = polygon[_index_0]
    local x, y
    x, y = _des_0[1], _des_0[2]
    x1, y1 = math.min(x1, x), math.min(y1, y)
    x2, y2 = math.max(x2, x), math.max(y2, y)
  end
  return {
    x1,
    y1,
    x2,
    y2
  }
end
local polygon_set_bbox
polygon_set_bbox = function(polygons)
  local x1, y1 = math.huge, math.huge
  local x2, y2 = -math.huge, -math.huge
  for _index_0 = 1, #polygons do
    local polygon = polygons[_index_0]
    for _index_1 = 1, #polygon do
      local _des_0 = polygon[_index_1]
      local x, y
      x, y = _des_0[1], _des_0[2]
      x1, y1 = math.min(x1, x), math.min(y1, y)
      x2, y2 = math.max(x2, x), math.max(y2, y)
    end
  end
  return {
    x1,
    y1,
    x2,
    y2
  }
end
return {
  convex_hull = convex_hull,
  scale_polygon = scale_polygon,
  polygon_set_bbox = polygon_set_bbox,
  polygon_bbox = polygon_bbox
}
