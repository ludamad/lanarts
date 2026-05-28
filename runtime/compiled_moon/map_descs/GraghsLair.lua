local map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator
do
  local _obj_0 = require("maps.GenerateUtils")
  map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator = _obj_0.map_place_object, _obj_0.ellipse_points, _obj_0.LEVEL_PADDING, _obj_0.Region, _obj_0.RVORegionPlacer, _obj_0.random_rect_in_rect, _obj_0.random_ellipse_in_ellipse, _obj_0.ring_region_delta_func, _obj_0.default_region_delta_func, _obj_0.spread_region_delta_func, _obj_0.center_region_delta_func, _obj_0.towards_region_delta_func, _obj_0.random_region_add, _obj_0.subregion_minimum_spanning_tree, _obj_0.region_minimum_spanning_tree, _obj_0.Tile, _obj_0.tile_operator
end
local NewMaps = require("maps.NewMaps")
local NewDungeons = require("maps.NewDungeons")
local MapDesc, MapNode
do
  local _obj_0 = require("maps.MapDesc")
  MapDesc, MapNode = _obj_0.MapDesc, _obj_0.MapNode
end
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
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_HAS_VAULT, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_HAS_VAULT, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_HAS_VAULT, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local graghs_lair_plan
graghs_lair_plan = function(rng)
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
    tileset = Tilesets.lair,
    floor1 = Tile.create(Tilesets.lair.floor, false, true, { }, {
      FLAG_OVERWORLD
    }),
    floor2 = Tile.create(Tilesets.lair.floor_alt, false, true, { }, {
      FLAG_OVERWORLD
    }),
    wall1 = Tile.create(Tilesets.lair.wall, true, false, { }, {
      FLAG_OVERWORLD
    }),
    wall2 = Tile.create(Tilesets.lair.wall_alt, true, false, { }, {
      FLAG_OVERWORLD
    }),
    n_subareas = 1,
    size = {
      100,
      100
    },
    n_items = 4,
    n_enemies = 0,
    n_stores = 2,
    n_epic_stores = 2,
    n_encounter_vaults = 0,
    enemy_entries = {
      {
        enemy = "Sheep",
        guaranteed_spawns = 2,
        chance = 65
      },
      {
        enemy = "Mouther",
        guaranteed_spawns = 1,
        chance = 15
      },
      {
        enemy = "Black Mamba",
        guaranteed_spawns = 1,
        chance = 20
      },
      {
        enemy = "Gragh",
        guaranteed_spawns = 1
      }
    },
    encounter_enemy_entries = {
      {
        enemy = "Sheep",
        chance = 60
      },
      {
        enemy = "Mouther",
        chance = 15
      },
      {
        enemy = "Black Mamba",
        chance = 25
      }
    },
    item_groups = {
      {
        ItemGroups.enchanted_items,
        4
      }
    },
    number_regions = 5,
    room_radius = function()
      return rng:random(5, 12)
    end,
    rect_room_num_range = {
      0,
      0
    },
    rect_room_size_range = {
      1,
      1
    },
    n_statues = 4
  }
end
local make_template
make_template = function(rng, back_links, forward_links)
  if back_links == nil then
    back_links = { }
  end
  if forward_links == nil then
    forward_links = { }
  end
  local plan = graghs_lair_plan(rng)
  local subtemplates
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, plan.n_subareas do
      _accum_0[_len_0] = graghs_lair_plan(rng)
      _len_0 = _len_0 + 1
    end
    subtemplates = _accum_0
  end
  return NewDungeons.make_dungeon_template({
    map_label = "Gragh's Lair",
    tileset = Tilesets.lair,
    subtemplates = subtemplates,
    w = plan.size[1] * 2.5 + 10,
    h = plan.size[2] * 2.5 + 10,
    _enemy_entries = function(self)
      return plan.enemy_entries
    end,
    _n_enemies = function(self)
      return math.ceil(plan.n_enemies)
    end,
    _n_encounter_vaults = function(self)
      return plan.n_encounter_vaults
    end,
    _encounter_vault = function(self)
      return Vaults.crypt_encounter_vault
    end,
    _create_encounter_rooms = function(self, map)
      local default_config = self:_default_vault_config({
        door_sprite = NewDungeons._runed_door_sprite,
        door_key = "dummykey"
      })
      local encounter_enemy_placer
      encounter_enemy_placer = function(map, xy)
        local enemy = OldMaps.enemy_generate(plan.encounter_enemy_entries)
        return MapUtils.spawn_enemy(map, enemy, xy)
      end
      for i = 1, self:_n_encounter_vaults() do
        local conf = table.merge(default_config, {
          rng = map.rng,
          enemy_placer = encounter_enemy_placer
        })
        local vault = SourceMap.area_template_create(self:_encounter_vault()(conf))
        if not NewDungeons.place_feature(map, vault) then
          return nil
        end
      end
      return true
    end,
    _item_groups = function(self)
      return plan.item_groups
    end,
    _create_stairs_down = function(self, map)
      local bbox = {
        0,
        0,
        map.size[1],
        map.size[2]
      }
      for _index_0 = 1, #forward_links do
        local forward_link = forward_links[_index_0]
        local xy = MapUtils.random_square(map, bbox, {
          matches_none = {
            FLAG_INNER_PERIMETER,
            SourceMap.FLAG_HAS_OBJECT,
            FLAG_HAS_VAULT,
            SourceMap.FLAG_SOLID
          }
        })
        forward_link(map, xy)
      end
      return true
    end,
    _create_stairs_up = function(self, map)
      local base_conf = self:_default_vault_config()
      for _index_0 = 1, #back_links do
        local back_link = back_links[_index_0]
        local conf = table.merge(base_conf, {
          rng = map.rng,
          item_placer = back_link
        })
        local vault = SourceMap.area_template_create(Vaults.crypt_entrance_vault(conf))
        if not NewDungeons.place_feature(map, vault) then
          return false
        end
      end
      return true
    end,
    on_create_source_map = function(self, map)
      if not self:_create_stairs_up(map) then
        print("ABORT: stairs up")
        return nil
      end
      if not self:_create_stairs_down(map) then
        print("ABORT: stairs down")
        return nil
      end
      if not self:_create_encounter_rooms(map) then
        print("ABORT: stairs encounter")
        return nil
      end
      if not self:_spawn_statues(map) then
        print("ABORT: statues")
        return nil
      end
      if not self:_spawn_stores(map) then
        print("ABORT: statues")
        return nil
      end
      if not self:_spawn_items(map) then
        print("ABORT: items")
        return nil
      end
      if not self:_spawn_enemies(map) then
        print("ABORT: enemies")
        return nil
      end
      NewMaps.generate_door_candidates(map, rng, map.regions)
      return true
    end
  })
end
return MapDesc.create({
  map_label = "Gragh's Lair",
  size = {
    130,
    130
  },
  default_content = Tilesets.lair.wall,
  wandering_enabled = false,
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
