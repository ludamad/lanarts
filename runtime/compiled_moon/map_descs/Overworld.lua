local Tile, spread_region_delta_func
do
  local _obj_0 = require("maps.GenerateUtils")
  Tile, spread_region_delta_func = _obj_0.Tile, _obj_0.spread_region_delta_func
end
local NewMaps = require("maps.NewMaps")
local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local Vaults = require("maps.Vaults")
local place_vault_in
place_vault_in = require("maps.VaultUtils").place_vault_in
local SourceMap = require("core.SourceMap")
local OldMaps = require("maps.OldMaps")
local Region1 = require("maps.Region1")
local MapDesc, MapNode
do
  local _obj_0 = require("maps.MapDesc")
  MapDesc, MapNode = _obj_0.MapDesc, _obj_0.MapNode
end
local FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_NO_ENEMY_SPAWN
FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_NO_ENEMY_SPAWN = Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_OVERWORLD, Vaults.FLAG_NO_ENEMY_SPAWN
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
local OVERWORLD_TILESET = create_overworld_scheme(Tilesets.grass)
local OVERWORLD_AMBIENT_ENEMIES = table.tconcat(OldMaps.medium_animals, {
  {
    enemy = "Sheep",
    chance = 45,
    group_chance = 35,
    group_size = 2
  },
  {
    enemy = "Black Mamba",
    chance = 20
  },
  {
    enemy = "Skeleton",
    chance = 18
  },
  {
    enemy = "Cloud Elemental",
    chance = 12,
    group_chance = 25,
    group_size = 2
  },
  {
    enemy = "Mouther",
    chance = 10
  }
})
local OVERWORLD_MEADOW_ENEMIES = {
  {
    enemy = "Sheep",
    chance = 70,
    group_chance = 45,
    group_size = 2
  },
  {
    enemy = "Chicken",
    chance = 18
  },
  {
    enemy = "Giant Rat",
    chance = 16
  },
  {
    enemy = "Giant Bat",
    chance = 12
  },
  {
    enemy = "Black Mamba",
    chance = 16
  },
  {
    enemy = "Skeleton",
    chance = 10
  },
  {
    enemy = "Cloud Elemental",
    chance = 6
  }
}
local OVERWORLD_GROVE_ENEMIES = {
  {
    enemy = "Sheep",
    chance = 25,
    group_chance = 25,
    group_size = 2
  },
  {
    enemy = "Giant Spider",
    chance = 24
  },
  {
    enemy = "Black Mamba",
    chance = 28
  },
  {
    enemy = "Skeleton",
    chance = 18
  },
  {
    enemy = "Cloud Elemental",
    chance = 18,
    group_chance = 30,
    group_size = 2
  },
  {
    enemy = "Mouther",
    chance = 18
  }
}
local OVERWORLD_STONES_ENEMIES = {
  {
    enemy = "Sheep",
    chance = 15,
    group_chance = 20,
    group_size = 2
  },
  {
    enemy = "Giant Bat",
    chance = 16
  },
  {
    enemy = "Skeleton",
    chance = 36
  },
  {
    enemy = "Cloud Elemental",
    chance = 22,
    group_chance = 25,
    group_size = 2
  },
  {
    enemy = "Mouther",
    chance = 18
  },
  {
    enemy = "Black Mamba",
    chance = 12
  }
}
local OVERWORLD_RIVER_ENEMIES = {
  {
    enemy = "Sheep",
    chance = 28,
    group_chance = 35,
    group_size = 2
  },
  {
    enemy = "Giant Rat",
    chance = 14
  },
  {
    enemy = "Black Mamba",
    chance = 34
  },
  {
    enemy = "Cloud Elemental",
    chance = 24,
    group_chance = 35,
    group_size = 2
  },
  {
    enemy = "Skeleton",
    chance = 12
  },
  {
    enemy = "Mouther",
    chance = 10
  }
}
local OVERWORLD_PREDATOR_ENEMIES = {
  {
    enemy = "Sheep",
    chance = 14,
    group_chance = 20,
    group_size = 2
  },
  {
    enemy = "Giant Spider",
    chance = 22
  },
  {
    enemy = "Black Mamba",
    chance = 38
  },
  {
    enemy = "Mouther",
    chance = 25
  },
  {
    enemy = "Skeleton",
    chance = 16
  },
  {
    enemy = "Hound",
    chance = 5
  }
}
local OVERWORLD_RUIN_ENEMIES = {
  {
    enemy = "Sheep",
    chance = 12,
    group_chance = 20,
    group_size = 2
  },
  {
    enemy = "Giant Bat",
    chance = 14
  },
  {
    enemy = "Skeleton",
    chance = 42
  },
  {
    enemy = "Cloud Elemental",
    chance = 20,
    group_chance = 20,
    group_size = 2
  },
  {
    enemy = "Mouther",
    chance = 16
  },
  {
    enemy = "Black Mamba",
    chance = 10
  }
}
local OVERWORLD_VAULT_ENEMIES = table.tconcat(OldMaps.medium_enemies, {
  {
    enemy = "Black Mamba",
    chance = 35
  },
  {
    enemy = "Mouther",
    chance = 25
  },
  {
    enemy = "Fire Bat",
    chance = 12
  },
  {
    enemy = "Centaur Hunter",
    chance = 6
  }
})
local OVERWORLD_PROFILES = {
  {
    name = "open_meadow",
    region_base = 5,
    region_rolls = {
      2,
      2
    },
    connect_base = 3,
    connect_rolls = {
      4,
      4,
      3
    },
    room_base = 8,
    room_rolls = {
      4,
      4
    },
    size_base = 84,
    shell = 25,
    ridges = {
      3,
      5
    },
    small_vaults = {
      1,
      2
    },
    mini_features = {
      Vaults.cavern,
      Vaults.stone_henge,
      Vaults.stone_henge
    },
    ambient_enemies = 7,
    enemy_entries = OVERWORLD_MEADOW_ENEMIES,
    n_statues = 3
  },
  {
    name = "broken_grove",
    region_base = 7,
    region_rolls = {
      3,
      3
    },
    connect_base = 2,
    connect_rolls = {
      3,
      3,
      2
    },
    room_base = 6,
    room_rolls = {
      4,
      3,
      2
    },
    size_base = 88,
    shell = 22,
    ridges = {
      4,
      6
    },
    small_vaults = {
      2,
      4
    },
    mini_features = {
      Vaults.cavern,
      Vaults.cavern,
      Vaults.stone_henge,
      Vaults.stone_henge
    },
    ambient_enemies = 8,
    enemy_entries = OVERWORLD_GROVE_ENEMIES,
    n_statues = 4
  },
  {
    name = "old_stones",
    region_base = 8,
    region_rolls = {
      3,
      3,
      2
    },
    connect_base = 1,
    connect_rolls = {
      3,
      2,
      2
    },
    room_base = 5,
    room_rolls = {
      3,
      3,
      2
    },
    size_base = 92,
    shell = 20,
    ridges = {
      2,
      4
    },
    small_vaults = {
      3,
      5
    },
    mini_features = {
      Vaults.stone_henge,
      Vaults.stone_henge,
      Vaults.stone_henge,
      Vaults.cavern
    },
    ambient_enemies = 9,
    enemy_entries = OVERWORLD_STONES_ENEMIES,
    n_statues = 6
  },
  {
    name = "river_road",
    region_base = 6,
    region_rolls = {
      2,
      2,
      2
    },
    connect_base = 4,
    connect_rolls = {
      4,
      3,
      3
    },
    room_base = 7,
    room_rolls = {
      4,
      4,
      2
    },
    size_base = 90,
    shell = 23,
    ridges = {
      2,
      4
    },
    small_vaults = {
      1,
      3
    },
    mini_features = {
      Vaults.cavern,
      Vaults.stone_henge,
      Vaults.cavern
    },
    ambient_enemies = 8,
    enemy_entries = OVERWORLD_RIVER_ENEMIES,
    n_statues = 2
  },
  {
    name = "predator_thicket",
    region_base = 6,
    region_rolls = {
      2,
      2
    },
    connect_base = 2,
    connect_rolls = {
      3,
      2,
      2
    },
    room_base = 6,
    room_rolls = {
      3,
      3,
      2
    },
    size_base = 90,
    shell = 21,
    ridges = {
      2,
      4
    },
    small_vaults = {
      1,
      1
    },
    mini_features = {
      Vaults.cavern,
      Vaults.cavern,
      Vaults.stone_henge
    },
    ambient_enemies = 9,
    enemy_entries = OVERWORLD_PREDATOR_ENEMIES,
    n_statues = 1
  },
  {
    name = "ruin_crossroads",
    region_base = 9,
    region_rolls = {
      2,
      2,
      2
    },
    connect_base = 3,
    connect_rolls = {
      3,
      3,
      2
    },
    room_base = 6,
    room_rolls = {
      3,
      3,
      3
    },
    size_base = 94,
    shell = 21,
    ridges = {
      2,
      3
    },
    small_vaults = {
      3,
      4
    },
    mini_features = {
      Vaults.stone_henge,
      Vaults.stone_henge,
      Vaults.cavern,
      Vaults.stone_henge
    },
    ambient_enemies = 8,
    enemy_entries = OVERWORLD_RUIN_ENEMIES,
    n_statues = 7
  }
}
local profile_by_name
profile_by_name = function(name)
  if not (name and name ~= "") then
    return nil
  end
  for _index_0 = 1, #OVERWORLD_PROFILES do
    local profile = OVERWORLD_PROFILES[_index_0]
    if profile.name == name then
      return profile
    end
  end
  return nil
