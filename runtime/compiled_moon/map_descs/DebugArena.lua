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
local MapNode, MapDesc
do
  local _obj_0 = require("maps.MapDesc")
  MapNode, MapDesc = _obj_0.MapNode, _obj_0.MapDesc
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
local Places = require("maps.Places")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local center, find_bbox, find_square, selector_filter, selector_map
do
  local _obj_0 = require("maps.MapRegionUtils")
  center, find_bbox, find_square, selector_filter, selector_map = _obj_0.center, _obj_0.find_bbox, _obj_0.find_square, _obj_0.selector_filter, _obj_0.selector_map
end
local PADDING = 10
local event_log
event_log = function(...)
  return print(string.format(...))
end
local shrink
shrink = function(bbox, x, y)
  local x1, y1, x2, y2
  x1, y1, x2, y2 = bbox[1], bbox[2], bbox[3], bbox[4]
  return {
    x1 + x,
    y1 + y,
    x2 - x,
    y2 - y
  }
end
local node_place_easy_overworld_rooms
node_place_easy_overworld_rooms = function(self)
  local n_rooms = 1
  event_log("(RNG #%d) generating %d rooms", self.rng:amount_generated(), n_rooms)
  local regions = { }
  for i = 1, n_rooms do
    local size = self.desc.room_size or {
      10,
      10
    }
    local result = find_bbox(self.region_set, size)
    if not result then
      return false
    end
    local bbox
    bbox = result.bbox
    local group = self:new_group()
    local region = from_bbox(unpack(bbox))
    region.inner_bbox = shrink(bbox, 1, 1)
    region.selector = {
      matches_group = {
        group,
        group
      }
    }
    append(regions, region)
    SourceMap.rectangle_apply({
      map = self.map,
      area = bbox,
      fill_operator = {
        group = group,
        add = SourceMap.FLAG_SEETHROUGH,
        remove = SourceMap.FLAG_SOLID,
        content = Tilesets.pebble.floor
      },
      perimeter_width = 0,
      perimeter_operator = {
        add = {
          SourceMap.FLAG_SOLID,
          SourceMap.FLAG_PERIMETER
        },
        remove = SourceMap.FLAG_SEETHROUGH,
        content = Tilesets.pebble.wall
      }
    })
    local _list_0 = self.desc.monster_spawns
    for _index_0 = 1, #_list_0 do
      local type = _list_0[_index_0]
      local enemy = MapUtils.random_enemy(self.map, type, region:bbox(), region.selector)
      if not enemy then
        return false
      end
    end
  end
  event_log("(RNG #%d) after generating %d rooms", self.rng:amount_generated(), n_rooms)
  self.region_set = {
    map = self.map,
    regions = regions
  }
  return true
end
local DebugArena = MapDesc.create({
  size = {
    20,
    20
  },
  children = {
    MapNode.create({
      place = node_place_easy_overworld_rooms
    })
  }
})
local MAX_MAP_TRIES = 100
local generate
generate = function(portal_spawns, monster_spawns, options)
  if options == nil then
    options = { }
  end
  DebugArena.monster_spawns = monster_spawns
  DebugArena.room_size = options.room_size or {
    10,
    10
  }
  local map = NewMaps.try_n_times(MAX_MAP_TRIES, function()
    return DebugArena:compile(portal_spawns)
  end)
  local game_map = NewMaps.generate_game_map(map)
  game_map.content_lab_spawn_bounds = {
    Map.TILE_SIZE,
    Map.TILE_SIZE,
    map.size[1] * Map.TILE_SIZE - Map.TILE_SIZE,
    map.size[2] * Map.TILE_SIZE - Map.TILE_SIZE
  }
  local x, y
  do
    local _obj_0 = MapUtils.random_square(map, nil, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    x, y = _obj_0[1], _obj_0[2]
  end
  append(map.player_candidate_squares, {
    x * 32 + 16,
    y * 32 + 16
  })
  local player_spawn_points = MapUtils.pick_player_squares(map, map.player_candidate_squares)
  local _list_0 = map.post_game_map
  for _index_0 = 1, #_list_0 do
    local post_poned = _list_0[_index_0]
    post_poned(game_map)
  end
  return game_map, player_spawn_points
end
DebugArena.generate_and_spawn = function(self, monster_spawns, options)
  if options == nil then
    options = { }
  end
  local game_map, player_spawn_points = generate({ }, monster_spawns, options)
  if #player_spawn_points > 0 then
    center = {
      0,
      0
    }
    for _index_0 = 1, #player_spawn_points do
      local point = player_spawn_points[_index_0]
      center[1] = center[1] + point[1]
      center[2] = center[2] + point[2]
    end
    center[1] = center[1] / #player_spawn_points
    center[2] = center[2] / #player_spawn_points
    game_map.content_lab_spawn_center = center
  end
  World.players_spawn(game_map, player_spawn_points)
  return game_map
end
return DebugArena
