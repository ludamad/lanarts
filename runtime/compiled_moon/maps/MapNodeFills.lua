local map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, rectangle_points, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator
do
  local _obj_0 = require("maps.GenerateUtils")
  map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, rectangle_points, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator = _obj_0.map_place_object, _obj_0.ellipse_points, _obj_0.LEVEL_PADDING, _obj_0.Region, _obj_0.RVORegionPlacer, _obj_0.random_rect_in_rect, _obj_0.random_ellipse_in_ellipse, _obj_0.ring_region_delta_func, _obj_0.default_region_delta_func, _obj_0.spread_region_delta_func, _obj_0.center_region_delta_func, _obj_0.towards_region_delta_func, _obj_0.rectangle_points, _obj_0.random_region_add, _obj_0.subregion_minimum_spanning_tree, _obj_0.region_minimum_spanning_tree, _obj_0.Tile, _obj_0.tile_operator
end
local GeometryUtils = require("maps.GeometryUtils")
local MapRegionShapes = require("maps.MapRegionShapes")
local MapRegion, combine_map_regions, from_bbox
do
  local _obj_0 = require("maps.MapRegion")
  MapRegion, combine_map_regions, from_bbox = _obj_0.MapRegion, _obj_0.combine_map_regions, _obj_0.from_bbox
end
local PolyPartition = require("core.PolyPartition")
local DebugUtils = require("maps.DebugUtils")
local NewMaps = require("maps.NewMaps")
local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper = MapUtils.make_tunnel_oper, MapUtils.make_rectangle_criteria, MapUtils.make_rectangle_oper
local Vaults = require("maps.Vaults")
local World = require("core.World")
local SourceMap = require("core.SourceMap")
local Map = require("core.Map")
local OldMaps = require("maps.OldMaps")
local Region1 = require("maps.Region1")
local MapCompilerContext, make_on_player_interact
do
  local _obj_0 = require("maps.MapCompilerContext")
  MapCompilerContext, make_on_player_interact = _obj_0.MapCompilerContext, _obj_0.make_on_player_interact
end
local MapNode, MapDesc
do
  local _obj_0 = require("maps.MapDesc")
  MapNode, MapDesc = _obj_0.MapNode, _obj_0.MapDesc
end
local Places = require("maps.Places")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local center, find_bbox, find_square, selector_filter, selector_map
do
  local _obj_0 = require("maps.MapRegionUtils")
  center, find_bbox, find_square, selector_filter, selector_map = _obj_0.center, _obj_0.find_bbox, _obj_0.find_square, _obj_0.selector_filter, _obj_0.selector_map
end
local MAX_TUNNEL_TRIES = 100
local node_connect_rect_rooms
node_connect_rect_rooms = function(self)
  local regions = self.region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local tries_without_tunnel = 0
    for j = 1, 2 do
      while tries_without_tunnel < MAX_TUNNEL_TRIES do
        local success = SourceMap.try_tunnel_apply({
          map = self.map,
          rng = self.rng,
          start_area = region.inner_bbox,
          start_selector = region.selector,
          validity_selector = {
            fill_selector = {
              matches_group = {
                self.parent.group,
                -1
              },
              matches_all = SourceMap.FLAG_SOLID,
              matches_none = SourceMap.FLAG_TUNNEL
            },
            perimeter_selector = {
              matches_all = SourceMap.FLAG_SOLID,
              matches_none = SourceMap.FLAG_TUNNEL
            }
          },
          completion_selector = {
            fill_selector = {
              matches_none = {
                SourceMap.FLAG_SOLID,
                SourceMap.FLAG_PERIMETER,
                SourceMap.FLAG_TUNNEL
              }
            },
            perimeter_selector = {
              matches_none = SourceMap.FLAG_SOLID
            }
          },
          fill_operator = {
            add = {
              SourceMap.FLAG_TUNNEL,
              SourceMap.FLAG_SEETHROUGH
            },
            remove = SourceMap.FLAG_SOLID,
            content = Tilesets.pebble.floor
          },
          perimeter_operator = {
            matches_all = SourceMap.FLAG_SOLID,
            add = {
              SourceMap.FLAG_SOLID,
              SourceMap.FLAG_TUNNEL,
              SourceMap.FLAG_PERIMETER
            },
            remove = SourceMap.FLAG_SEETHROUGH,
            content = Tilesets.pebble.wall
          },
          perimeter_width = 1,
          max_length = (tries_without_tunnel % 10 + 1) * 10,
          size_range = {
            1,
            1
          },
          tunnels_per_room_range = {
            1,
            1
          }
        })
        if success then
          tries_without_tunnel = 0
          break
        else
          tries_without_tunnel = tries_without_tunnel + 1
        end
      end
    end
  end
  return true
