Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
OldMaps = require "maps.OldMaps"

Vaults = require "maps.Vaults"
{:place_vault_in} = require "maps.VaultUtils"
VaultKit = require "maps.vaults.Kit"
VaultLibrary = require "maps.vaults.Library"

-- Digs the way into dungeon: one of its own entrance vaults (see maps/vaults) or one any dungeon
-- can use. Tries another if the first won't fit.
place_entrance = (region_set, dungeon, args) ->
    used = {}
    args = table.merge args, {
        palette: VaultLibrary.DUNGEON_PALETTES[dungeon]
        dungeon_placer: callable_once(args.dungeon_placer)
    }
    for attempt=1,4
        def = VaultLibrary.pick(region_set.map.rng, "entrance", dungeon, used)
        if not def
            break
        used[def.name] = true
        if VaultKit.place(region_set, def, args)
            return true
    return false

place_ogre_lair = (region_set) ->
    ogre_lair = require("map_descs.OgreLair")\linker()
    return place_entrance region_set, "ogre", {
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

    return place_entrance region_set, "outpost", {
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
            -- Only guard about half of the 'e' squares, the gate should not be a wall of monsters
            if map.rng\chance(.5)
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
            if not place_outpost(node.region_set)
                return false
            if not place_crypt(node.region_set)
                print "RETRY: place_crypt()"
                return false
            if not place_graghs_lair(node.region_set)
                return false
            if not place_pixullochia(node.region_set)
                return false
            return true
    }

    -- MapLinker.create {map_label: "Underdungeon", generate: (backwards) => underdungeon_create(backwards)}
    return place_entrance region_set, "underdungeon", {
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
    return place_entrance region_set, "hive", {
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

    return place_entrance region_set, "temple", {
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
    return place_entrance region_set, "snake", {
        door_placer: (map, xy) ->
            MapUtils.spawn_door(map, xy)
        dungeon_placer: callable_once (map, xy) ->
            portal = MapUtils.spawn_portal(map, xy, "spr_gates.enter_snake")
            entrance\link_portal(portal, "spr_gates.exit_dungeon")
        player_spawn_area: true
    }


-- Overworld layouts. Each game picks one of OVERWORLD_NAMES at random (from the game seed), unless
-- LANARTS_OVERWORLD or M.overworld (eg engine.StartMapGenCheck --overworld) names any layout in
-- OVERWORLDS, which draws no random number. The layouts outside the rotation are kept as starting
-- points for other dungeons.
OVERWORLD_NAMES = {"classic", "ringwood", "farmland", "crossroads", "craters", "spiral", "rootways", "patchwork"}
OVERWORLDS = {
    classic: "map_descs.Overworld"
    wildlands: "map_descs.OverworldNoise"          -- noise: forest, meadow and ridges by elevation
    settled: "map_descs.OverworldSimulation"       -- simulation: forest cleared by settlers over the years
    ringwood: "map_descs.OverworldShapes"          -- shapes: glades ringed around a rock mesa
    badlands: "map_descs.OverworldBadlands"        -- cellular automaton: rock ridges and dry basins
    farmland: "map_descs.OverworldFarmland"        -- binary space partition: fields, hedges and lanes
    terraces: "map_descs.OverworldTerraces"        -- noise contours: stepped terraces with cliffs
    ravine: "map_descs.OverworldRavine"            -- random walks: a rock ravine winding between wooded uplands
    crossroads: "map_descs.OverworldCrossroads"    -- lines and shapes: four roads meeting at a village green
    craters: "map_descs.OverworldCraters"          -- ellipses: an impact field of rock-rimmed craters
    spiral: "map_descs.OverworldSpiral"            -- parametric curve: a road coiling in to a central clearing
    labyrinth: "map_descs.OverworldLabyrinth"      -- maze generation: a hedge maze with open lawns
    canyons: "map_descs.OverworldCanyons"          -- random walks in rock: canyons cut through rock country
    rootways: "map_descs.OverworldRootways"        -- growth automaton: branching, root-like meadows
    patchwork: "map_descs.OverworldPatchwork"      -- Voronoi cells: meadows between hedgerows
    ridges: "map_descs.OverworldRidges"            -- banded noise: parallel ridges with valleys and passes
    ruins: "map_descs.OverworldRuins"              -- grid and rectangles: an overgrown ruined town
}

forced = os.getenv("LANARTS_OVERWORLD")
M = {overworld: (forced ~= "" and forced or nil), :OVERWORLDS, :OVERWORLD_NAMES}

M.generate = () ->
        player_spawn_points = nil
        name = M.overworld or random_choice(OVERWORLD_NAMES)
        M.generated_overworld = name
        desc = OVERWORLDS[name]
        if not desc
            known = [k for k in pairs OVERWORLDS]
            table.sort known
            error "Unknown overworld '#{name}', expected one of: #{table.concat(known, ', ')}"
        overworld = require(desc)\linker {
            root: () =>
                -- A forced temple or snake pit entrance (see maps/vaults/Kit.moon) picks its dungeon
                forced = VaultLibrary.BY_NAME[VaultKit.force_name]
                first_dungeon = if forced and forced.themes[1] == "temple"
                    place_temple
                elseif forced and forced.themes[1] == "snake"
                    place_snake_pit
                else
                    @rng\random_choice {place_temple, place_snake_pit}
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

return M
