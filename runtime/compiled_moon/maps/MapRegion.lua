local B2GenerateUtils = require("maps.B2GenerateUtils")
local GenerateUtils = require("maps.GenerateUtils")
local DebugUtils = require("maps.DebugUtils")
local SourceMap = require("core.SourceMap")
local MapRegion, map_region_bbox
MapRegion = newtype({
  init = function(self, polygons, tunnels, selector)
    if tunnels == nil then
      tunnels = { }
    end
    if selector == nil then
      selector = false
    end
    self.polygons = assert(polygons)
    self.tunnels = tunnels
    self.selector = selector
  end,
  as_set = function(self, map)
    return {
      map = map,
      regions = {
        self
      }
    }
  end,
  with_selector = function(self, selector)
    return MapRegion.create(self.polygons, self.tunnels, selector)
  end,
  inner_apply = function(self, args)
    local map, area, operator
    map, area, operator = args.map, args.area, args.operator
    local _list_0 = self.polygons
    for _index_0 = 1, #_list_0 do
      local polygon = _list_0[_index_0]
      for _index_1 = 1, #polygon do
        local _des_0 = polygon[_index_1]
        local x, y
        x, y = _des_0[1], _des_0[2]
        assert(x > 5 and y > 5)
        assert(x < map.size[1] - 5 and y < map.size[2] - 5)
      end
      SourceMap.polygon_apply({
        map = map,
        area = area,
        operator = operator,
        points = polygon
      })
    end
    return nil
  end,
  bbox = function(self)
    return map_region_bbox(self)
  end,
  tunnel_apply = function(self, args)
    local map, area, operator
    map, area, operator = args.map, args.area, args.operator
    local _list_0 = self.tunnels
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local polygon
      polygon = _des_0.polygon
      for _index_1 = 1, #polygon do
        local _des_1 = polygon[_index_1]
        local x, y
        x, y = _des_1[1], _des_1[2]
        assert(x > 5 and y > 5)
        assert(x < map.size[1] - 5 and y < map.size[2] - 5)
      end
      SourceMap.polygon_apply({
        map = map,
        area = area,
        operator = operator,
        points = polygon
      })
    end
    return nil
  end,
  apply = function(self, args)
    self:tunnel_apply(args)
    return self:inner_apply(args)
  end,
  translate = function(self, x, y)
    local _list_0 = self.polygons
    for _index_0 = 1, #_list_0 do
      local polygon = _list_0[_index_0]
      for _index_1 = 1, #polygon do
        local point = polygon[_index_1]
        point[1] = point[1] + x
        point[2] = point[2] + y
      end
    end
    local _list_1 = self.tunnels
    for _index_0 = 1, #_list_1 do
      local _des_0 = _list_1[_index_0]
      local polygon
      polygon = _des_0.polygon
      for _index_1 = 1, #polygon do
        local point = polygon[_index_1]
        point[1] = point[1] + x
        point[2] = point[2] + y
      end
    end
    return nil
  end,
  rotate = function(self, angle, center)
    if center == nil then
      center = nil
    end
    center = center or self:center()
    local cx, cy
    cx, cy = center[1], center[2]
    local cos, sin = math.cos(angle), math.sin(angle)
    self:translate(-cx, -cy)
    local _list_0 = self.polygons
    for _index_0 = 1, #_list_0 do
      local polygon = _list_0[_index_0]
      for _index_1 = 1, #polygon do
        local point = polygon[_index_1]
        local x, y
        x, y = point[1], point[2]
        point[1] = x * cos - y * sin
        point[2] = x * sin + y * cos
      end
    end
    local _list_1 = self.tunnels
    for _index_0 = 1, #_list_1 do
      local _des_0 = _list_1[_index_0]
      local polygon
      polygon = _des_0.polygon
      for _index_1 = 1, #polygon do
        local point = polygon[_index_1]
        local x, y
        x, y = point[1], point[2]
        point[1] = x * cos - y * sin
        point[2] = x * sin + y * cos
      end
    end
    return self:translate(cx, cy)
  end,
  center = function(self)
    local accum_x, accum_y = 0, 0
    local n_points = 0
    local polygons, tunnels
    polygons, tunnels = self.polygons, self.tunnels
    for _index_0 = 1, #polygons do
      local polygon = polygons[_index_0]
      for _index_1 = 1, #polygon do
        local _des_0 = polygon[_index_1]
        local x, y
        x, y = _des_0[1], _des_0[2]
        accum_x, accum_y = accum_x + x, accum_y + y
        n_points = n_points + 1
      end
    end
    if n_points <= 0 then
      return {
        0,
        0
      }
    end
    return {
      accum_x / n_points,
      accum_y / n_points
    }
  end
})
local combine_map_regions
combine_map_regions = function(regions)
  local c_polygons = { }
  local c_tunnels = { }
  for _index_0 = 1, #regions do
    local _des_0 = regions[_index_0]
    local polygons, tunnels
    polygons, tunnels = _des_0.polygons, _des_0.tunnels
    for _index_1 = 1, #polygons do
      local p = polygons[_index_1]
      append(c_polygons, p)
    end
    for _index_1 = 1, #tunnels do
      local t = tunnels[_index_1]
      append(c_tunnels, t)
    end
  end
  return MapRegion.create(c_polygons, c_tunnels)