end
local roll_sum
roll_sum = function(rng, base, rolls)
  local value = base
  local _list_0 = (rolls or { })
  for _index_0 = 1, #_list_0 do
    local roll = _list_0[_index_0]
    value = value + rng:random(0, roll)
  end
  return value
end
local random_range
random_range = function(rng, range)
  return rng:random(range[1], range[2])
end
local place_doors_and_statues
place_doors_and_statues = function(region_set, profile)
  local map, regions
  map, regions = region_set.map, region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
    local conf = rawget(region, "conf") or profile or { }
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
    for i = 1, (conf.n_statues or 0) do
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
local overworld_items_and_enemies
overworld_items_and_enemies = function(region_set, profile)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local enemy_entries = profile.enemy_entries or OVERWORLD_AMBIENT_ENEMIES
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
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
    OldMaps.generate_from_enemy_entries(map, enemy_entries, profile.ambient_enemies, area, {
      matches_none = {
        SourceMap.FLAG_SOLID,
        Vaults.FLAG_HAS_VAULT,
        FLAG_NO_ENEMY_SPAWN
      }
    })
  end
  return true
end
local overworld_features
overworld_features = function(region_set, profile)
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
    if not place_vault_in(region_set, Vaults.ridge_dungeon({
      dungeon_placer = item_placer,
      door_placer = door_placer,
      tileset = Tilesets.pebble
    })) then
      return true
    end
    return true
  end
  for i = 1, random_range(region_set.map.rng, profile.ridges) do
    if not place_outdoor_ridges() then
      print("RETRY: place_outdoor_ridges()")
      return false
    end
  end
  local place_small_vaults
  place_small_vaults = function()
    for i = 1, random_range(region_set.map.rng, profile.small_vaults) do
      local _continue_0 = false
      repeat
        local enemy_placer
        enemy_placer = function(map, xy)
          local enemy = OldMaps.enemy_generate(OVERWORLD_VAULT_ENEMIES)
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
          if map.rng:chance(.1) then
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
        if not place_vault_in(region_set, Vaults.small_random_vault({
          rng = region_set.map.rng,
          item_placer = item_placer,
          enemy_placer = enemy_placer,
          gold_placer = gold_placer,
          store_placer = store_placer,
          tileset = tileset,
          door_placer = door_placer
        })) then
          _continue_0 = true
          break
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return true
  end
  if not place_small_vaults() then
    print("RETRY: place_small_vaults()")
    return nil
  end
  local place_mini_features
  place_mini_features = function()
    for i, template in ipairs(profile.mini_features) do
      local _continue_0 = false
      repeat
        local enemy_placer
        enemy_placer = function(map, xy)
          local enemy = OldMaps.enemy_generate(OVERWORLD_VAULT_ENEMIES)
          return MapUtils.spawn_enemy(map, enemy, xy)
        end
        local store_placer
        if i ~= 1 and i ~= 4 then
          store_placer = do_nothing
        elseif i == 4 then
          store_placer = function(map, xy)
            return MapUtils.spawn_item(map, "Gold", random(2, 10), xy)
          end
        else
          store_placer = function(map, xy)
            return Region1.generate_store(map, xy)
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
        if not place_vault_in(region_set, template({
          enemy_placer = enemy_placer,
          store_placer = store_placer,
          item_placer = item_placer,
          gold_placer = gold_placer,
          door_placer = door_placer
        })) then
          _continue_0 = true
          break
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return true
  end
  if not place_mini_features() then
    print("RETRY: place_mini_features()")
    return nil
  end
  return true
