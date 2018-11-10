local map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, rectangle_points, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator
do
  local _obj_0 = require("maps.GenerateUtils")
  map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, rectangle_points, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator = _obj_0.map_place_object, _obj_0.ellipse_points, _obj_0.LEVEL_PADDING, _obj_0.Region, _obj_0.RVORegionPlacer, _obj_0.random_rect_in_rect, _obj_0.random_ellipse_in_ellipse, _obj_0.ring_region_delta_func, _obj_0.default_region_delta_func, _obj_0.spread_region_delta_func, _obj_0.center_region_delta_func, _obj_0.towards_region_delta_func, _obj_0.rectangle_points, _obj_0.random_region_add, _obj_0.subregion_minimum_spanning_tree, _obj_0.region_minimum_spanning_tree, _obj_0.Tile, _obj_0.tile_operator
end
local MapRegion
MapRegion = require("maps.MapRegion").MapRegion
local NewMaps = require("maps.NewMaps")
local NewDungeons = require("maps.NewDungeons")
local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper = MapUtils.make_tunnel_oper, MapUtils.make_rectangle_criteria, MapUtils.make_rectangle_oper
local MapSequence = require("maps.MapSequence")
local MapLinker
MapLinker = require("maps.MapLink").MapLinker
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
local M = nilprotect({ })
local OVERWORLD_VISION_RADIUS = 8
local underdungeon_create
local create_overworld_scheme
create_overworld_scheme = function(tileset)
  return nilprotect({
    floor1 = Tile.create(tileset.floor, false, true, {
      FLAG_OVERWORLD
    }),
    floor2 = Tile.create(tileset.floor_alt, false, true, {
      FLAG_OVERWORLD
    }),
    wall1 = Tile.create(tileset.wall, true, true, {
      FLAG_OVERWORLD
    }),
    wall2 = Tile.create(tileset.wall_alt, true, false)
  })
end
local create_dungeon_scheme
create_dungeon_scheme = function(tileset)
  return nilprotect({
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
  })
end
local OVERWORLD_TILESET = create_overworld_scheme(Tilesets.grass)
local DUNGEON_TILESET = create_dungeon_scheme(Tilesets.pebble)
local OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE = 300, 300
local TEST_CONF = {
  map_label = os.getenv("LANARTS_TESTCASE"),
  is_overworld = true,
  size = {
    100,
    100
  },
  number_regions = 0,
  floor1 = OVERWORLD_TILESET.floor1,
  floor2 = OVERWORLD_TILESET.floor2,
  wall1 = OVERWORLD_TILESET.wall1,
  wall2 = OVERWORLD_TILESET.wall2,
  rect_room_num_range = {
    0,
    0
  },
  rect_room_size_range = {
    10,
    15
  },
  rvo_iterations = 0,
  n_stairs_down = 0,
  n_stairs_up = 0,
  connect_line_width = function()
    return 1
  end,
  region_delta_func = spread_region_delta_func,
  room_radius = 1,
  n_statues = 0
}
local OVERWORLD_CONF
OVERWORLD_CONF = function(rng)
  local type = rng:random_choice({
    {
      number_regions = rng:random(25, 30),
      region_delta_func = ring_region_delta_func,
      room_radius = function()
        local r = 7
        local bound = rng:random(1, 20)
        for j = 1, rng:random(0, bound) do
          r = r + rng:randomf(0, 1)
        end
        return r
      end
    },
    {
      number_regions = 4,
      arc_chance = 1,
      region_delta_func = spread_region_delta_func,
      room_radius = function()
        return rng:random(10, 20)
      end
    },
    {
      number_regions = rng:random(15, 22),
      region_delta_func = ring_region_delta_func,
      arc_chance = 1,
      room_radius = function()
        return rng:random(5, 10)
      end
    }
  })
  return {
    map_label = "Plain Valley",
    is_overworld = true,
    size = (function()
      if rng:random(0, 2) == 0 then
        return {
          85,
          105
        }
      else
        return {
          85,
          105
        }
      end
    end)(),
    number_regions = type.number_regions,
    floor1 = OVERWORLD_TILESET.floor1,
    floor2 = OVERWORLD_TILESET.floor2,
    wall1 = OVERWORLD_TILESET.wall1,
    wall2 = OVERWORLD_TILESET.wall2,
    rect_room_num_range = {
      0,
      0
    },
    rect_room_size_range = {
      10,
      15
    },
    rvo_iterations = 150,
    n_stairs_down = 0,
    n_stairs_up = 0,
    connect_line_width = function()
      return rng:random(2, 6)
    end,
    region_delta_func = type.region_delta_func,
    arc_chance = type.arc_chance or 0,
    room_radius = type.room_radius,
    n_statues = 4
  }
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
  local C = create_dungeon_scheme(tileset)
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
    map_label = "A Dungeon",
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
local place_doors_and_statues
place_doors_and_statues = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
    local conf = region.conf
    local _list_0 = SourceMap.rectangle_match({
      map = map,
      selector = {
        matches_none = {
          SourceMap.FLAG_HAS_OBJECT,
          SourceMap.FLAG_SOLID
        },
        matches_all = {
          FLAG_DOOR_CANDIDATE
        }
      }
    })
    for _index_1 = 1, #_list_0 do
      local xy = _list_0[_index_1]
      MapUtils.spawn_door(map, xy)
    end
    for i = 1, conf.n_statues do
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
end
local overdungeon_items_and_enemies
overdungeon_items_and_enemies = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
    local conf = region.conf
    for i = 1, OldMaps.adjusted_item_amount(10) do
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
      local item = ItemUtils.item_generate(ItemGroups.basic_items)
      MapUtils.spawn_item(map, item.type, item.amount, sqr)
    end
    OldMaps.generate_from_enemy_entries(map, OldMaps.hard_enemies, 10, area, {
      matches_none = {
        SourceMap.FLAG_SOLID,
        Vaults.FLAG_HAS_VAULT,
        FLAG_NO_ENEMY_SPAWN
      }
    })
    OldMaps.generate_from_enemy_entries(map, OldMaps.fast_enemies, 10, area, {
      matches_none = {
        SourceMap.FLAG_SOLID,
        Vaults.FLAG_HAS_VAULT,
        FLAG_NO_ENEMY_SPAWN
      }
    })
  end
