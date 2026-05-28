local map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator
do
  local _obj_0 = require("maps.GenerateUtils")
  map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator = _obj_0.map_place_object, _obj_0.ellipse_points, _obj_0.LEVEL_PADDING, _obj_0.Region, _obj_0.RVORegionPlacer, _obj_0.random_rect_in_rect, _obj_0.random_ellipse_in_ellipse, _obj_0.ring_region_delta_func, _obj_0.default_region_delta_func, _obj_0.spread_region_delta_func, _obj_0.center_region_delta_func, _obj_0.towards_region_delta_func, _obj_0.random_region_add, _obj_0.subregion_minimum_spanning_tree, _obj_0.region_minimum_spanning_tree, _obj_0.Tile, _obj_0.tile_operator
end
local NewMaps = require("maps.NewMaps")
local TileSets = require("tiles.Tilesets")
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
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local DEFAULT_DUNGEON_VISION_RADIUS = 7
local M = nilprotect({ })
M.create_dungeon_scheme = function(tileset)
  return {
    tileset = tileset,
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
end
M.dungeon_defaults = function(rng)
  return {
    rvo_iterations = 20,
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
    n_statues = 0
  }
end
M.place_feature = function(map, template, region_filter)
  if region_filter == nil then
    region_filter = function()
      return true
    end
  end
  local attempt_placement
  attempt_placement = function(template)
    local orient = map.rng:random_choice({
      SourceMap.ORIENT_DEFAULT,
      SourceMap.ORIENT_FLIP_X,
      SourceMap.ORIENT_FLIP_Y,
      SourceMap.ORIENT_TURN_90,
      SourceMap.ORIENT_TURN_180,
      SourceMap.ORIENT_TURN_270
    })
    local _list_0 = map.regions
    for _index_0 = 1, #_list_0 do
      local _continue_0 = false
      repeat
        local r = _list_0[_index_0]
        if not region_filter(r) then
          _continue_0 = true
          break
        end
        local w, h
        do
          local _obj_0 = template.size
          w, h = _obj_0[1], _obj_0[2]
        end
        if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180 then
          w, h = h, w
        end
        local x1, y1, x2, y2
        do
          local _obj_0 = r:bbox()
          x1, y1, x2, y2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
        end
        x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
        x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
        local top_left_xy = MapUtils.random_square(map, {
          x1,
          y1,
          x2,
          y2
        })
        local apply_args = {
          map = map,
          top_left_xy = top_left_xy,
          orientation = orient
        }
        if template:matches(apply_args) then
          template:apply(apply_args)
          return true
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return false
  end
  local attempt_placement_n_times
  attempt_placement_n_times = function(template, n)
    for i = 1, n do
      if attempt_placement(template) then
        return true
      end
    end
    return false
  end
  if attempt_placement_n_times(template, 100) then
    return true
  end
  return false
end
M._runed_door_sprite = tosprite("spr_doors.runed_door")
M.make_dungeon_template = function(data)
  return table.merge({
    outer_conf = data.subtemplates[1],
    shell = 40,
    default_wall = Tile.create(data.tileset.wall, true, true, { }),
    post_pone = function(self, f)
      return append(self._post_poned, f)
    end,
    _post_poned = { },
    _vision_radius = DEFAULT_DUNGEON_VISION_RADIUS,
    _player_spawn_points = nil,
    _base_vault_config = function(self, args)
      if args == nil then
        args = { }
      end
      local enemy_placer
      enemy_placer = function(map, xy)
        local enemy = OldMaps.enemy_generate(self:_enemy_entries())
        return MapUtils.spawn_enemy(map, enemy, xy)
      end
      local door_placer
      door_placer = function(map, xy)
        return MapUtils.spawn_door(map, xy, nil, args.door_sprite, args.door_key)
      end
      local store_placer
      store_placer = function(map, xy)
        return Region1.generate_store(map, xy)
      end
      local item_placer
      item_placer = function(map, xy)
        local item = ItemUtils.randart_generate(1)
        return MapUtils.spawn_item(map, item.type, item.amount, xy)
      end
      return {
        enemy_placer = enemy_placer,
        door_placer = door_placer,
        store_placer = store_placer,
        item_placer = item_placer,
        tileset = self.tileset
      }
    end,
    _default_vault_config = function(self, args)
      if args == nil then
        args = { }
      end
      return self:_base_vault_config(args)
    end,
    _spawn_enemies = function(self, map)
      local area = {
        0,
        0,
        map.size[1],
        map.size[2]
      }
      OldMaps.generate_from_enemy_entries(map, self:_enemy_entries(), self:_n_enemies(), area, {
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
      local _list_0 = self:_item_groups()
      for _index_0 = 1, #_list_0 do
        local group = _list_0[_index_0]
        for i = 1, OldMaps.adjusted_item_amount(group[2]) do
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
          local item = ItemUtils.item_generate(group[1], 1, (group[3] or 2))
          MapUtils.spawn_item(map, item.type, item.amount, sqr)
        end
      end
      return true
    end,
    _recalculate_perimeter = function(self, map)
      SourceMap.perimeter_apply({
        map = map,
        candidate_selector = {
          matches_all = SourceMap.FLAG_SOLID
        },
        inner_selector = {
          matches_none = SourceMap.FLAG_SOLID
        },
        operator = {
          add = SourceMap.FLAG_PERIMETER
        }
      })
      return SourceMap.perimeter_apply({
        map = map,
        candidate_selector = {
          matches_none = {
            SourceMap.FLAG_SOLID
          }
        },
        inner_selector = {
          matches_all = {
            SourceMap.FLAG_PERIMETER,
            SourceMap.FLAG_SOLID
          }
        },
        operator = {
          add = FLAG_INNER_PERIMETER
        }
      })
    end,
    _spawn_stores = function(self, map)
      local _list_0 = map.regions
      for _index_0 = 1, #_list_0 do
        local region = _list_0[_index_0]
        local area = region:bbox()
        for i = 1, region.conf.n_stores or 0 do
          local sqr = MapUtils.random_square(map, area, {
            matches_none = {
              SourceMap.FLAG_HAS_OBJECT,
              Vaults.FLAG_HAS_VAULT,
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
          Region1.generate_store(map, sqr)
        end
        for i = 1, region.conf.n_epic_stores or 0 do
          local sqr = MapUtils.random_square(map, area, {
            matches_none = {
              SourceMap.FLAG_HAS_OBJECT,
              Vaults.FLAG_HAS_VAULT,
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
          Region1.generate_epic_store(map, sqr)
        end
      end
      return true
    end,
    _spawn_statues = function(self, map)
      self:_recalculate_perimeter(map)
      local _list_0 = map.regions
      for _index_0 = 1, #_list_0 do
        local region = _list_0[_index_0]
        for i = 1, region.conf.n_statues do
          local area = region:bbox()
          local sqr = MapUtils.random_square(map, area, {
            matches_none = {
              FLAG_INNER_PERIMETER,
              SourceMap.FLAG_HAS_OBJECT,
              Vaults.FLAG_HAS_VAULT,
              SourceMap.FLAG_SOLID
            }
          })
          if not sqr then
            break
          end
          map:square_apply(sqr, {
            add = {
              SourceMap.FLAG_SOLID,
              SourceMap.FLAG_HAS_OBJECT
            },
            remove = SourceMap.FLAG_SEETHROUGH
          })
          MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0, 17))
        end
      end
      return true
    end,
    _spawn_healing_squares = function(self, map)
      local conf = data.subtemplates[1]
      for i = 1, conf.n_healing_squares or 0 do
        local region = random_choice(map.regions)
        local area = region:bbox()
        local sqr = MapUtils.random_square(map, area, {
          matches_none = {
            FLAG_INNER_PERIMETER,
            SourceMap.FLAG_HAS_OBJECT,
            Vaults.FLAG_HAS_VAULT,
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
        MapUtils.spawn_healing_square(map, sqr)
      end
      return true
    end,
    _create_encounter_rooms = function(self, map)
      local default_config = self:_default_vault_config({
        door_sprite = M._runed_door_sprite,
        door_key = "dummykey"
      })
      for i = 1, self:_n_encounter_vaults() do
        local conf = table.merge(default_config, {
          rng = map.rng
        })
        local vault = SourceMap.area_template_create(self:_encounter_vault()(conf))
        if not M.place_feature(map, vault) then
          return nil
        end
      end
      return true
    end,
    on_create_game_map = function(self, game_map)
      local _list_0 = self._post_poned
      for _index_0 = 1, #_list_0 do
        local f = _list_0[_index_0]
        f(game_map)
      end
      Map.set_vision_radius(game_map, self._vision_radius)
      if self._player_spawn_points then
        World.players_spawn(game_map, self._player_spawn_points)
      end
      return self.connector.post_connect(game_map)
    end
  }, data)
end
return M
