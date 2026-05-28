local Tile, default_region_delta_func
do
  local _obj_0 = require("maps.GenerateUtils")
  Tile, default_region_delta_func = _obj_0.Tile, _obj_0.default_region_delta_func
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
local Vaults = require("maps.Vaults")
local SourceMap = require("core.SourceMap")
local OldMaps = require("maps.OldMaps")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_HAS_VAULT, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_HAS_VAULT, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_HAS_VAULT, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local snake_pit_floor_plan
snake_pit_floor_plan = function(rng)
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
    tileset = Tilesets.snake,
    floor1 = Tile.create(Tilesets.snake.floor, false, true, { }, {
      FLAG_OVERWORLD
    }),
    floor2 = Tile.create(Tilesets.snake.floor_alt, false, true, { }, {
      FLAG_OVERWORLD
    }),
    wall1 = Tile.create(Tilesets.snake.wall, true, false, { }, {
      FLAG_OVERWORLD
    }),
    wall2 = Tile.create(Tilesets.snake.wall_alt, true, false, { }, {
      FLAG_OVERWORLD
    }),
    size = {
      45,
      45
    },
    n_subareas = 3,
    n_enemies = 0,
    n_encounter_vaults = 1,
    enemy_entries = {
      {
        enemy = "Adder",
        guaranteed_spawns = 8,
        chance = 100
      },
      {
        enemy = "Black Mamba",
        guaranteed_spawns = 2,
        chance = 25
      },
      {
        enemy = "Mouther",
        guaranteed_spawns = 3
      }
    },
    item_groups = {
      {
        ItemGroups.basic_items,
        8
      },
      {
        {
          item = "Scroll of Experience",
          chance = 100
        },
        1
      }
    },
    number_regions = 2,
    room_radius = function()
      return rng:randomf(7, 12)
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
  local plan = snake_pit_floor_plan(rng)
  local subtemplates
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, plan.n_subareas do
      _accum_0[_len_0] = snake_pit_floor_plan(rng)
      _len_0 = _len_0 + 1
    end
    subtemplates = _accum_0
  end
  return NewDungeons.make_dungeon_template({
    tileset = Tilesets.snake,
    subtemplates = subtemplates,
    w = plan.size[1] * 3.5 + 10,
    h = plan.size[2] * 3.5 + 10,
    _enemy_entries = function(self)
      return plan.enemy_entries
    end,
    _n_enemies = function(self)
      return math.ceil(plan.n_enemies * OldMaps.enemy_bonus())
    end,
    _n_encounter_vaults = function(self)
      return plan.n_encounter_vaults
    end,
    _create_encounter_vault = function(self, map)
      local default_config = self:_default_vault_config({
        door_sprite = NewDungeons._runed_door_sprite,
        door_key = "dummykey"
      })
      local n_items_placed = 0
      local conf = table.merge(default_config, {
        rng = map.rng,
        item_placer = function(map, xy)
          local item
          if n_items_placed == 0 then
            item = {
              type = "Azurite Key",
              amount = 1
            }
          else
            item = ItemUtils.randart_generate(1)
          end
          MapUtils.spawn_item(map, item.type, item.amount, xy)
          n_items_placed = n_items_placed + 1
        end
      })
      local vault = SourceMap.area_template_create(Vaults.crypt_encounter_vault(conf))
      return NewDungeons.place_feature(map, vault)
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
      if not self:_create_encounter_vault(map) then
        print("ABORT: enemies")
        return nil
      end
      if not self:_spawn_statues(map) then
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
  map_label = "Snake Pit Depths",
  size = {
    130,
    130
  },
  default_content = Tilesets.snake.wall,
  arc_chance = 1.0,
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
