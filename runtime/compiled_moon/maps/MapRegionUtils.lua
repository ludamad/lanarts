local MapUtils = require("maps.MapUtils")
local SourceMap = require("core.SourceMap")
local MapRegion, map_regions_bbox
do
  local _obj_0 = require("maps.MapRegion")
  MapRegion, map_regions_bbox = _obj_0.MapRegion, _obj_0.map_regions_bbox
end
local shuffle
shuffle = function(rng, tbl)
  for i = #tbl, 1, -1 do
    local rand = rng:random(1, #tbl + 1)
    tbl[i], tbl[rand] = tbl[rand], tbl[i]
  end
  return tbl
end
local shuffle_cpy
shuffle_cpy = function(rng, tbl)
  return shuffle(rng, table.clone(tbl))
end
local find_bbox
find_bbox = function(region_set, size, perimeter, n_attempts)
  if perimeter == nil then
    perimeter = { }
  end
  if n_attempts == nil then
    n_attempts = 100
  end
  local map, regions
  map, regions = region_set.map, region_set.regions
  local shuffled_regions = shuffle_cpy(map.rng, regions)
  for i = 1, n_attempts do
    for _index_0 = 1, #shuffled_regions do
      local region = shuffled_regions[_index_0]
      local w, h
      w, h = size[1], size[2]
      local x1, y1, x2, y2
      do
        local _obj_0 = region:bbox()
        x1, y1, x2, y2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
      end
      x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
      x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
      local top_left = MapUtils.random_square(map, {
        x1,
        y1,
        x2,
        y2
      }, region.selector, nil, 1)
      if top_left then
        local top_x, left_y
        top_x, left_y = top_left[1], top_left[2]
        local bbox = {
          top_x,
          left_y,
          top_x + w,
          left_y + h
        }
        if SourceMap.rectangle_query({
          map = map,
          area = bbox,
          fill_selector = region.selector or { },
          perimeter_width = perimeter.width,
          perimeter_selector = perimeter.selector or { }
        }) then
          return {
            region = region,
            bbox = bbox
          }
        end
      end
    end
  end
  return nil
end
local find_square
find_square = function(region_set)
  local region, bbox
  do
    local _obj_0 = random_bbox(region_set, {
      1,
      1
    })
    region, bbox = _obj_0.region, _obj_0.bbox
  end
  return {
    region = region,
    square = {
      bbox[1],
      bbox[2]
    }
  }
end
local selector_map
selector_map = function(region_set, f)
  local map, regions
  map, regions = region_set.map, region_set.regions
  return {
    map = map,
    regions = map_call((function(r)
      return r:with_selector(f(r.selector))
    end), regions)
  }
end
local selector_filter
selector_filter = function(region_set, selector)
  local map, regions
  map, regions = region_set.map, region_set.regions
  return {
    map = map,
    regions = map_call((function(r)
      return r:with_selector(selector)
    end), regions)
  }
end
local bbox
bbox = function(region_set)
  return map_regions_bbox(region_set.regions)
end
local center
center = function(region_set)
  bbox = map_regions_bbox(region_set.regions)
  local x = (bbox[1] + bbox[3]) / 2
  local y = (bbox[2] + bbox[4]) / 2
  return {
    x,
    y
  }
end
return {
  center = center,
  bbox = bbox,
  find_bbox = find_bbox,
  find_square = find_square,
  selector_filter = selector_filter,
  selector_map = selector_map
}
