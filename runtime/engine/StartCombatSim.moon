-- Headless combat simulation: bot party of P players (given class + level) vs a set of monsters in DebugArena.
-- Jobs file lines: class level players seed monster[:count][;monster[:count]...]
argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Map = require "core.Map"
World = require "core.World"

BIG_HP = 100000
ORIGINAL = {}

make_step = (class_name) -> () =>
    player = @player
    enemies = Map.enemies_list(player)
    -- Head for the nearest originally-spawned monster (like a player would go for a summoner), else the nearest summon
    best, best_d = nil, math.huge
    for pass in *{true, false}
        for e in *enemies
            if (ORIGINAL[e.id] ~= nil) == pass
                d = math.sqrt((e.x - player.x)^2 + (e.y - player.y)^2)
                if d < best_d
                    best, best_d = e, d
        if best
            break
    if not best
        return
    -- Casters close in and use their weapon when out of mana
    melee = (class_name == "Fighter" or class_name == "Rogue" or class_name == "Templar" or player.stats.mp < 10)
    want = if melee then (player.radius + best.radius + 4) else 110
    if best_d > want
        dx, dy = best.x - player.x, best.y - player.y
        @set("move_direction", {dx / best_d, dy / best_d})
    if melee
        @set("should_use_weapon", true)
    else
        -- Caster: use first spell when possible, else weapon
        @set("use_spell_slot", 0)

game_init = (job) ->
    GameState = require("core.GameState")
    GameState.clear_players()
    job.maxhp = {}
    for i=1,job.players
        input_source = (player) ->
            for _=2,job.level
                player\gain_level()
            append job.maxhp, player.stats.max_hp
            source = require("input.ProgrammableInputSource").create(player)
            source.step = make_step(job.class)
            player.stats.max_hp = BIG_HP
            player.stats.hp = BIG_HP
            return source
        GameState.register_player 'Sim' .. i, job.class, input_source, true, i - 1, 0
    EngineInternal.init_gamestate()
    random_seed(job.seed)
    Engine.first_map_create = () ->
        return require("map_descs.DebugArena")\generate_and_spawn(job.monsters)
    EngineInternal.start_game()
    GameState.input_capture()
    GameState.input_handle()
    GameState.frame = 0
    ORIGINAL = {}
    for e in *Map.enemies_list(World.players[1].instance)
        ORIGINAL[e.id] = true

report = (job) ->
    GameState = require("core.GameState")
    dmg, xp = {}, {}
    for p in *World.players
        append dmg, BIG_HP - p.instance.stats.hp
        append xp, p.instance.stats.level .. ":" .. p.instance.stats.xp
    enemy_hp, enemy_max_hp, n_left = 0, 0, 0
    for e in *Map.enemies_list(World.players[1].instance)
        if ORIGINAL[e.id]
            enemy_hp += math.max(0, e.stats.hp)
            enemy_max_hp += e.stats.max_hp
            n_left += 1
    print string.format("SIM class=%s level=%d players=%d seed=%d mon=%s frames=%d maxhp=%s dmg=%s left=%d left_hp=%d/%d xp=%s",
        job.class, job.level, job.players, job.seed, job.spec, GameState.frame,
        table.concat(job.maxhp, "/"), table.concat(dmg, "/"), n_left, enemy_hp, enemy_max_hp, table.concat(xp, "/"))
    io.stdout\flush()

parse_jobs = (fname) ->
    jobs = {}
    for line in io.lines(fname)
        cls, lvl, pl, seed, spec = line\match("^(%S+)%s+(%d+)%s+(%d+)%s+(%d+)%s+(.+)$")
        if cls
            monsters = {}
            for part in spec\gmatch("[^;]+")
                name, count = part\match("^(.-):(%d+)$")
                if not name
                    name, count = part, 1
                for _=1,tonumber(count)
                    append monsters, name
            append jobs, {class: cls, level: tonumber(lvl), players: tonumber(pl), seed: tonumber(seed), :spec, :monsters}
    return jobs

run = (raw_args) ->
    parser = argparse("lanarts", "Combat sim")
    parser\option("--jobs", "Jobs file.")
    parser\option("--steps", "Max frames per fight.", "5000")
    args = parser\parse(raw_args)
    jobs = parse_jobs(args.jobs)
    max_steps = tonumber(args.steps)
    idx = 0
    local next_job
    fight_step = (job) ->
        GameLoop = require("GameLoop")
        GameState = require("core.GameState")
        step = nil
        step = () ->
            -- Done once every originally-spawned monster is dead (left-over summons expire on their own)
            n_original, n_all = 0, 0
            for e in *Map.enemies_list(World.players[1].instance)
                n_all += 1
                if ORIGINAL[e.id]
                    n_original += 1
            if n_original == 0 and not job.originals_dead_frame
                job.originals_dead_frame = GameState.frame
            summons_expired = job.originals_dead_frame and GameState.frame > job.originals_dead_frame + 250
            if n_all == 0 or summons_expired or GameState.frame >= max_steps
                report(job)
                return next_job()
            if not GameLoop.game_step(true)
                report(job)
                return next_job()
            return step
        return step
    next_job = () ->
        idx += 1
        job = jobs[idx]
        if not job
            print "SIM DONE"
            os.exit(0)
        game_init(job)
        return fight_step(job)
    EngineHooks.game_won = () -> nil
    patch_spawns = () ->
        MapUtils = require "maps.MapUtils"
        SourceMap = require "core.SourceMap"
        orig_pick = MapUtils.pick_player_squares
        MapUtils.pick_player_squares = (map, positions) ->
            tries = 0
            while #positions < World.player_amount and tries < 1000
                tries += 1
                sqr = MapUtils.random_square(map, nil, {matches_none: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}})
                if sqr
                    append positions, {sqr[1]*32+16, sqr[2]*32+16}
            return orig_pick(map, positions)

        return next_job()
    game_start = () -> ResourceLoading.ensure_resources_before(patch_spawns)
    return StartEngine.start_engine {
        settings: {
            fullscreen: false
            view_width: 640
            view_height: 480
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: false
            regen_on_death: false
        },
        entry_point: game_start,
        on_exit: () -> nil
    }

return run
