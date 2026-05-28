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
local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper = MapUtils.make_tunnel_oper, MapUtils.make_rectangle_criteria, MapUtils.make_rectangle_oper
local Vaults = require("maps.Vaults")
local World = require("core.World")
local SourceMap = require("core.SourceMap")
local place_vault_in
place_vault_in = require("maps.VaultUtils").place_vault_in
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
local center, bbox, find_bbox, find_square, selector_filter, selector_map
do
  local _obj_0 = require("maps.MapRegionUtils")
  center, bbox, find_bbox, find_square, selector_filter, selector_map = _obj_0.center, _obj_0.bbox, _obj_0.find_bbox, _obj_0.find_square, _obj_0.selector_filter, _obj_0.selector_map
end
local load_map_polys, node_paint_group
do
  local _obj_0 = require("maps.MapNodeFills")
  load_map_polys, node_paint_group = _obj_0.load_map_polys, _obj_0.node_paint_group
end
local node_dungeon_room_template
node_dungeon_room_template = function(self)
  local area_temp = SourceMap.area_template_create({
    data_file = self.map.rng:random_choice({
      "maps/dungeon1room1a.txt",
      "maps/dungeon1room1b.txt",
      "maps/dungeon1room1c.txt"
    }),
    legend = {
      ['x'] = {
        add = SourceMap.FLAG_SOLID,
        content = Tilesets.pixulloch.wall
      },
      ['.'] = {
        add = SourceMap.FLAG_SEETHROUGH,
        content = (function()
          if chance(.5) then
            return Tilesets.pixulloch.floor
          else
            return Tilesets.pixulloch.floor_alt
          end
        end)()
      }
    }
  })
  local orientation = self.map.rng:random_choice({
    SourceMap.ORIENT_DEFAULT,
    SourceMap.ORIENT_FLIP_X,
    SourceMap.ORIENT_FLIP_Y,
    SourceMap.ORIENT_TURN_90,
    SourceMap.ORIENT_TURN_180,
    SourceMap.ORIENT_TURN_270
  })
  local cx, cy
  do
    local _obj_0 = center(self.region_set)
    cx, cy = _obj_0[1], _obj_0[2]
  end
  local w, h
  do
    local _obj_0 = area_temp.size
    w, h = _obj_0[1], _obj_0[2]
  end
  if orientation == SourceMap.ORIENT_TURN_90 or orientation == SourceMap.ORIENT_TURN_270 then
    w, h = h, w
  end
  area_temp:apply({
    map = self.map,
    top_left_xy = {
      cx - w / 2,
      cy - h / 2
    },
    orientation = orientation
  })
  return true
end
local node_fill_dungeon_room
node_fill_dungeon_room = function(self)
  local rbbox = bbox(self.region_set)
  local _list_0 = self.desc.back_links
  for _index_0 = 1, #_list_0 do
    local back_link = _list_0[_index_0]
    local xy = MapUtils.random_square(self.map, nil, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    if not xy then
      return false
    end
    back_link(self.map, xy)
  end
  local _list_1 = self.desc.forward_links
  for _index_0 = 1, #_list_1 do
    local forward_link = _list_1[_index_0]
    local xy = MapUtils.random_square(self.map, nil, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    if not xy then
      return false
    end
    forward_link(self.map, xy)
  end
  local spawn_enemies
  spawn_enemies = function(enemies)
    local selector = {
      matches_none = {
        SourceMap.FLAG_HAS_OBJECT,
        SourceMap.FLAG_SOLID
      }
    }
    for _index_0 = 1, #enemies do
      local type = enemies[_index_0]
      local enemy = MapUtils.random_enemy(self.map, type, rbbox, selector)
      if not enemy then
        return false
      end
    end
    return true
  end
  if not spawn_enemies({
    "Pixulloch"
  }) then
    return false
  end
  return true
end
return MapDesc.create({
  map_label = "Pixullochia Depths",
  size = {
    130,
    130
  },
  default_content = Tilesets.pixulloch.wall,
  children = {
    MapNode.create({
      place = node_dungeon_room_template,
      children = {
        MapNode.create({
          place = node_fill_dungeon_room
        })
      }
    })
  }
})
