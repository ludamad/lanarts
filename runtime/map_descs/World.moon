Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
OldMaps = require "maps.OldMaps"

Vaults = require "maps.Vaults"
{:place_vault_in} = require "maps.VaultUtils"

place_ogre_lair = (region_set) ->
    ogre_lair = require("map_descs.OgreLair")\linker()
    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.snake
        gold_placer: (map_, xy) ->
            MapUtils.spawn_item(map_, "Gold", map_.rng\random(2,10), xy)
        door_placer: (map_, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map_, xy, nil, Vaults._door_key1, 'Azurite Key')
        dungeon_placer: callable_once (map_, xy) ->
            portal = MapUtils.spawn_portal(map_, xy, "spr_gates.enter_orc")
            ogre_lair\link_portal(portal, "spr_gates.exit_orc")
    }

place_outpost = (region_set) ->
    entrance = require("map_descs.OutpostEntrance")\linker {
        root: (node) -> place_ogre_lair node.region_set
    }
    stockroom = require("map_descs.OutpostStockroom")\linker()

    -- Link entrance & stockroom
    for i=1,3
        entrance\link_linker(stockroom, "spr_gates.enter", "spr_gates.return")

    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.snake
        gold_placer: (map, xy) -> nil -- dont need gold here
        door_placer: (map, xy) -> MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.desolation_portal")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
    }

-----------------------------
-- Place optional dungeon 2, the crypt: --
place_crypt = (region_set) ->
    crypt = require("map_descs.Crypt")\linker()
    hell = require("map_descs.Hell")\linker()
    crypt\link_linker(hell, 'spr_gates.enter_hell3', 'spr_gates.return_hell')
    return place_vault_in region_set, Vaults.crypt_dungeon {
        tileset: Tilesets.crypt
        door_placer: (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_crypt")
            crypt\link_portal(portal, 'spr_gates.exit_dungeon')
        enemy_placer: (map, xy) ->
            enemy = OldMaps.enemy_generate(OldMaps.strong_undead)
            MapUtils.spawn_enemy(map, enemy, xy)
    }

place_pixullochia = (region_set) ->
    entrance = require("map_descs.PixullochiaEntrance")\linker()
    sanctum = require("map_descs.PixullochiaDepths")\linker()
    for i=1,3
        entrance\link_linker(sanctum, "spr_gates.enter", "spr_gates.return")

    return place_vault_in region_set, Vaults.hell_dungeon {
        tileset: Tilesets.pixulloch
        enemy_placer: (map, xy) ->
            enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
            MapUtils.spawn_enemy(map, enemy, xy)
        door_placer: (map, xy) -> MapUtils.spawn_lanarts_door(map, xy)
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.zig_portal")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
    }

-----------------------------
-- Gragh's lair            --
place_graghs_lair = (region_set) ->
    lair = require("map_descs.GraghsLair")\linker()

    return place_vault_in region_set, Vaults.graghs_lair_entrance {
        tileset: Tilesets.lair
        dungeon_placer: (map, xy) ->
            -- Make portal
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_slime")
            lair\link_portal(portal, 'spr_gates.exit_dungeon')
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
        enemy_placer: (map, xy) ->
            enemy = OldMaps.enemy_generate({{enemy: "Sheep", chance: 100}})
            MapUtils.spawn_enemy(map, enemy, xy)
    }

place_underdungeon = (region_set) ->
    underdungeon =  require("map_descs.Underdungeon")\linker {
        root: (node) ->
            -- if not place_outpost(node.region_set)
            --     return false
            -- if not place_crypt(node.region_set)
            --     print "RETRY: place_crypt()"
            --     return false
            -- if not place_graghs_lair(node.region_set)
            --     return false
            -- if not place_pixullochia(node.region_set)
            --     return false
            return true
    }

    -- MapLinker.create {map_label: "Underdungeon", generate: (backwards) => underdungeon_create(backwards)}
    return place_vault_in region_set, Vaults.sealed_dungeon {
        rng: region_set.map.rng,
        tileset: Tilesets.snake
        door_placer: (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, 'Dandelite Key')
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_vaults_open")
            underdungeon\link_portal(portal, "spr_gates.exit_dungeon")
    }

-------------------------------------------------------------------------------
-- Game phase 1
-------------------------------------------------------------------------------

place_hive = (region_set) ->
    entrance = require("map_descs.HiveEntrance")\linker()
    -- Link entrance to further depths
    depths = require("map_descs.HiveDepths")\linker()
    for i=1,3
        entrance\link_linker(depths, "spr_gates.enter_lair", "spr_gates.exit_lair")
    -- Link to hive entrance
    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.hive
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
        dungeon_placer: (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.hive_portal")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
    }

place_temple = (region_set) ->
    entrance = require("map_descs.TempleEntrance")\linker()
    chamber = require("map_descs.TempleChamber")\linker()
    sanctum = require("map_descs.TempleSanctum")\linker()
    for i=1,3
        entrance\link_linker(chamber, "spr_gates.enter", "spr_gates.return")
        chamber\link_linker(sanctum, "spr_gates.enter", "spr_gates.return")

    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.temple
        gold_placer: (map, xy) -> nil -- dont need gold here
        door_placer: (map, xy) -> MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_temple")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
        player_spawn_area: true
    }

place_snake_pit = (region_set) ->
    entrance = require("map_descs.SnakePitEntrance")\linker()
    -- Link entrance to further depths
    depths = require("map_descs.SnakePitDepths")\linker()
    for i=1,3
        entrance\link_linker(depths, "spr_gates.enter", "spr_gates.return")
    -- Link to snake pit entrance
    return place_vault_in region_set, Vaults.sealed_dungeon {
        tileset: Tilesets.snake
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_snake")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
        player_spawn_area: true
    }

return {
    generate: () ->
        player_spawn_points = nil
        overworld = require("map_descs.Overworld")\linker {
            root: () =>
                first_dungeon = @rng\random_choice {place_temple, place_snake_pit}
                if not first_dungeon(@region_set)
                    return false
                if not place_hive(@region_set)
                    return false
                if not place_underdungeon(@region_set)
                    return false
                player_spawn_points = MapUtils.pick_player_squares(@map, @map.player_candidate_squares)
                assert player_spawn_points, "Could not pick player spawn squares!"
                return true
        }
        game_map = overworld\get()
        require("core.World").players_spawn(game_map, player_spawn_points)
        return game_map
}
