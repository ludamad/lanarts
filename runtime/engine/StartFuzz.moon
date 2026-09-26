-- Fuzzes the game with bots (ai/BotInputSource): a party of bots plays from a fresh world or a save,
-- now and then jumping to a random portal to spread across the levels, and saving the game at
-- random frames. Any Lua error ends the run with a FUZZ ERROR line and exit code 1. Used by
-- tools/fuzz/fuzz.sh:
--   ./lanarts engine.StartFuzz --seed N [--players P] [--classes Fighter,Rogue] [--overworld NAME]
--       [--steps N] [--load FILE] [--save FILE]
-- Prints FUZZ lines: START, one POS line per bot every --report frames, SAVE, and DONE.
argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"

CLASSES = {"Fighter", "Rogue", "Templar", "Pyrocaster", "Stormcaller", "Blue Mage", "Necromancer"}

run = (raw_args) ->
    parser = argparse("lanarts", "Fuzz the game with bots.")
    parser\option("--seed", "World and fuzz seed.", "1")
    parser\option("--players", "Bots in the party.", "1")
    parser\option("--classes", "Comma-separated classes, one per bot; random from the seed if not given.")
    parser\option("--overworld", "Overworld layout (see map_descs/World.moon); random if not given.")
    parser\option("--steps", "Frames to run.", "20000")
    parser\option("--report", "Frames between POS lines.", "2000")
    parser\option("--jump", "Chance per frame that a bot jumps to a random portal.", "0.0005")
    parser\option("--save_chance", "Chance per frame of saving to --save.", "0.0003")
    parser\option("--load", "Save file to start from.")
    parser\option("--save", "Save file for random saves, and the final save.")
    parser\option("--bench_astar", "Instead of playing, time this many A* searches between random open tiles of the first map.")
    args = parser\parse(raw_args)
    seed = tonumber(args.seed)
    fuzz_rng = require("mtwist").create(seed * 7919 + 17)

    fail = (err) ->
        print "FUZZ ERROR #{err}"
        os.exit(1)

    local game_step, bench_astar
    guarded = (f) -> (...) ->
        results = {xpcall(f, debug.traceback)}
        if not results[1]
            fail(results[2])
        return unpack(results, 2)

    game_start = () ->
        return ResourceLoading.ensure_resources_before guarded () ->
            GameState = require "core.GameState"
            GameState.clear_players()
            classes = if args.classes then args.classes\split(",") else [fuzz_rng\random_choice(CLASSES) for i=1,tonumber(args.players)]
            for i, class_name in ipairs classes
                GameState.register_player "Bot#{i}", class_name, (require("ai.BotInputSource").create), true, i - 1, 0
            if args.overworld
                require("map_descs.World").overworld = args.overworld
            EngineInternal.init_gamestate()
            random_seed(seed)
            if args.load
                GameState.load(args.load)
            else
                EngineInternal.start_game()
            print "FUZZ START seed=#{seed} classes=#{table.concat(classes, ',')} overworld=#{require('map_descs.World').generated_overworld or '?'} load=#{args.load or '-'}"
            if args.bench_astar
                bench_astar(tonumber(args.bench_astar))
            GameState.input_capture()
            if not GameState.input_handle()
                fail("game exited at start")
            return game_step()

    -- Half the searches span the whole map (as the bot's and monsters' long wanders do), half a
    -- region around both ends (as monster pathing does)
    bench_astar = (n) ->
        World = require "core.World"
        Map = require "core.Map"
        PathFinding = require "core.PathFinding"
        map = World.players[1].instance.map
        {w, h} = Map.size(map)
        rng = require("mtwist").create(seed)
        random_open = () ->
            while true
                xy = {rng\random(0, w), rng\random(0, h)}
                if not Map.tile_is_solid(map, xy)
                    return xy
        buffer = PathFinding.astar_buffer_create()
        for {name, region} in *{{"full", (a, b) -> nil}, {"local", (a, b) -> {math.min(a[1], b[1]) - 3, math.min(a[2], b[2]) - 3, math.max(a[1], b[1]) + 4, math.max(a[2], b[2]) + 4}}}
            pairs = [{random_open(), random_open()} for i=1,n]
            found, steps = 0, 0
            start = os.clock()
            for {a, b} in *pairs
                path = if region(a, b) then buffer\calculate_path(map, a, b, region(a, b)) else buffer\calculate_path(map, a, b)
                if #path > 1
                    found += 1
                    steps += #path
            ms = (os.clock() - start) * 1000
            print string.format("ASTAR %s map=%dx%d searches=%d found=%d steps=%d total_ms=%.1f per_search_ms=%.3f", name, w, h, n, found, steps, ms, ms / n)
        os.exit(0)

    report = () ->
        GameState = require "core.GameState"
        World = require "core.World"
        Map = require "core.Map"
        for {:instance} in *World.players
            {x, y} = instance.xy
            print string.format("FUZZ POS frame=%d %s %s map=%q xy=%d,%d hp=%d/%d",
                GameState.frame, instance.name, instance.class_name, Map.map_label(instance.map), x, y, instance.stats.hp, instance.stats.max_hp)

    save = (why) ->
        GameState = require "core.GameState"
        GameState.save(args.save)
        print "FUZZ SAVE frame=#{GameState.frame} #{why}"

    finish = (why) ->
        report()
        if args.save
            save("final")
        print "FUZZ DONE frame=#{require('core.GameState').frame} #{why}"
        os.exit(0)

    game_step = () ->
        EngineHooks.game_won = () -> finish("won")
        local step
        step = guarded () ->
            GameState = require "core.GameState"
            GameLoop = require "GameLoop"
            World = require "core.World"
            frame = GameState.frame
            if frame % tonumber(args.report) == 0
                report()
            if frame >= tonumber(args.steps)
                finish("steps")
            for {:instance} in *World.players
                source = instance.input_source
                if source and source._goto_random_portal and fuzz_rng\randomf() < tonumber(args.jump)
                    source\_goto_random_portal()
            if args.save and fuzz_rng\randomf() < tonumber(args.save_chance)
                save("random")
            if not GameLoop.game_step(true)
                finish("game loop exited")
            return step
        return step()

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
