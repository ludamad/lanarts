local B2GenerateUtils = require("maps.B2GenerateUtils")
local GenerateUtils = require("maps.GenerateUtils")
local GeometryUtils = require("maps.GeometryUtils")
local json = require("json")
local success, shape_data = json.parse(file_as_string("maps/shapes.json"))
assert(success, "Could not load maps/shapes.json as a JSON object!")
local get_shape_polygons = memoized(function(name)
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    local _list_0 = shape_data[name]
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local shape
      shape = _des_0.shape
      do
        local _accum_1 = { }
        local _len_1 = 1
        for i = 1, #shape, 2 do
          _accum_1[_len_1] = {
            shape[i],
            shape[i + 1]
          }
          _len_1 = _len_1 + 1
        end
        _accum_0[_len_0] = _accum_1
      end
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end)
local get_shape_convex_hull = memoized(function(name)
  return GeometryUtils.convex_hull(get_shape_polygons(name))
end)
local get_shape_bbox = memoized(function(name)
  local bbox = GeometryUtils.polygon_set_bbox(get_shape_polygons(name))
  return bbox
end)
local transform_point
transform_point = function(point, x, y, width, height, bbox)
  local bw, bh = bbox[3] - bbox[1], bbox[4] - bbox[2]
  local px, py
  px, py = point[1], point[2]
  px, py = px - bbox[1], py - bbox[2]
  px, py = px / bw - 0.5, py / bh - 0.5
  return {
    x + px * width,
    y + py * height
  }
end
return {
  transform_point = transform_point,
  get_shape_polygons = function(name, x, y, width, height)
    local polygons = get_shape_polygons(name)
    local bbox = get_shape_bbox(name)
    return (function()
      local _accum_0 = { }
      local _len_0 = 1
      for _index_0 = 1, #polygons do
        local polygon = polygons[_index_0]
        do
          local _accum_1 = { }
          local _len_1 = 1
          for _index_1 = 1, #polygon do
            local p = polygon[_index_1]
            _accum_1[_len_1] = transform_point(p, x, y, width, height, bbox)
            _len_1 = _len_1 + 1
          end
          _accum_0[_len_0] = _accum_1
        end
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)()
  end,
  get_shape_convex_hull = function(name, x, y, width, height)
    local polygon = get_shape_convex_hull(name)
    local bbox = get_shape_bbox(name)
    return (function()
      local _accum_0 = { }
      local _len_0 = 1
      for _index_0 = 1, #polygon do
        local p = polygon[_index_0]
        _accum_0[_len_0] = transform_point(p, x, y, width, height, bbox)
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)()
  end
}