end
local node_paint_group
node_paint_group = function(self)
  self:apply({
    operator = {
      group = self.group
    }
  })
  local matches_group = {
    self.group,
    self.group
  }
  self.region_set = selector_map(self.region_set, function(s)
    return table.merge(s, {
      matches_group = matches_group
    })
  end)
  return true
end
local _load_map_poly = memoized(function(name)
  local json = require("json")
  local success, data = json.parse(file_as_string("map_descs/" .. tostring(name) .. ".json"))
  assert(success)
  local parts = { }
  local _list_0 = data.layers
  for _index_0 = 1, #_list_0 do
    local _des_0 = _list_0[_index_0]
    local objects
    objects = _des_0.objects
    for idx, _des_1 in ipairs((objects or { })) do
      local _continue_0 = false
      repeat
        local x, y, polygon
        x, y, name, polygon = _des_1.x, _des_1.y, _des_1.name, _des_1.polygon
        local ox, oy = x, y
        if not polygon then
          _continue_0 = true
          break
        end
        local tuple_format_polygon
        do
          local _accum_0 = { }
          local _len_0 = 1
          for _index_1 = 1, #polygon do
            local _des_2 = polygon[_index_1]
            x, y = _des_2.x, _des_2.y
            _accum_0[_len_0] = {
              x + ox,
              y + oy
            }
            _len_0 = _len_0 + 1
          end
          tuple_format_polygon = _accum_0
        end
        local convex_polys = PolyPartition.decompose(tuple_format_polygon)
        if name == "" then
          error("Need to specify name!")
          name = tostring(idx)
        end
        parts[name] = MapRegion.create(convex_polys)
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end
  return parts
end)
local resolve_parts
resolve_parts = function(rng, parts)
  local possibilities = { }
  for name, part in spairs(parts) do
    local prefix, idx
    do
      local _obj_0 = name:split("_")
      prefix, idx = _obj_0[1], _obj_0[2]
    end
    possibilities[prefix] = possibilities[prefix] or { }
    append(possibilities[prefix], part)
  end
  for k, choices in spairs(possibilities) do
    possibilities[k] = rng:random_choice(choices)
  end
  return possibilities
end
local load_map_polys
load_map_polys = function(rng, name, nx, ny, nw, nh, angle)
  local parts = table.deep_clone(_load_map_poly(name))
  parts = resolve_parts(rng, parts)
  local full_map = combine_map_regions((function()
    local _accum_0 = { }
    local _len_0 = 1
    for _, region in spairs(parts) do
      _accum_0[_len_0] = region
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)())
  local bbox = full_map:bbox()
  local _list_0 = full_map.polygons
  for _index_0 = 1, #_list_0 do
    local points = _list_0[_index_0]
    for i = 1, #points do
      points[i] = MapRegionShapes.transform_point(points[i], nx, ny, nw, nh, bbox)
    end
  end
  full_map:rotate(angle, {
    nx,
    ny
  })
  return parts
end
return nilprotect({
  load_map_polys = load_map_polys,
  node_paint_group = node_paint_group,
  node_connect_rect_rooms = node_connect_rect_rooms
})
