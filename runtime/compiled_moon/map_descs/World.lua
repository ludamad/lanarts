local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local OldMaps = require("maps.OldMaps")
local Vaults = require("maps.Vaults")
local place_vault_in
place_vault_in = require("maps.VaultUtils").place_vault_in
local place_ogre_lair
place_ogre_lair = function(region_set)
  local ogre_lair = require("map_descs.OgreLair"):linker()
  return place_vault_in(region_set, Vaults.sealed_dungeon({
    tileset = Tilesets.snake,
    gold_placer = function(map_, xy)
      return MapUtils.spawn_item(map_, "Gold", map_.rng:random(2, 10), xy)
    end,
    door_placer = function(map_, xy)
      return MapUtils.spawn_door(map_, xy, nil, Vaults._door_key1, 'Azurite Key')
    end,
    dungeon_placer = callable_once(function(map_, xy)
      local portal = MapUtils.spawn_portal(map_, xy, "spr_gates.enter_orc")
      return ogre_lair:link_portal(portal, "spr_gates.exit_orc")
    end)
  }))
end
local place_outpost
place_outpost = function(region_set)
  local entrance = require("map_descs.OutpostEntrance"):linker({
    root = function(node)
      return place_ogre_lair(node.region_set)
    end
  })
  local stockroom = require("map_descs.OutpostStockroom"):linker()
  for i = 1, 3 do
    entrance:link_linker(stockroom, "spr_gates.enter", "spr_gates.return")
  end
  return place_vault_in(region_set, Vaults.sealed_dungeon({
    tileset = Tilesets.snake,
    gold_placer = function(map, xy)
      return nil
    end,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy)
    end,
    dungeon_placer = callable_once(function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.desolation_portal")
      return entrance:link_portal(portal, "spr_gates.exit_dungeon")
    end)
  }))
end
local place_crypt
place_crypt = function(region_set)
  local crypt = require("map_descs.Crypt"):linker()
  local hell = require("map_descs.Hell"):linker()
  crypt:link_linker(hell, 'spr_gates.enter_hell3', 'spr_gates.return_hell')
  return place_vault_in(region_set, Vaults.crypt_dungeon({
    tileset = Tilesets.crypt,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
    end,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_crypt")
      return crypt:link_portal(portal, 'spr_gates.exit_dungeon')
    end,
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate(OldMaps.strong_undead)
      return MapUtils.spawn_enemy(map, enemy, xy)
    end
  }))
end
local place_pixullochia
place_pixullochia = function(region_set)
  local entrance = require("map_descs.PixullochiaEntrance"):linker()
  local sanctum = require("map_descs.PixullochiaDepths"):linker()
  for i = 1, 3 do
    entrance:link_linker(sanctum, "spr_gates.enter", "spr_gates.return")
  end
  return place_vault_in(region_set, Vaults.hell_dungeon({
    tileset = Tilesets.pixulloch,
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
      return MapUtils.spawn_enemy(map, enemy, xy)
    end,
    door_placer = function(map, xy)
      return MapUtils.spawn_lanarts_door(map, xy)
    end,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.zig_portal")
      return entrance:link_portal(portal, "spr_gates.exit_dungeon")
    end
  }))
end
local place_graghs_lair
place_graghs_lair = function(region_set)
  local lair = require("map_descs.GraghsLair"):linker()
  return place_vault_in(region_set, Vaults.graghs_lair_entrance({
    tileset = Tilesets.lair,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_slime")
      return lair:link_portal(portal, 'spr_gates.exit_dungeon')
    end,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
    end,
    enemy_placer = function(map, xy)
      local enemy = OldMaps.enemy_generate({
        {
          enemy = "Sheep",
          chance = 100
        }
      })
      return MapUtils.spawn_enemy(map, enemy, xy)
    end
  }))
end
local place_underdungeon
place_underdungeon = function(region_set)
  local underdungeon = require("map_descs.Underdungeon"):linker({
    root = function(node)
      if not place_outpost(node.region_set) then
        return false
      end
      if not place_crypt(node.region_set) then
        print("RETRY: place_crypt()")
        return false
      end
      if not place_graghs_lair(node.region_set) then
        return false
      end
      if not place_pixullochia(node.region_set) then
        return false
      end
      return true
    end
  })
  return place_vault_in(region_set, Vaults.sealed_dungeon({
    rng = region_set.map.rng,
    tileset = Tilesets.underdungeon,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, 'Dandelite Key')
    end,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_vaults_open")
      return underdungeon:link_portal(portal, "spr_gates.exit_dungeon")
    end
  }))
end
local place_hive
place_hive = function(region_set)
  local entrance = require("map_descs.HiveEntrance"):linker()
  local depths = require("map_descs.HiveDepths"):linker()
  for i = 1, 3 do
    entrance:link_linker(depths, "spr_gates.enter_lair", "spr_gates.exit_lair")
  end
  return place_vault_in(region_set, Vaults.sealed_dungeon({
    tileset = Tilesets.hive,
    door_placer = function(map, xy)
      return MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
    end,
    dungeon_placer = function(map, xy)
      local portal = MapUtils.spawn_portal(map, xy, "spr_gates.hive_portal")
      return entrance:link_portal(portal, "spr_gates.exit_dungeon")
    end
  }))
end
local place_temple
place_temple = function(region_set)
  local entrance = require("map_descs.TempleEntrance"):linker()
  local chamber = require("map_descs.TempleChamber"):linker()
  local sanctum = require("map_descs.TempleSanctum"):linker()
  for i = 1, 3 do
    entrance:link_linker(chamber, "spr_gates.enter", "spr_gates.return")
    chamber:link_linker(sanctum, "spr_gates.enter", "spr_gates.return")
  end
  return place_vault_in(region_set, Vaults.sealed_dungeon({
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
end
local place_snake_pit
place_snake_pit = function(region_set)
  local entrance = require("map_descs.SnakePitEntrance"):linker()
  local depths = require("map_descs.SnakePitDepths"):linker()
  for i = 1, 3 do
    entrance:link_linker(depths, "spr_gates.enter", "spr_gates.return")
  end
  return place_vault_in(region_set, Vaults.sealed_dungeon({
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
end
return {
  generate = function()
    local player_spawn_points = nil
    local overworld = require("map_descs.Overworld"):linker({
      root = function(self)
        local first_dungeon = self.rng:random_choice({
          place_temple,
          place_snake_pit
        })
        if not first_dungeon(self.region_set) then
          return false
        end
        if not place_hive(self.region_set) then
          return false
        end
        if not place_underdungeon(self.region_set) then
          return false
        end
        player_spawn_points = MapUtils.pick_player_squares(self.map, self.map.player_candidate_squares)
        assert(player_spawn_points, "Could not pick player spawn squares!")
        return true
      end
    })
    local game_map = overworld:get()
    require("core.World").players_spawn(game_map, player_spawn_points)
    return game_map
  end
}