end
local overworld_items_and_enemies
overworld_items_and_enemies = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
    local conf = region.conf
    for i = 1, OldMaps.adjusted_item_amount(10) do
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
      local item = ItemUtils.item_generate(ItemGroups.basic_items)
      MapUtils.spawn_item(map, item.type, item.amount, sqr)
    end
    OldMaps.generate_from_enemy_entries(map, OldMaps.medium_animals, 8, area, {
      matches_none = {
        SourceMap.FLAG_SOLID,
        Vaults.FLAG_HAS_VAULT,
        FLAG_NO_ENEMY_SPAWN
      }
    })
  end
end
local place_feature
place_feature = function(map, template, regions)
  if regions == nil then
    regions = map.regions
  end
  event_log("(RNG #%d) placing feature", map.rng:amount_generated())
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
    event_log("(RNG #%d) orient=%d", map.rng:amount_generated(), orient)
    for _index_0 = 1, #regions do
      local r = regions[_index_0]
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
    end
    return false
  end
  local attempt_placement_n_times
  attempt_placement_n_times = function(template, n)
    for i = 1, n do
      event_log("(RNG #%d) placement=%d", map.rng:amount_generated(), i)
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
local safe_portal_spawner
safe_portal_spawner = function(tileset)
  return function(map, map_area, sprite, callback, frame)
    local portal_holder = { }
    local portal_placer
    portal_placer = function(map, xy)
      portal_holder[1] = MapUtils.spawn_portal(map, xy, sprite, callback, frame)
    end
    local vault = SourceMap.area_template_create(Vaults.small_item_vault_multiway({
      rng = map.rng,
      item_placer = portal_placer,
      tileset = tileset
    }))
    if not place_feature(map, vault) then
      return MapUtils.random_portal(map, map_area, sprite, callback, frame)
    end
    assert(portal_holder[1])
    return portal_holder[1]
  end
