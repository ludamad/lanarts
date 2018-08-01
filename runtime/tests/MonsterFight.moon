GameState = require "core.GameState"
World = require "core.World"
GameObject = require "core.GameObject"
PathFinding = require "core.PathFinding"
Map = require "core.Map"
ExploreUtils = require "tests.ExploreUtils"
World = require "core.World"

HARDCODED_AI_SEED = 1234567800

import Spread, Shape
    from require "maps.MapElements"

MonsterFightRoom = () -> newtype {
    parent: require("maps.MapCompiler").MapCompiler
    root_node: Shape {
            shape: 'deformed_ellipse'
            size: {20, 20}
        }
    tileset: require("tiles.Tilesets").lair
    -- Called before compile() is called
    generate: (args) =>
        MapUtils = require "maps.MapUtils"
        for enemy, amount in spairs args.enemies
            for i=1,amount
                sqr = MapUtils.random_square(@map, nil)
                -- Team 1
                MapUtils.spawn_enemy(@map, enemy, sqr, 1)
        for enemy, amount in spairs args.enemies
            for i=1,amount
                sqr = MapUtils.random_square(@map, nil)
                -- Team 2
                MapUtils.spawn_enemy(@map, enemy, sqr, 2)
}

PROGRESSION = () ->
    idx = 0
    fs = {}
    init = () ->
        random_seed(1000) -- TEST_SEED + math.random() * 1000000 * 2)
        template = MonsterFightRoom()
        for enemy, iattributes in spairs enemies
            --if not enemy\match "Dragon"
            --    continue
            append fs, () ->
                P = require "maps.Places"
                map = P.create_isolated {
                    label: enemy
                    :template
                    items: {} -- random_items()
                    enemies: {[enemy]: 10}
                }
                World.set_level(map)
                return map
    return () ->
        init()
        init = do_nothing
        idx += 1
        if idx > #fs
            os.exit()
        return fs[idx]()

sim = (k) ->
    Keyboard = require "core.Keyboard"
    print "SIMULATING ", k
    GameState._simulate_key_press(Keyboard[k])
user_input_capture = GameState.input_capture
user_input_handle = GameState.input_handle
user_gamestate_step = GameState.step
user_player_input = false
ASTAR_BUFFER = PathFinding.astar_buffer_create()


M = {}
M.progger = PROGRESSION()
M.create_player = () -> nilprotect {
    simulate: (k) => sim k
    _n_inputs: 0
    -- Forwarding to handle events from a variety of modules, acts as a pseudo-require:
    intercept: (str) =>
        itable = nilprotect {}
        for k,v in pairs @
            itable[k] = (...) -> v(@, ...)
        return itable
    -- START EVENTS --
    player_has_won: () => false
    trigger_event: (event, ...) =>
        print '*** EVENT:', event
        if event == "PlayerDeath"
            if (@_n_same_square >= 100)
                return true
            dead = require("Events").events.PlayerDeath(...)
            if dead
                @input_source = false
            require("core.GlobalData").n_lives = 100
            return dead
        return true
    overworld_create: () => M.progger()
    -- END EVENTS --
    -- Default is to always want to simulate:
    should_simulate_input: () => true
    simulate_menu_input: () =>
        Keyboard = require "core.Keyboard"
        switch @_n_inputs
            when 0
                sim 'n'
            when 1 -- Mage
                sim 'TAB'
            when 2  -- Fighter
                sim 'TAB'
            when 3 -- Ranger
                sim 'TAB'
            --when 5 -- Necro
            --    sim 'TAB'
            else
                sim 'ENTER'
        @_n_inputs += 1
    menu_start: () =>
        print "MENU START"
        GameState.input_capture = () ->
            if @should_simulate_input()
                GameState._input_clear()
            --    return true
            --else
                x = user_input_capture()
                @simulate_menu_input()
                return x
    game_start: () =>
        pid = 1
        user_player_input or= Engine.player_input
        GameState.input_capture = user_input_capture
        Engine.player_input = (player) ->
            if not rawget @, "input_source"
                @input_source = (require "input.ProgrammableInputSource").create(player)
                return @input_source
            else
                --@input_source = (require "input.ProgrammableInputSource").create(player)
                player.input_source = @input_source
                @input_source.player = player
                return @input_source
                --log_info "Calling user_player_input(player)"
                --return user_player_input(player)
        GameState.step = (...) ->
            if @_should_end()
                GlobalData.__test_initialized = false
                @_n_inputs = 0 -- Reset menu state TODO refactor
                @_n_same_square = 0
                @input_source.player = false
                @input_source = false
                GameState.lazy_reset()
                return true
            should_continue = user_gamestate_step(...)
            RATE = 125
            if GameState.frame % RATE == 2
                --state = {}
                --for k,v in pairs(@)
                --    state[k] = v
                @_lpx = 0
                @_lpy = 0
                @_used_portals = {}
                @_queued = {}
                GlobalData.__test_initialized = false
                if not os.getenv "DISABLE_SAVING"
                    @input_source = false
                    @_ai_state = false
                    GameState.save("saves/test-save.save")
                    GameState.load("saves/test-save.save")
                    -- Copy over state:W
                    GlobalData = require("core.GlobalData")
                --for k,v in pairs state
                --    @[k] = v
            return should_continue
        Engine.io = () ->
            --if rawget @, "input_source"
            --    @input_source\reset()
            --    @simulate_game_input()

    _should_end: () =>
        return false
}

return M
