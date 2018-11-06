argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"
Map = require "core.Map"
World = require "core.World"

MAX_TIME = 10000000

game_init = (seed, class_name, monster_spawns, input_func, level) ->
    input_source = (player) ->
        player\add_effect("DebugMonsterEvalEffect", MAX_TIME)
        for i=2,level
            player\gain_level()
        source = require("input.ProgrammableInputSource").create(player)
        source.step = input_func
        player.stats.max_hp = 10000
        player.stats.hp = 10000
        return source
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    GameState.register_player 'Playabot', class_name, -- Name and class
        input_source, -- input source
        true, 0, --netid
        0 -- team

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    random_seed(seed)
    Engine.first_map_create = () ->
        return require("maps.0X_Arena").generate_and_spawn(monster_spawns)
    EngineInternal.start_game()

    -- Set up input for first game step
    GameState.input_capture()
    GameState.input_handle()
    GameState.frame = 0

_iterate = (tbl, f, next) ->
    idx = 1
    iterate = () ->
        val = tbl[idx]
        idx += 1
        if val == nil
            return next()
        else
            return f(val, iterate)
    return iterate

CONFIGS = {
    {
        name: "BaseFighter"
        class: "Fighter"
        input: () =>
            @set("should_use_weapon", true)
    }
    {
        name: "BaseNecromancer"
        class: "Necromancer"
        input: () =>
            @set("should_use_weapon", true)
    }
    {
        name: "BaseStormcaller"
        class: "Stormcaller"
        input: () =>
            @set("use_spell_slot", 0)
    }
    {
        name: "BasePyrocaster"
        class: "Pyrocaster"
        input: () =>
            @set("use_spell_slot", 0)
    }
}

dump_stats = (config_name, monster) ->
    GameState = require("core.GameState")
    player = World.players[1].instance
    enemy_hp, enemy_max_hp = 0, 0
    for {:stats} in *Map.enemies_list(player)
        enemy_hp += stats.hp
        enemy_max_hp += stats.max_hp
    { :defender_count, :attacker_count, :melee_count,
      :projectile_count, :receive_melee_count } = player\get_effect("DebugMonsterEvalEffect")
    return {
        :config_name
        level: player.stats.level
        :monster
        frames: GameState.frame
        player_damage: player.stats.max_hp - player.stats.hp
        :enemy_hp, :enemy_max_hp
        :defender_count, :attacker_count, :melee_count,
        :projectile_count, :receive_melee_count
    }

run_bot_tests = (raw_args) ->
    parser = argparse("lanarts", "Run lanarts bot tests.")
    parser\option("--steps", "Maximum number of steps to take.", 10000)
    parser\option("--event_log", "Event log to write to.", nil)
    parser\option("--seed", "Starting seed.", "12345678")
    parser\option("--xp", "Bot XP.", "Fighter")
    parser\option("--comparison_event_log", "Event log to compare to.", nil)
    args = parser\parse(raw_args)

    local game_start, config_eval_step, monster_eval_step, game_step

    game_start = () ->
        return ResourceLoading.ensure_resources_before(config_eval_step)

    game_exit = (msg) ->
        print msg
        return nil

    config_eval_step = () ->
        stats = {}
        level_eval_step = (level, next_level) ->
            return _iterate CONFIGS, monster_eval_step(level, stats), next_level
        return _iterate {1,2,3,4,5,6,7,8,9,10}, level_eval_step, () ->
            file_dump_csv("saves/monster_stats.csv", stats)
            return game_exit("Done.")

    monster_eval_step = (level, stats) -> (config, next_config) ->
        monster_names = table.sorted_key_list(enemies)
        step = (monster_name, next_monster) ->
            print('level', level, 'config', config.name, 'mon', monster_name)
            on_finish = () ->
                append stats, dump_stats(config.name, monster_name)
                return next_monster()
            -- Continue simulation iteration:
            game_init(tonumber(args.seed), config.class, {monster_name}, config.input, level)
            return game_step(on_finish)
        return _iterate monster_names, step, next_config

    game_step = (next_sim) ->
        step = () ->
            -- (1) Load dependencies
            GameState = require("core.GameState")
            GameLoop = require("GameLoop")

            -- (2) Update with frame stats
            step_only = (GameState.frame % 100 ~= 0)
            if #Map.enemies_list(World.players[1].instance) == 0
                return next_sim()
            -- (3) Check if we are at final frame:
            if args.steps and GameState.frame >= tonumber(args.steps)
                return next_sim()

            -- (4) Run game loop & and see if game
            -- should be saved + exited
            if not GameLoop.game_step(step_only)
                return next_sim()

            -- (5) Loop again
            return step
        require("Events").events.PlayerDeath = () ->
            step = next_sim
            return false
        EngineHooks.game_won = () ->
            -- Exit on next step
            step = next_sim
        return step()

    return StartEngine.start_engine {
        settings: { -- Settings object
            -- Window settings
            fullscreen: false
            view_width: 640
            view_height: 480
            -- Font settings
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: false
            regen_on_death: false
            event_log: args.event_log
            comparison_event_log: args.comparison_event_log
        },
        entry_point: game_start,
        debug: args.debug,
        on_exit: () -> nil
    }

return run_bot_tests