end
local hell_create
hell_create = function(MapSeq, seq_idx, number_entrances)
  if number_entrances == nil then
    number_entrances = 1
  end
  local tileset = Tilesets.hell
  return NewMaps.map_create(function(rng)
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
      post_poned = { },
      _create_stairs_up = function(self, map)
        local door_placer
        door_placer = function(map, xy)
          return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
        end
        for i = 1, 3 do
          local up_stairs_placer
          up_stairs_placer = function(map, xy)
            local portal = MapUtils.spawn_portal(map, xy, "spr_gates.return_hell")
            return MapSeq:backward_portal_resolve(seq_idx, portal, i)
          end
          local vault = SourceMap.area_template_create(Vaults.hell_entrance_vault({
            rng = map.rng,
            item_placer = up_stairs_placer,
            door_placer = door_placer,
            tileset = tileset
          }))
          if not place_feature(map, vault) then
            return nil
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
        OldMaps.generate_from_enemy_entries(map, OldMaps.strong_hell, 25, area, {
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
      end,
      on_create_game_map = function(self, game_map)
        local _list_0 = self.post_poned
        for _index_0 = 1, #_list_0 do
          local f = _list_0[_index_0]
          f(game_map)
        end
        return Map.set_vision_radius(game_map, 6)
      end
    }
  end)
end
M.hive_create = function(MapSeq)
  local offset = 1
  local dungeon = require("maps.Hive")
  return NewDungeons.make_linear_dungeon({
    MapSeq = MapSeq,
    offset = offset,
    dungeon_template = dungeon.TEMPLATE,
    on_generate = function(floor)
      assert(floor)
      return log_verbose("on_generate", floor)
    end,
    sprite_up = function(floor)
      if floor == 1 then
        return "spr_gates.exit_lair"
      else
        return "spr_gates.return"
      end
    end,
    sprite_down = function(floor)
      return "spr_gates.enter"
    end,
    portals_up = function(floor)
      return 3
    end,
    portals_down = function(floor)
      if floor == dungeon.N_FLOORS then
        return 0
      else
        return 3
      end
    end
  })
end
M.crypt_create = function(MapSeq, seq_idx, number_entrances)
  if number_entrances == nil then
    number_entrances = 1
  end
  local tileset = Tilesets.crypt
  return NewMaps.map_create(function(rng)
    return {
      map_label = "Crypt",
      w = 110,
      h = 100,
      seethrough = false,
      outer_conf = DUNGEON_CONF(rng, Tilesets.crypt, nil, 10),
      subtemplates = {
        DUNGEON_CONF(rng, Tilesets.crypt, 4, 10)
      },
      shell = 10,
      default_wall = Tile.create(Tilesets.crypt.wall, true, true, { }),
      post_poned = { },
      _create_encounter_rooms = function(self, map)
        local enemy_placer
        enemy_placer = function(map, xy)
          local enemy = OldMaps.enemy_generate(OldMaps.weak_undead)
          return MapUtils.spawn_enemy(map, enemy, xy)
        end
        local door_placer
        door_placer = function(map, xy)
          return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
        end
        local store_placer
        store_placer = function(map, xy)
          return Region1.generate_epic_store(map, xy)
        end
        local _items_placed = 0
        local item_placer
        item_placer = function(map, xy)
          local item
          if _items_placed >= 1 then
            item = ItemUtils.item_generate(ItemGroups.enchanted_items)
          else
            item = {
              type = "Tomb Lanart",
              amount = 1
            }
          end
          MapUtils.spawn_item(map, item.type, item.amount, xy)
          _items_placed = _items_placed + 1
        end
        for i = 1, 2 do
          local up_stairs_placer
          up_stairs_placer = function(map, xy)
            return MapSeq:backward_portal_resolve(seq_idx, portal, i)
          end
          local vault = SourceMap.area_template_create(Vaults.crypt_encounter_vault({
            rng = map.rng,
            enemy_placer = enemy_placer,
            door_placer = door_placer,
            store_placer = store_placer,
            item_placer = item_placer,
            tileset = tileset
          }))
          if not place_feature(map, vault) then
            return nil
          end
        end
        return true
      end,
      _place_hell = function(self, map)
        local Seq = MapSequence.create({
          preallocate = 1
        })
        local door_placer
        door_placer = function(map, xy)
          return MapUtils.spawn_door(map, xy, nil, Vaults._door_magentite, "Magentite Key")
        end
        local next_dungeon = {
          1
        }
        local place_dungeon
        place_dungeon = function(map, xy)
          local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_hell1")
          local c = (Seq:forward_portal_add(1, portal, next_dungeon[1], function()
            return hell_create(Seq, 2)
          end))
          if World.player_amount > 1 then
            append(map.post_maps, c)
          end
          next_dungeon[1] = next_dungeon[1] + 1
        end
        local enemy_placer
        enemy_placer = function(map, xy)
          local enemy = OldMaps.enemy_generate(OldMaps.strong_undead)
          return MapUtils.spawn_enemy(map, enemy, xy)
        end
        local vault = SourceMap.area_template_create(Vaults.hell_dungeon({
          dungeon_placer = place_dungeon,
          tileset = Tilesets.hell,
          door_placer = door_placer,
          enemy_placer = enemy_placer,
          player_spawn_area = false
        }))
        if not place_feature(map, vault) then
          return nil
        end
        append(self.post_poned, function(game_map)
          return Seq:slot_resolve(1, game_map)
        end)
        return true
      end,
      _create_stairs_up = function(self, map)
        local door_placer
        door_placer = function(map, xy)
          return MapUtils.spawn_door(map, xy)
        end
        for i = 1, 3 do
          local up_stairs_placer
          up_stairs_placer = function(map, xy)
            local portal = MapUtils.spawn_portal(map, xy, "spr_gates.exit_crypt")
            return MapSeq:backward_portal_resolve(seq_idx, portal, i)
          end
          local vault = SourceMap.area_template_create(Vaults.crypt_entrance_vault({
            rng = map.rng,
            item_placer = up_stairs_placer,
            door_placer = door_placer,
            tileset = tileset
          }))
          if not place_feature(map, vault) then
            return nil
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
        OldMaps.generate_from_enemy_entries(map, OldMaps.strong_undead, 25, area, {
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
            ItemGroups.enchanted_items,
            5
          },
          {
            {
              item = "Scroll of Experience",
              chance = 100
            },
            1
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
        return true
      end,
      on_create_source_map = function(self, map)
        if not self:_place_hell(map) then
          return nil
        end
        if not self:_create_encounter_rooms(map) then
          return nil
        end
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
      end,
      on_create_game_map = function(self, game_map)
        local _list_0 = self.post_poned
        for _index_0 = 1, #_list_0 do
          local f = _list_0[_index_0]
          f(game_map)
        end
        return Map.set_vision_radius(game_map, 6)
      end
    }
  end)
end
local map_linker
map_linker = function(map, map_f)
  local MapSeq = MapSequence.create({
    preallocate = 1
  })
  append(map.post_game_map, function(game_map)
    return MapSeq:slot_resolve(1, game_map)
  end)
  local n_portals = 0
  return function(forward_portal)
    local generate
    generate = function()
      return map_f((function()
        local _accum_0 = { }
        local _len_0 = 1
        for i = 1, n_portals do
          _accum_0[_len_0] = function(back_portal)
            return MapSeq:backward_portal_resolve(2, back_portal, i)
          end
          _len_0 = _len_0 + 1
        end
        return _accum_0
      end)())
    end
    n_portals = n_portals + 1
    return MapSeq:forward_portal_add(1, forward_portal, n_portals, generate)
  end
end
local place_underdungeon_vault
place_underdungeon_vault = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local underdungeon = MapLinker.create({
    map_label = "Underdungeon",
    generate = function(self, backwards)
      return underdungeon_create(backwards)
    end
  })
  local vault = SourceMap.area_template_create(Vaults.sealed_dungeon({
    rng = map.rng,
    tileset = Tilesets.snake,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, 'Dandelite Key')
    end,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_vaults_open")
      return underdungeon:link_portal(portal, "spr_gates.exit_dungeon")
    end
  }))
  return place_feature(map, vault, regions)
end
local place_hive
place_hive = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local entrance = require("map_descs.HiveEntrance"):linker()
  local depths = require("map_descs.HiveDepths"):linker()
  for i = 1, 3 do
    entrance:link_linker(depths, "spr_gates.enter_lair", "spr_gates.exit_lair")
  end
  local vault = SourceMap.area_template_create(Vaults.sealed_dungeon({
    tileset = Tilesets.hive,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
    end,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.hive_portal")
      return entrance:link_portal(portal, "spr_gates.exit_dungeon")
    end
  }))
  if not place_feature(map, vault, regions) then
    return false
  end
  return true
end
local place_snake_pit
place_snake_pit = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local entrance = require("map_descs.SnakePitEntrance"):linker()
  local depths = require("map_descs.SnakePitDepths"):linker()
  for i = 1, 3 do
    entrance:link_linker(depths, "spr_gates.enter", "spr_gates.return")
  end
  local vault = SourceMap.area_template_create(Vaults.sealed_dungeon({
    tileset = Tilesets.snake,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy)
    end,
    dungeon_placer = callable_once(function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_snake")
      return entrance:link_portal(portal, "spr_gates.exit_dungeon")
    end),
    player_spawn_area = true
  }))
  if not place_feature(map, vault, regions) then
    return false
  end
  return true
end
local place_outpost
place_outpost = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local MapSeq = MapSequence.create({
    preallocate = 1
  })
  local InnerMapSeq = MapSequence.create({
    preallocate = 1
  })
  append(map.post_game_map, function(game_map)
    return MapSeq:slot_resolve(1, game_map)
  end)
  local inner_encounter_template = {
    {
      layout = {
        {
          size = {
            60,
            40
          },
          rooms = {
            padding = 0,
            amount = 40,
            size = {
              3,
              7
            }
          },
          tunnels = {
            padding = 0,
            width = {
              1,
              3
            },
            per_room = 5
          }
        }
      },
      content = {
        items = {
          amount = 8,
          group = ItemGroups.enchanted_items
        },
        enemies = {
          wandering = true,
          amount = 0,
          generated = {
            {
              enemy = "Ogre Mage",
              guaranteed_spawns = 5
            },
            {
              enemy = "Orc Warrior",
              guaranteed_spawns = 3
            },
            {
              enemy = "Adder",
              guaranteed_spawns = 5
            }
          }
        }
      }
    }
  }
  local templates = OldMaps.Dungeon1
  local on_generate_dungeon
  on_generate_dungeon = function(map, floor)
    if floor == #templates then
      for i = 1, 2 do
        local item_placer
        item_placer = function(map, xy)
          local item = ItemUtils.item_generate(ItemGroups.basic_items)
          return MapUtils.spawn_item(map, item.type, item.amount, xy)
        end
        local tileset = Tilesets.pebble
        local vault = SourceMap.area_template_create(Vaults.small_item_vault({
          rng = map.rng,
          item_placer = item_placer,
          tileset = tileset
        }))
        if not place_feature(map, vault) then
          return nil
        end
      end
      local area = {
        0,
        0,
        map.size[1],
        map.size[2]
      }
      for i = 1, 4 do
        local sqr = MapUtils.random_square(map, area, {
          matches_none = {
            FLAG_INNER_PERIMETER,
            SourceMap.FLAG_HAS_OBJECT,
            SourceMap.FLAG_SOLID
          }
        })
        if not sqr then
          return nil
        end
        Region1.generate_store(map, sqr)
      end
      local sqr = MapUtils.random_square(map, area, {
        matches_none = {
          FLAG_INNER_PERIMETER,
          SourceMap.FLAG_HAS_OBJECT,
          SourceMap.FLAG_SOLID
        }
      })
      if not sqr then
        return nil
      end
    else
      if floor == 1 then
        on_generate_dungeon = function(map, floor)
          for i = 1, 3 do
            local item = ItemUtils.randart_generate(1)
            local item_placer
            item_placer = function(map, xy)
              return MapUtils.spawn_item(map, item.type, item.amount, xy)
            end
            local tileset = Tilesets.snake
            local vault = SourceMap.area_template_create(Vaults.small_item_vault({
              rng = map.rng,
              item_placer = item_placer,
              tileset = tileset
            }))
            if not place_feature(map, vault) then
              return nil
            end
          end
          return true
        end
        local spawn_portal = safe_portal_spawner(Tilesets.snake)
        local dungeon = {
          label = 'Ogre Lair',
          tileset = Tilesets.snake,
          templates = inner_encounter_template,
          on_generate = on_generate_dungeon,
          spawn_portal = spawn_portal,
          sprite_out = Region1.stair_kinds_index(5, 7)
        }
        local gold_placer
        gold_placer = function(map, xy)
          return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
        end
        local door_placer
        door_placer = function(map, xy)
          return MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
        end
        local place_dungeon = callable_once(Region1.old_dungeon_placement_function(InnerMapSeq, dungeon))
        local vault = SourceMap.area_template_create(Vaults.sealed_dungeon({
          dungeon_placer = place_dungeon,
          tileset = Tilesets.snake,
          door_placer = door_placer,
          gold_placer = gold_placer
        }))
        if not place_feature(map, vault) then
          return nil
        end
        if not SourceMap.area_fully_connected({
          map = map,
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
          }
        }) then
          return nil
        end
      end
    end
    return true
  end
  local door_placer
  door_placer = function(map, xy)
    return MapUtils.spawn_door(map, xy)
  end
  local on_placement
  on_placement = function(map)
    return InnerMapSeq:slot_resolve(1, map)
  end
  local dungeon = {
    label = 'Outpost',
    tileset = Tilesets.pebble,
    templates = templates,
    on_generate = on_generate_dungeon,
    on_placement = on_placement
  }
  local FIRST = true
  local place_dungeon
  place_dungeon = function(map, xy)
    if FIRST then
      Region1.old_dungeon_placement_function(MapSeq, dungeon)(map, xy)
      FIRST = false
    end
  end
  local vault = SourceMap.area_template_create(Vaults.ridge_dungeon({
    dungeon_placer = place_dungeon,
    door_placer = door_placer,
    tileset = Tilesets.pebble
  }))
  if not place_feature(map, vault, regions) then
    return false
  end
  return true