end
local map_regions_bbox
map_regions_bbox = function(regions)
  local x1, y1 = math.huge, math.huge
  local x2, y2 = -math.huge, -math.huge
  for _index_0 = 1, #regions do
    local _des_0 = regions[_index_0]
    local polygons, tunnels
    polygons, tunnels = _des_0.polygons, _des_0.tunnels
    for _index_1 = 1, #polygons do
      local polygon = polygons[_index_1]
      for _index_2 = 1, #polygon do
        local _des_1 = polygon[_index_2]
        local x, y
        x, y = _des_1[1], _des_1[2]
        x1, y1 = math.min(x1, x), math.min(y1, y)
        x2, y2 = math.max(x2, x), math.max(y2, y)
      end
    end
    for _index_1 = 1, #tunnels do
      local _des_1 = tunnels[_index_1]
      local polygon
      polygon = _des_1.polygon
      for _index_2 = 1, #polygon do
        local _des_2 = polygon[_index_2]
        local x, y
        x, y = _des_2[1], _des_2[2]
        x1, y1 = math.min(x1, x), math.min(y1, y)
        x2, y2 = math.max(x2, x), math.max(y2, y)
      end
    end
  end
  return {
    x1,
    y1,
    x2,
    y2
  }
end
map_region_bbox = function(region)
  local x1, y1 = math.huge, math.huge
  local x2, y2 = -math.huge, -math.huge
  local polygons, tunnels
  polygons, tunnels = region.polygons, region.tunnels
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
  for _index_0 = 1, #tunnels do
    local _des_0 = tunnels[_index_0]
    local polygon
    polygon = _des_0.polygon
    for _index_1 = 1, #polygon do
      local _des_1 = polygon[_index_1]
      local x, y
      x, y = _des_1[1], _des_1[2]
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
local main
main = function(raw_args)
  local argparse = require("argparse")
  local parser = argparse("", "")
  parser:option("-S --shape", "Give a shape description"):count("*")
  local args = parser:parse(raw_args)
  local MapRegionShapes = require("maps.MapRegionShapes")
  local rng = require("mtwist").create(os.time())
  local make_polygon
  make_polygon = function(x, y, w, h, points)
    return GenerateUtils.skewed_ellipse_points(rng, {
      x,
      y
    }, {
      w,
      h
    }, points)
  end
  local sample_shape
  sample_shape = function()
    local make_map_regions
    make_map_regions = function(n)
      local regions = { }
      for i, shape_desc in ipairs(args.shape) do
        local x, y = rng:random(-200, 200), rng:random(-200, 200)
        local w, h = (i + 5) * 10, (i + 5) * 10
        local polygons
        if shape_desc == 'random' then
          local polygon = make_polygon(x, y, w, h, (i + 5) * math.random() + 3)
          polygons = append(regions, MapRegion.create({
            polygon
          }))
        else
          local parts = shape_desc:split(':')
          local name = parts[1]
          if parts[2] == 'convex' then
            local polygon = MapRegionShapes.get_shape_convex_hull(name, x, y, w, h)
            polygons = append(regions, MapRegion.create({
              polygon
            }))
          else
            polygons = MapRegionShapes.get_shape_polygons(name, x, y, w, h)
            polygons = append(regions, MapRegion.create(polygons))
          end
        end
      end
      return regions
    end
    local regions = make_map_regions(4)
    if #regions > 1 then
      B2GenerateUtils.spread_map_regions({
        rng = rng,
        regions = regions,
        fixed_polygons = {
          make_polygon(0, 0, 8, 8)
        },
        n_iterations = 50,
        mode = 'towards_fixed_shapes',
        clump_once_near = true
      })
    end
    DebugUtils.enable_visualization(800, 600)
    if #regions > 1 then
      B2GenerateUtils.connect_map_regions({
        rng = rng,
        regions = regions,
        fixed_polygons = {
          make_polygon(0, 0, 8, 8)
        },
        n_connections = 5
      })
    end
    DebugUtils.visualize_map_regions({
      regions = regions,
      title = "After generation"
    })
    return regions
  end
  SourceMap = require("core.SourceMap")
  local regions = sample_shape()
  local bbox = map_regions_bbox(regions)
  local w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
  local padding = 8
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    region:translate(-bbox[1] + padding, -bbox[2] + padding)
  end
  assert(w > 0 and h > 0, "w, h should be > 0, " .. tostring(w) .. ", " .. tostring(h))
  for _index_0 = 1, #regions do
    local _des_0 = regions[_index_0]
    local polygons
    polygons = _des_0.polygons
    for _index_1 = 1, #polygons do
      local polygon = polygons[_index_1]
      for _index_2 = 1, #polygon do
        local _des_1 = polygon[_index_2]
        local x, y
        x, y = _des_1[1], _des_1[2]
        assert(x >= padding and x <= w + padding, "0, " .. tostring(x) .. ", " .. tostring(w))
        assert(y >= padding and y <= h + padding, "0, " .. tostring(y) .. ", " .. tostring(h))
      end
    end
  end
  local map = SourceMap.map_create({
    rng = rng,
    label = "Test",
    size = {
      w + padding * 2,
      h + padding * 2
    },
    flags = SourceMap.FLAG_SOLID
  })
  local timer = timer_create()
  for i = 1, #regions do
    regions[i]:apply({
      map = map,
      operator = {
        remove = SourceMap.FLAG_SOLID,
        add = SourceMap.FLAG_SEETHROUGH,
        content = i
      }
    })
  end
  print("Time spent rasterizing polygons: ", timer:get_milliseconds())
  timer = timer_create()
  SourceMap.area_fill_unconnected({
    map = map,
    seed = {
      w / 2,
      h / 2
    },
    unfilled_selector = {
      matches_none = {
        SourceMap.FLAG_SOLID
      }
    },
    mark_operator = {
      add = {
        SourceMap.FLAG_RESERVED2
      }
    },
    marked_selector = {
      matches_all = {
        SourceMap.FLAG_RESERVED2
      }
    },
    fill_operator = {
      content = 0,
      add = SourceMap.FLAG_SOLID,
      remove = SourceMap.FLAG_SEETHROUGH
    }
  })
  print("Time spent filling unconnected: ", timer:get_milliseconds())
  return DebugUtils.debug_show_source_map(map, 1, 1)
end
local from_bbox
from_bbox = function(x1, y1, x2, y2)
  local polygon = {
    {
      x1,
      y2
    },
    {
      x2,
      y2
    },
    {
      x2,
      y1
    },
    {
      x1,
      y1
    }
  }
  return MapRegion.create({
    polygon
  })
end
return {
  MapRegion = MapRegion,
  main = main,
  combine_map_regions = combine_map_regions,
  map_regions_bbox = map_regions_bbox,
  map_region_bbox = map_region_bbox,
  from_bbox = from_bbox
}
