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
local NewDungeons = require("maps.NewDungeons")
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
local place_vault
place_vault = require("maps.VaultUtils").place_vault
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
local DUNGEON_CONF
DUNGEON_CONF = function(rng, tileset, schema, n_regions)
  if tileset == nil then
    tileset = Tilesets.pebble
  end
  if schema == nil then
    schema = 1
  end
  if n_regions == nil then
    n_regions = nil
  end
  local C = {
    floor1 = Tile.create(tileset.floor, false, true, { }, {
      FLAG_OVERWORLD
    }),
    floor2 = Tile.create(tileset.floor_alt, false, true, { }, {
      FLAG_OVERWORLD
    }),
    wall1 = Tile.create(tileset.wall, true, false, { }, {
      FLAG_OVERWORLD
    }),
    wall2 = Tile.create(tileset.wall_alt, true, false, { }, {
      FLAG_OVERWORLD
    })
  }
  local _exp_0 = schema
  if 3 == _exp_0 then
    C.number_regions = n_regions or rng:random(15, 20)
    C.room_radius = function()
      local r = 8
      for j = 1, rng:random(5, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      2,
      2
    }
    C.rect_room_size_range = {
      7,
      15
    }
  elseif 4 == _exp_0 then
    C.number_regions = n_regions or rng:random(13, 18)
    C.room_radius = function()
      local r = 5
      for j = 1, rng:random(5, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      10,
      10
    }
    C.rect_room_size_range = {
      7,
      15
    }
  elseif 0 == _exp_0 then
    C.number_regions = n_regions or rng:random(15, 20)
    C.room_radius = function()
      local r = 4
      for j = 1, rng:random(5, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      10,
      10
    }
    C.rect_room_size_range = {
      7,
      15
    }
  elseif 1 == _exp_0 then
    C.number_regions = n_regions or rng:random(15, 20)
    C.room_radius = function()
      local r = 2
      for j = 1, rng:random(0, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      2,
      10
    }
    C.rect_room_size_range = {
      7,
      15
    }
  elseif 2 == _exp_0 then
    C.number_regions = n_regions or rng:random(2, 7)
    C.room_radius = function()
      local r = 2
      for j = 1, rng:random(0, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      10,
      15
    }
    C.rect_room_size_range = {
      7,
      15
    }
  end
  return table.merge(C, {
    size = (function()
      if schema == 4 then
        return {
          85,
          85
        }
      else
        return {
          65,
          65
        }
      end
    end)(),
    rvo_iterations = 20,
    n_stairs_down = 3,
    n_stairs_up = 0,
    connect_line_width = function()
      return 2 + ((function()
        if rng:random(5) == 4 then
          return 1
        else
          return 0
        end
      end)())
    end,
    region_delta_func = default_region_delta_func,
    n_statues = 4
  })
end
local tileset = Tilesets.hell
local HELL_ENEMIES = {
  {
    enemy = "Hell Storm",
    chance = 8
  },
  {
    enemy = "Centaur Marksman",
    chance = 4
  },
  {
    enemy = "Hell Forged",
    chance = 12
  },
  {
    enemy = "Executioner",
    chance = 36,
    guaranteed_spawns = 1
  },
  {
    enemy = "Mana Sapper",
    chance = 6
  },
  {
    enemy = "Stara",
    guaranteed_spawns = 1
  }
}
local make_template
make_template = function(rng, back_links, forward_links)
  return {
    map_label = "Hell",
    subtemplates = {
      DUNGEON_CONF(rng, tileset, 3)
    },
    w = 200,
    h = 200,
    seethrough = false,
    outer_conf = DUNGEON_CONF(rng, tileset),
    shell = 10,
    default_wall = Tile.create(tileset.wall, true, true, { }),
    _create_stairs_up = function(self, map)
      local _list_0 = table.tconcat(back_links, forward_links)
      for _index_0 = 1, #_list_0 do
        local link = _list_0[_index_0]
        if not place_vault(map, Vaults.hell_entrance_vault({
          rng = map.rng,
          item_placer = link,
          door_placer = function(map, xy)
            return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
          end,
          tileset = tileset
        })) then
          return false
        end
      end
      return true
    end,
    _spawn_enemies = function(self, map)
      local area = {
        0,
        0,
        map.size[1],
        map.size[2]
      }
      OldMaps.generate_from_enemy_entries(map, HELL_ENEMIES, 2, area, {
        matches_none = {
          SourceMap.FLAG_SOLID,
          Vaults.FLAG_HAS_VAULT,
          FLAG_NO_ENEMY_SPAWN
        }
      })
      return true
    end,
    _spawn_items = function(self, map)
      local area = {
        0,
        0,
        map.size[1],
        map.size[2]
      }
      local _list_0 = {
        {
          ItemGroups.basic_items,
          20
        },
        {
          ItemGroups.enchanted_items,
          10
        },
        {
          {
            item = "Scroll of Experience",
            chance = 100
          },
          2
        }
      }
      for _index_0 = 1, #_list_0 do
        local group = _list_0[_index_0]
        for i = 1, group[2] do
          local sqr = MapUtils.random_square(map, area, {
            matches_none = {
              FLAG_INNER_PERIMETER,
              SourceMap.FLAG_HAS_OBJECT,
              SourceMap.FLAG_SOLID
            }
          })
          if not sqr then
            break
          end
          map:square_apply(sqr, {
            add = {
              SourceMap.FLAG_HAS_OBJECT
            }
          })
          local item = ItemUtils.item_generate(group[1])
          MapUtils.spawn_item(map, item.type, item.amount, sqr)
        end
      end
      for i = 1, OldMaps.adjusted_item_amount(8) do
        local sqr = MapUtils.random_square(map, area, {
          matches_none = {
            FLAG_INNER_PERIMETER,
            SourceMap.FLAG_HAS_OBJECT,
            SourceMap.FLAG_SOLID
          }
        })
        if not sqr then
          break
        end
        map:square_apply(sqr, {
          add = {
            SourceMap.FLAG_HAS_OBJECT
          }
        })
        local item = ItemUtils.randart_generate(1)
        MapUtils.spawn_item(map, item.type, item.amount, sqr)
      end
      for i = 1, OldMaps.adjusted_item_amount(2) do
        local sqr = MapUtils.random_square(map, area, {
          matches_none = {
            FLAG_INNER_PERIMETER,
            SourceMap.FLAG_HAS_OBJECT,
            SourceMap.FLAG_SOLID
          }
        })
        if not sqr then
          break
        end
        map:square_apply(sqr, {
          add = {
            SourceMap.FLAG_HAS_OBJECT
          }
        })
        local item = ItemUtils.randart_generate(2)
        MapUtils.spawn_item(map, item.type, item.amount, sqr)
      end
      return true
    end,
    on_create_source_map = function(self, map)
      if not self:_create_stairs_up(map) then
        return nil
      end
      if not self:_spawn_items(map) then
        return nil
      end
      if not self:_spawn_enemies(map) then
        return nil
      end
      NewMaps.generate_door_candidates(map, rng, map.regions)
      return true
    end
  }
end
return MapDesc.create({
  map_label = "Hell",
  size = {
    200,
    200
  },
  default_content = Tilesets.hive.wall,
  children = {
    MapNode.create({
      place = function(self)
        local template = make_template(self.map.rng, self.desc.back_links, self.desc.forward_links)
        if not NewMaps.map_try_create(self.map, self.map.rng, template) then
          return false
        end
        if not template:on_create_source_map(self.map) then
          return false
        end
        return true
      end
    })
  }
})