end
local place_temple
place_temple = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  do
    local _accum_0 = { }
    local _len_0 = 1
    for floor, base in ipairs(OldMaps.Dungeon2) do
      local template = table.merge(base, {
        tileset = Tilesets.temple,
        label = ({
          "Temple Entrance",
          "Temple Chamber",
          "Temple Inner Sanctum"
        })[floor],
        on_generate = function()
          if floor < #templates then
            return true
          end
          local _list_0 = {
            "Azurite Key",
            "Scroll of Experience",
            false
          }
          for _index_0 = 1, #_list_0 do
            local type = _list_0[_index_0]
            local vault = SourceMap.area_template_create(Vaults.small_item_vault({
              rng = map.rng,
              item_placer = function(map, xy)
                local amount = 1
                if not type then
                  do
                    local _obj_0 = ItemUtils.item_generate(ItemGroups.enchanted_items)
                    type, amount = _obj_0.type, _obj_0.amount
                  end
                end
                return MapUtils.spawn_item(map, type, amount, xy)
              end,
              tileset = Tilesets.snake
            }))
            if not place_feature(map, vault) then
              return false
            end
          end
          return true
        end
      })
      local _value_0
      do
        local _base_0 = OldMaps.create_map_desc(template)
        local _fn_0 = _base_0.linker
        _value_0 = function(...)
          return _fn_0(_base_0, ...)
        end
      end
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    local entrance, chamber, sanctum
    entrance, chamber, sanctum = _accum_0[1], _accum_0[2], _accum_0[3]
  end
  for i = 1, 3 do
    entrance:link_linker(chamber, "spr_gates.enter", "spr_gates.return")
    chamber:link_linker(sanctum, "spr_gates.enter", "spr_gates.return")
  end
  local vault = SourceMap.area_template_create(Vaults.sealed_dungeon({
    tileset = Tilesets.temple,
    gold_placer = function(map, xy)
      return nil
    end,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy)
    end,
    dungeon_placer = callable_once(function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_temple")
      return entrance:link_portal(portal, "spr_gates.exit_dungeon")
    end),
    player_spawn_area = true
  }))
  return place_feature(map, vault, regions)