end
return MapDesc.create({
  map_label = "Plain Valley",
  size = {
    120,
    120
  },
  default_content = Tilesets.grass.wall,
  default_flags = {
    SourceMap.FLAG_SOLID,
    SourceMap.FLAG_SEETHROUGH
  },
  arc_chance = 0.05,
  children = {
    MapNode.create({
      place = function(self)
        local requested_profile = self.desc.generate_hooks and self.desc.generate_hooks.overworld_profile
        local profile = profile_by_name(requested_profile) or self.rng:random_choice(OVERWORLD_PROFILES)
        local number_regions = roll_sum(self.rng, profile.region_base, profile.region_rolls)
        local connect_line_width
        connect_line_width = function()
          return roll_sum(self.rng, profile.connect_base, profile.connect_rolls)
        end
        local room_radius
        room_radius = function()
          return roll_sum(self.rng, profile.room_base, profile.room_rolls)
        end
        local factor = math.sqrt(number_regions / 5)
        local size = {
          profile.size_base * factor,
          profile.size_base * factor
        }
        local template = nilprotect({
          default_wall = Tile.create(Tilesets.grass.wall, true, true, {
            FLAG_OVERWORLD
          }),
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
              n_statues = profile.n_statues
            })
          },
          outer_conf = nilprotect({
            floor1 = OVERWORLD_TILESET.floor1,
            floor2 = OVERWORLD_TILESET.floor2,
            connect_line_width = connect_line_width
          }),
          shell = profile.shell
        })
        if not NewMaps.map_try_create(self.map, self.map.rng, template) then
          return false
        end
        if not overworld_features(self.region_set, profile) then
          return false
        end
        append(self.map.post_map, function()
          if not overworld_items_and_enemies(self.region_set, profile) then
            return false
          end
          return true
        end)
        return true
      end
    })
  }
})
