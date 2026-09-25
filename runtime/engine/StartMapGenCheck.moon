argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"

run = (raw_args) ->
    parser = argparse("lanarts", "Map gen check")
    parser\option("--seed", "Starting seed.", "12345678")
    parser\option("--players", "Players.", "1")
    args = parser\parse(raw_args)
    game_start = () ->
        return ResourceLoading.ensure_resources_before () ->
            GameState = require("core.GameState")
            GameState.clear_players()
            for i=1,tonumber(args.players)
                GameState.register_player 'Bot' .. i, "Fighter",
                    (require("ai.BotInputSource").create),
                    true, i - 1, 0
            EngineInternal.init_gamestate()
            random_seed(tonumber(args.seed))

            MapUtils = require "maps.MapUtils"
            {:MapLinker} = require "maps.MapLink"
            current = {"?"}
            tallies = {}
            order = {}
            tally = (kind, name) ->
                label = current[1]
                if not tallies[label]
                    tallies[label] = {enemies: 0, items: 0, doors: 0, decor: 0, names: {}}
                    append order, label
                t = tallies[label]
                t[kind] += 1
                if kind == "enemies" or (kind == "items" and type(name) == "string" and name\find("Lanart"))
                    t.names[name] = (t.names[name] or 0) + 1
            for {fname, kind} in *{{"spawn_enemy", "enemies"}, {"spawn_item", "items"}, {"spawn_door", "doors"}, {"spawn_lanarts_door", "doors"}, {"spawn_decoration", "decor"}}
                orig = MapUtils[fname]
                MapUtils[fname] = (map, a, ...) ->
                    tally(kind, a)
                    return orig(map, a, ...)
            linkers = {}
            orig_create = MapLinker.create
            MapLinker.create = (...) ->
                l = orig_create(...)
                append linkers, l
                return l
            orig_get = MapLinker.get
            MapLinker.get = (self) ->
                prev = current[1]
                current[1] = self\label()
                -- The global RNG ends up in the same state for every world seed after the overworld is made,
                -- so reseed per level from (seed, label) to sample varied compositions.
                h = 0
                for c in current[1]\gmatch(".")
                    h = (h * 31 + c\byte()) % 1000003
                random_seed((tonumber(args.seed) * 7919 + h) % 2147483647)
                -- LEVEL lines let log readers attribute the engine's retry messages to a level
                print "LEVEL " .. current[1]
                ret = orig_get(self)
                current[1] = prev
                print "LEVEL " .. prev
                return ret
            -- Attribute gate-guard spawns (placed while generating the Underdungeon) to the dungeon they guard
            Vaults = require "maps.Vaults"
            Tilesets = require "tiles.Tilesets"
            for vname in *{"crypt_dungeon", "hell_dungeon", "graghs_lair_entrance"}
                orig_vault = Vaults[vname]
                Vaults[vname] = (vargs) ->
                    if vargs.enemy_placer
                        gate = vname
                        if vname == "graghs_lair_entrance"
                            gate = if vargs.tileset == Tilesets.hell then "dragon_gate" else "gragh_gate"
                        placer = vargs.enemy_placer
                        vargs.enemy_placer = (...) ->
                            prev = current[1]
                            current[1] = prev .. "|GATE:" .. gate
                            ret = placer(...)
                            current[1] = prev
                            return ret
                    return orig_vault(vargs)
            NewMaps = require "maps.NewMaps"
            orig_try = NewMaps.try_n_times
            NewMaps.try_n_times = (n, f) ->
                mark = #linkers
                return orig_try n, (...) ->
                    -- Drop linkers created by failed attempts, they are unreachable
                    while #linkers > mark
                        table.remove(linkers)
                    -- Only count the final (successful) attempt
                    label = current[1]
                    for k, _ in pairs tallies
                        if k == label or k\find(label .. "|", 1, true) == 1
                            tallies[k] = {enemies: 0, items: 0, doors: 0, decor: 0, names: {}}
                    return f(...)
            current[1] = "Plain Valley"
            ok, err = pcall () -> require("map_descs.World").generate()
            if not ok
                print "WORLD GEN ERROR", err
                os.exit(1)
            i = 1
            while i <= #linkers
                ok, err = pcall () -> linkers[i]\get()
                if not ok
                    print "GEN ERROR", linkers[i]\label(), err
                    os.exit(1)
                i += 1
            for label in *order
                t = tallies[label]
                names = {}
                for k, v in pairs t.names
                    append names, k .. "=" .. v
                table.sort names
                print string.format("RESULT seed=%s players=%s | %-24s enemies=%3d items=%3d doors=%3d decor=%3d | %s", args.seed, args.players, label, t.enemies, t.items, t.doors, t.decor, table.concat(names, ","))
            os.exit(0)

    return StartEngine.start_engine {
        settings: {
            fullscreen: false
            view_width: 640
            view_height: 480
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: true
        },
        entry_point: game_start,
        on_exit: () -> nil
    }

return run