end
local overdungeon_features
overdungeon_features = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local OldMapSeq4 = MapSequence.create({
    preallocate = 1
  })
  append(map.post_game_map, function(game_map)
    return OldMapSeq4:slot_resolve(1, game_map)
  end)
  local place_purple_dragon_lair
  place_purple_dragon_lair = function()
    local name = "Purple Dragon Lair"
    local create_compiler_context
    create_compiler_context = function()
      local cc = MapCompilerContext.create()
      cc:register(name, Places.DragonLair)
      append(map.post_game_map, function(game_map)
        return cc:register("root", function()
          return game_map
        end)
      end)
      return cc
    end
    local cc = create_compiler_context()
    local vault = SourceMap.area_template_create(Vaults.graghs_lair_entrance({
      tileset = Tilesets.hell,
      dungeon_placer = function(map, xy)
        local portal = MapUtils.spawn_portal(map, xy, "spr_gates.volcano_portal")
        local other_portal = cc:add_pending_portal(name, function(feature, compiler)
          return MapUtils.random_portal(compiler.map, nil, "spr_gates.volcano_exit")
        end)
        other_portal:connect({
          feature = portal,
          label = "root"
        })
        portal.on_player_interact = make_on_player_interact(cc, other_portal)
        return portal
      end,
      door_placer = function(map, xy)
        return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
      end,
      enemy_placer = function(map, xy)
        local enemy = OldMaps.enemy_generate({
          {
            enemy = "Fire Bat",
            chance = 100
          }
        })
        return MapUtils.spawn_enemy(map, enemy, xy)
      end
    }))
    return place_feature(map, vault, regions)
  end
  if not place_purple_dragon_lair() then
    return false
  end
  local place_graghs_lair
  place_graghs_lair = function()
    local Seq = MapSequence.create({
      preallocate = 1
    })
    local create
    create = function(offset)
      if offset == nil then
        offset = 1
      end
      local HiveSeq = MapSequence.create({
        preallocate = 1
      })
      local on_generate
      on_generate = function(map, floor)
        return true
      end
      local on_finish
      on_finish = function(game_map, floor)
        if floor == 1 then
          return HiveSeq:slot_resolve(1, game_map)
        end
      end
      local dungeon = require("maps.GraghsLair")
      return NewDungeons.make_linear_dungeon({
        MapSeq = Seq,
        offset = offset,
        dungeon_template = dungeon.TEMPLATE,
        on_generate = on_generate,
        on_finish = on_finish,
        sprite_up = function(floor)
          return "spr_gates.exit_lair"
        end,
        sprite_down = function(floor)
          return "spr_gates.enter"
        end,
        portals_up = function(floor)
          return 3
        end,
        portals_down = function(floor)
          return 0
        end
      })
    end
    local tileset = Tilesets.lair
    local door_placer
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
    end
    local next_dungeon = {
      1
    }
    local place_dungeon
    place_dungeon = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_lair")
      local c = (Seq:forward_portal_add(1, portal, next_dungeon[1], function()
        return create(1)
      end))
      if World.player_amount > 1 then
        append(map.post_maps, c)
      end
      next_dungeon[1] = next_dungeon[1] + 1
    end
    local enemy_placer
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate({
        {
          enemy = "Sheep",
          chance = 100
        }
      })
      return MapUtils.spawn_enemy(map, enemy, xy)
    end
    local vault = SourceMap.area_template_create(Vaults.graghs_lair_entrance({
      dungeon_placer = place_dungeon,
      tileset = tileset,
      door_placer = door_placer,
      enemy_placer = enemy_placer,
      player_spawn_area = false
    }))
    if not place_feature(map, vault, regions) then
      return true
    end
    return append(map.post_game_map, function(game_map)
      return Seq:slot_resolve(1, game_map)
    end)
  end
  if place_graghs_lair() then
    print("RETRY: place_graghs_lair()")
    return false
  end
  if not place_outpost(region_set) then
    return nil
  end
  local place_crypt
  place_crypt = function()
    local CryptSeq = MapSequence.create({
      preallocate = 1
    })
    local door_placer
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
    end
    local next_dungeon = {
      1
    }
    local place_dungeon
    place_dungeon = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_crypt")
      local c = (CryptSeq:forward_portal_add(1, portal, next_dungeon[1], function()
        return M.crypt_create(CryptSeq, 2)
      end))
      if World.player_amount > 1 then
        append(map.post_game_map, c)
      end
      next_dungeon[1] = next_dungeon[1] + 1
    end
    local enemy_placer
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
      return MapUtils.spawn_enemy(map, enemy, xy)
    end
    local vault = SourceMap.area_template_create(Vaults.crypt_dungeon({
      dungeon_placer = place_dungeon,
      tileset = Tilesets.crypt,
      door_placer = door_placer,
      enemy_placer = enemy_placer,
      player_spawn_area = false
    }))
    if not place_feature(map, vault, regions) then
      return true
    end
    return append(map.post_game_map, function(game_map)
      return CryptSeq:slot_resolve(1, game_map)
    end)
  end
  if place_crypt() then
    print("RETRY: place_crypt()")
    return false
  end
  local place_hard
  place_hard = function()
    local tileset = Tilesets.pixulloch
    local dungeon = {
      label = 'Pixullochia',
      tileset = tileset,
      templates = OldMaps.Dungeon4,
      spawn_portal = safe_portal_spawner(tileset)
    }
    local door_placer
    door_placer = function(map, xy)
      return MapUtils.spawn_lanarts_door(map, xy)
    end
    local enemy_placer
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
      return MapUtils.spawn_enemy(map, enemy, xy)
    end
    local place_dungeon = Region1.old_dungeon_placement_function(OldMapSeq4, dungeon)
    local vault = SourceMap.area_template_create(Vaults.skull_surrounded_dungeon({
      dungeon_placer = place_dungeon,
      enemy_placer = enemy_placer,
      door_placer = door_placer,
      tileset = tileset,
      player_spawn_area = false
    }))
    if not place_feature(map, vault, regions) then
      return true
    end
  end
  if place_hard() then
    print("RETRY: place_hard()")
    return nil
  end
  append(map.post_maps, function()
    return overdungeon_items_and_enemies(region_set)
  end)
  return true
end
local overworld_features
overworld_features = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local OldMapSeq3 = MapSequence.create({
    preallocate = 1
  })
  append(map.post_game_map, function(game_map)
    return OldMapSeq3:slot_resolve(1, game_map)
  end)
  local place_outdoor_ridges
  place_outdoor_ridges = function()
    local door_placer
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy)
    end
    local item_placer
    item_placer = function(map, xy)
      local item = ItemUtils.item_generate(ItemGroups.basic_items)
      return MapUtils.spawn_item(map, item.type, item.amount, xy)
    end
    local vault = SourceMap.area_template_create(Vaults.ridge_dungeon({
      dungeon_placer = item_placer,
      door_placer = door_placer,
      tileset = Tilesets.pebble
    }))
    if not place_feature(map, vault, regions) then
      return false
    end
  end
  for i = 1, 4 do
    if place_outdoor_ridges() then
      print("RETRY: place_outdoor_ridges()")
      return nil
    end
  end
  if not map.rng:random_choice({
    place_snake_pit,
    place_temple
  })(region_set) then
    return nil
  end
  if not place_hive(region_set) then
    print("RETRY: place_hive()")
    return nil
  end
  if not place_underdungeon_vault(region_set) then
    print("RETRY: place_hive()")
    return nil
  end
  local place_small_vaults
  place_small_vaults = function()
    for i = 1, map.rng:random(2, 3) do
      local enemy_placer
      enemy_placer = function(map, xy)
        local enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
        return MapUtils.spawn_enemy(map, enemy, xy)
      end
      local door_placer
      door_placer = function(map, xy)
        return MapUtils.spawn_door(map, xy, nil, xy)
      end
      local store_placer
      store_placer = function(map, xy)
        return Region1.generate_store(map, xy)
      end
      local item_placer
      item_placer = function(map, xy)
        if map.rng.chance(.1) then
          return MapUtils.spawn_item(map, "Scroll of Experience", 1, xy)
        else
          local item = ItemUtils.item_generate(ItemGroups.basic_items)
          return MapUtils.spawn_item(map, item.type, item.amount, xy)
        end
      end
      local gold_placer
      gold_placer = function(map, xy)
        return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
      end
      local tileset = Tilesets.snake
      local vault = SourceMap.area_template_create(Vaults.small_random_vault({
        rng = map.rng,
        item_placer,
        enemy_placer = enemy_placer,
        gold_placer = gold_placer,
        store_placer = store_placer,
        tileset = tileset,
        door_placer = door_placer,
        tileset = tileset
      }))
      if not place_feature(map, vault) then
        return false
      end
    end
  end
  if place_small_vaults() then
    print("RETRY: place_small_vaults()")
    return nil
  end
  local place_big_vaults
  place_big_vaults = function()
    local _list_0 = {
      Vaults.big_encounter1
    }
    for _index_0 = 1, #_list_0 do
      local template = _list_0[_index_0]
      local enemy_placer
      enemy_placer = function(map, xy)
        local enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
        return MapUtils.spawn_enemy(map, enemy, xy)
      end
      local item_placer
      item_placer = function(map, xy)
        local item = ItemUtils.item_generate(ItemGroups.basic_items)
        return MapUtils.spawn_item(map, item.type, item.amount, xy)
      end
      local gold_placer
      gold_placer = function(map, xy)
        if map.rng:chance(.7) then
          return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
        end
      end
      local door_placer
      door_placer = function(map, xy)
        return MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
      end
      local vault = SourceMap.area_template_create(template({
        enemy_placer = enemy_placer,
        item_placer = item_placer,
        gold_placer = gold_placer,
        door_placer = door_placer
      }))
      if not place_feature(map, vault) then
        return true
      end
    end
  end
  local place_centaur_challenge
  place_centaur_challenge = function()
    local enemy_placer
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
      return MapUtils.spawn_enemy(map, enemy, xy)
    end
    local boss_placer
    boss_placer = function(map, xy)
      return MapUtils.spawn_enemy(map, "Centaur Hunter", xy)
    end
    local n_items_placed = 0
    local item_placer
    item_placer = function(map, xy)
      local item = ItemUtils.item_generate(ItemGroups.basic_items, false, 1, ((function()
        if n_items_placed == 0 then
          return 100
        else
          return 2
        end
      end)()))
      MapUtils.spawn_item(map, item.type, item.amount, xy)
      n_items_placed = n_items_placed + 1
    end
    local gold_placer
    gold_placer = function(map, xy)
      if map.rng:chance(.7) then
        return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
      end
    end
    local door_placer
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
    end
    local vault = SourceMap.area_template_create(Vaults.anvil_encounter({
      enemy_placer = enemy_placer,
      boss_placer = boss_placer,
      item_placer = item_placer,
      gold_placer = gold_placer,
      door_placer = door_placer
    }))
    if not place_feature(map, vault, regions) then
      return true
    end
  end
  local place_mini_features
  place_mini_features = function()
    local i = 0
    local _list_0 = {
      Vaults.cavern,
      Vaults.stone_henge,
      Vaults.stone_henge,
      Vaults.stone_henge
    }
    for _index_0 = 1, #_list_0 do
      local _continue_0 = false
      repeat
        local template = _list_0[_index_0]
        i = i + 1
        local enemy_placer
        enemy_placer = function(map, xy)
          local enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
          return MapUtils.spawn_enemy(map, enemy, xy)
        end
        local store_placer
        store_placer = function(map, xy)
          return Region1.generate_store(map, xy)
        end
        if i ~= 1 and i ~= 4 then
          store_placer = do_nothing
        end
        if i == 4 then
          store_placer = function(map, xy)
            return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
          end
        end
        local item_placer
        item_placer = function(map, xy)
          local item = ItemUtils.item_generate(ItemGroups.basic_items)
          return MapUtils.spawn_item(map, item.type, item.amount, xy)
        end
        local gold_placer
        gold_placer = function(map, xy)
          return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
        end
        local door_placer
        door_placer = function(map, xy)
          return MapUtils.spawn_door(map, xy)
        end
        local vault = SourceMap.area_template_create(template({
          enemy_placer = enemy_placer,
          store_placer = store_placer,
          item_placer = item_placer,
          gold_placer = gold_placer,
          door_placer = door_placer
        }))
        if not place_feature(map, vault, regions) then
          _continue_0 = true
          break
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end
  if place_mini_features() then
    print("RETRY: place_mini_features()")
    return nil
  end
  append(map.post_maps, function()
    return overworld_items_and_enemies(region_set)
  end)
  return true
end
local generate_map_node
local pebble_overdungeon
pebble_overdungeon = function(rng)
  local arc_chance = 0.05
  local size = rng:random_choice({
    {
      125,
      85
    },
    {
      85,
      125
    }
  })
  local number_regions = rng:random(5, 7)
  local connect_line_width
  connect_line_width = function()
    return rng:random(2, 6)
  end
  local default_wall = Tile.create(Tilesets.pebble.wall, true, true, {
    SourceMap.FLAG_SOLID
  })
  local room_radius
  room_radius = function()
    return rng:random(5, 10)
  end
  return generate_map_node(function()
    local map = NewMaps.source_map_create({
      rng = rng,
      size = {
        OVERWORLD_DIM_LESS,
        OVERWORLD_DIM_MORE
      },
      default_content = default_wall.id,
      default_flags = {
        SourceMap.FLAG_SOLID
      },
      map_label = "Underdungeon",
      arc_chance = arc_chance
    })
    local template = nilprotect({
      default_wall = default_wall,
      subtemplates = {
        nilprotect({
          is_overworld = true,
          size = size,
          number_regions = number_regions,
          floor1 = DUNGEON_TILESET.floor1,
          floor2 = DUNGEON_TILESET.floor2,
          wall1 = DUNGEON_TILESET.wall1,
          wall2 = DUNGEON_TILESET.wall2,
          rect_room_num_range = {
            4,
            8
          },
          rect_room_size_range = {
            14,
            20
          },
          rvo_iterations = 100,
          connect_line_width = connect_line_width,
          room_radius = room_radius,
          region_delta_func = spread_region_delta_func,
          n_statues = 4
        })
      },
      outer_conf = nilprotect({
        floor1 = DUNGEON_TILESET.floor1,
        floor2 = DUNGEON_TILESET.floor2,
        connect_line_width = connect_line_width
      }),
      shell = 25
    })
    if not NewMaps.map_try_create(map, rng, template) then
      return nil
    end
    local full_region_set = {
      map = map,
      regions = map.regions
    }
    append(map.post_maps, function()
      return place_doors_and_statues(full_region_set)
    end)
    if not overdungeon_features(full_region_set) then
      return nil
    end
    return map
  end)
end
local grassy_overworld
grassy_overworld = function(rng)
  local arc_chance = 0.05
  local size = rng:random_choice({
    {
      65,
      45
    },
    {
      45,
      65
    }
  })
  local number_regions = rng:random(5, 7)
  local connect_line_width
  connect_line_width = function()
    return rng:random(2, 6)
  end
  local default_wall = Tile.create(Tilesets.grass.wall, true, true, {
    FLAG_OVERWORLD
  })
  local room_radius
  room_radius = function()
    return rng:random(5, 10)
  end
  return generate_map_node(function()
    local map = NewMaps.source_map_create({
      rng = rng,
      size = {
        OVERWORLD_DIM_LESS,
        OVERWORLD_DIM_MORE
      },
      default_content = default_wall.id,
      default_flags = {
        SourceMap.FLAG_SOLID,
        SourceMap.FLAG_SEETHROUGH
      },
      map_label = "Plain Valley",
      arc_chance = arc_chance
    })
    local template = nilprotect({
      default_wall = default_wall,
      subtemplates = {
        nilprotect({
          is_overworld = true,
          size = size,
          number_regions = number_regions,
          floor1 = OVERWORLD_TILESET.floor1,
          floor2 = OVERWORLD_TILESET.floor2,
          wall1 = OVERWORLD_TILESET.wall1,
          wall2 = OVERWORLD_TILESET.wall2,
          rect_room_num_range = {
            0,
            0
          },
          rect_room_size_range = {
            10,
            15
          },
          rvo_iterations = 100,
          connect_line_width = connect_line_width,
          room_radius = room_radius,
          region_delta_func = spread_region_delta_func,
          n_statues = 4
        })
      },
      outer_conf = nilprotect({
        floor1 = OVERWORLD_TILESET.floor1,
        floor2 = OVERWORLD_TILESET.floor2,
        connect_line_width = connect_line_width
      }),
      shell = 25
    })
    if not NewMaps.map_try_create(map, rng, template) then
      return nil
    end
    local full_region_set = {
      map = map,
      regions = map.regions
    }
    local overworld_region = {
      map = map,
      regions = table.filter(map.regions, function(r)
        return r.conf.is_overworld
      end)
    }
    local overdungeon_region = {
      map = map,
      regions = table.filter(map.regions, function(r)
        return not r.conf.is_overworld
      end)
    }
    append(map.post_maps, function()
      return place_doors_and_statues(full_region_set)
    end)
    if not overworld_features(overworld_region) then
      return nil
    end
    return map
  end)
end
local MAX_GENERATE_ITERS = 1000
generate_map_node = function(create_map)
  return NewMaps.try_n_times(MAX_GENERATE_ITERS, function()
    local map = create_map()
    if not map then
      return nil
    end
    if not NewMaps.check_connection(map) then
      print("ABORT: connection check failed")
      return nil
    end
    NewMaps.generate_door_candidates(map, map.rng, map.regions)
    local _list_0 = map.post_maps
    for _index_0 = 1, #_list_0 do
      local f = _list_0[_index_0]
      if f() == 'reject' then
        return nil
      end
    end
    return {
      map = map,
      static_area = MapRegion.create({ }),
      editable_area = MapRegion.create({
        rectangle_points(0, 0, map.size[1], map.size[2])
      })
    }
  end)
end
underdungeon_create = function(links)
  local map
  map = pebble_overdungeon(NewMaps.new_rng()).map
  for _index_0 = 1, #links do
    local link = links[_index_0]
    local xy = MapUtils.random_square(map, nil, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    link(map, xy)
  end
  local game_map = NewMaps.generate_game_map(map)
  local _list_0 = map.post_game_map
  for _index_0 = 1, #_list_0 do
    local f = _list_0[_index_0]
    f(game_map)
  end
  return game_map
end
local overworld_create
overworld_create = function()
  local map
  map = grassy_overworld(NewMaps.new_rng()).map
  local player_spawn_points = MapUtils.pick_player_squares(map, map.player_candidate_squares)
  assert(player_spawn_points, "Could not pick player spawn squares!")
  local game_map = NewMaps.generate_game_map(map)
  local _list_0 = map.post_game_map
  for _index_0 = 1, #_list_0 do
    local f = _list_0[_index_0]
    f(game_map)
  end
  World.players_spawn(game_map, player_spawn_points)
  Map.set_vision_radius(game_map, OVERWORLD_VISION_RADIUS)
  return game_map
end
return {
  overworld_create = overworld_create,
  place_feature = place_feature,
  test_determinism = function()
    return nil
  end,
  generate_map_node = generate_map_node
}
