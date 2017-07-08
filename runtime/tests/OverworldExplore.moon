GameState = require "core.GameState"
GlobalData = require "core.GlobalData"
World = require "core.World"
GameObject = require "core.GameObject"
PathFinding = require "core.PathFinding"
Map = require "core.Map"
ExploreUtils = require "tests.ExploreUtils"
World = require "core.World"

HARDCODED_AI_SEED = 1234567800

sim = (k) ->
    Keyboard = require "core.Keyboard"
    print "SIMULATING ", k
    GameState._simulate_key_press(Keyboard[k])
user_input_capture = GameState.input_capture
user_input_handle = GameState.input_handle
user_gamestate_step = GameState.step
user_player_input = false
ASTAR_BUFFER = PathFinding.astar_buffer_create()

random_enemies = () ->
    enemylist = (for k,v in pairs enemies do v)
    ret = {}
    for i=1,10
        enemy = random_choice(enemylist)
        ret[enemy.name] = 1
    return ret

random_items = () ->
    itemlist = (for k,v in pairs items do v)
    ret = {}
    for i=1,10
        while true
            item = random_choice(itemlist)
            ret[item.name] = 1
            if item.is_randart
                continue -- TODO test all these much later. mostly boring effects
            if item.spr_item == 'none' -- Not meant to be picked up
                continue
    return ret

PROGRESSION = () ->
    idx = 0
    fs = {}
    init = () ->
        random_seed(1000) -- TEST_SEED + math.random() * 1000000 * 2)
        for item, attributes in pairs items
            if not attributes.types or #attributes.types < 2
                continue
            --if attributes.is_randart
            --    continue -- TODO test all these much later. mostly boring effects
            if attributes.spr_item == 'none' -- Not meant to be picked up
                continue
            append fs, () ->
                P = require "maps.Places"
                return P.create_isolated {
                    label: item
                    template: P.SimpleRoom
                    items: {[item]: 10}
                    enemies: random_enemies()
                    spawn_players: true
                }

        for enemy, attributes in pairs enemies
            --if not enemy\match "Dragon"
            --    continue
            append fs, () ->
                P = require "maps.Places"
                return P.create_isolated {
                    label: enemy
                    template: P.SimpleRoom
                    items: random_items()
                    enemies: {[enemy]: 10}
                    spawn_players: true
                }
    if os.getenv "LANARTS_OVERWORLD_TEST"
        init = do_nothing
        append fs, () ->
            random_seed(1000) -- TEST_SEED + math.random() * 1000000 * 2)
            O = require("maps.01_Overworld")
            return O.overworld_create()
    return () ->
        init()
        init = do_nothing
        idx += 1
        if idx > #fs
            os.exit()
        return fs[idx]()

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
            if @_should_end() and os.getenv "LANARTS_OVERWORLD_TEST"
                @_goto_random_portal()
                @_n_inputs = 0 -- Reset menu state TODO refactor
                @_n_same_square = 0
            elseif @_should_end()
                GlobalData.__test_initialized = false
                @_n_inputs = 0 -- Reset menu state TODO refactor
                @_n_same_square = 0
                @input_source = false
                GameState.lazy_reset()
                return true
            should_continue = user_gamestate_step(...)
            RATE = 125
            if os.getenv "LANARTS_OVERWORLD_TEST"
                RATE = 10000
            if GameState.frame % RATE == 2
                --state = {}
                --for k,v in pairs(@)
                --    state[k] = v
                @input_source = false
                @_lpx = 0
                @_lpy = 0
                @_ai_state = false
                @_used_portals = {}
                @_queued = {}
                GameState.save("saves/test-save.save")
                GameState.load("saves/test-save.save")
                -- Copy over state:W
                GlobalData = require("core.GlobalData")
                GlobalData.__test_initialized = false
                --for k,v in pairs state
                --    @[k] = v
            return should_continue
        Engine.io = () ->
            if rawget @, "input_source"
                @input_source\reset()
                @simulate_game_input()

        --GameState.input_handle = () ->
        --    if @should_simulate_input()
        --        --GameState._input_clear()
        --        return user_input_handle(true)
        --    return user_input_handle(true)
    _goto_random_portal: () =>
        if not @input_source
            return false
        player = @input_source.player
        portals = {}
        for obj in *Map.objects_list(player.map)
            if GameObject.get_type(obj) == "feature"
                append portals, obj
        portal = @_rng\random_choice(portals)
        player.xy = portal.xy
        append @_queued, {1,1}
        @_use_portal = true
        --portal\on_player_interact(player)
    _should_end: () =>
        if not @input_source
            return false
        player = @input_source.player
        if @_n_same_square < 100
            return false
        for obj in *Map.objects_list(player.map)
            --if GameObject.get_type(obj) == "item"
            --    return false
            if GameObject.get_type(obj) == "actor" and obj.team ~= player.team
                return false
        return true
    _try_move_action: (dx, dy) =>
        @input_source\set("move_direction", {dx, dy})
        return true

    _fighter_action: () =>
        USE_DASH = @_rng\randomf() < 0.01
        USE_KNOCK = @_rng\randomf() < 0.1
        USE_ITEM = @_rng\randomf() < 0.1
        SELL_ITEM = @_rng\randomf() < 0.1
        dir = nil
        if not @_use_portal
            if USE_DASH
                @input_source\set("use_spell_slot", 2)
            elseif USE_KNOCK
                @input_source\set("use_spell_slot", 1)
            elseif USE_ITEM
                @input_source\set("use_item_slot", @_rng\random(0, 40))
            elseif SELL_ITEM and #@input_source.player\inventory() > 30
                @input_source\set("sell_item_slot", @_rng\random(0, 40))
        if not @_use_portal and @_attack_action() and not USE_DASH and not USE_KNOCK and not USE_ITEM
            dir = {0,0}
        if #@_queued > 0
            dir = @_queued[1]
            table.remove(@_queued, 1)
        if not dir
            dir = @_try_explore()
        if not dir
            dir = @_ai_state\get_next_direction()
            if dir then for i=1,4 do append @_queued, dir
        if not dir
            dir = @_ai_state\get_next_wander_direction()
            if dir then for i=1,4 do append @_queued, dir
        if dir
            @input_source\set("move_direction", dir)
        @_use_portal = false
        return true
    _attack_action: () =>
        -- Necromancer:
        player = @input_source.player
        enemies = Map.enemies_list(@input_source.player)
        for enemy in *enemies
            -- Cannot rest if an enemy is visible:
            if Map.object_visible(enemy, enemy.xy, @input_source.player)
                -- Necromancer:
                if player.class_name == 'Necromancer'
                    if player.stats.hp < player.stats.max_hp * 0.3 and not player\has_effect("Baleful Regeneration") 
                        @input_source\set("use_spell_slot", 1)
                    else
                        @input_source\set("use_spell_slot", 0)
                        --@simulate 'y'
                        --@simulate 'i'
                elseif player.class_name == 'Fighter'
                    @input_source\set("should_use_weapon", true)
                else
                    @input_source\set("use_spell_slot", @_rng\random(0, 4))

                return true
        return false

    _try_rest_if_needed: () =>
        player = @input_source.player
        if not player\can_benefit_from_rest()
            return false
        enemies = Map.enemies_list(player)
        for enemy in *enemies
            -- Cannot rest if an enemy is visible:
            if Map.object_visible(enemy, enemy.xy, player)
                return false
        -- Simply don't do any actions and return that we have resolved this step:
        return true
    _try_explore: () =>
        player = @input_source.player
        return player\direction_towards_unexplored()
    _n_same_square: 0
    simulate_game_input: () =>
        if not GlobalData.__test_initialized
            GlobalData.__test_initialized = true
            random_seed(HARDCODED_AI_SEED)
            @_lpx = 0
            @_lpy = 0
            @_ai_state = ExploreUtils.ai_state(@input_source.player)
            @_rng = require("mtwist").create(HARDCODED_AI_SEED)
            @_used_portals = {}
            @_queued = {}
            @_n_same_square = 0
            @_use_portal = false
            @input_source.player\gain_xp(100000)
            @input_source.player.stats.max_hp = 100000
            @input_source.player.stats.hp = 100000
        @_ai_state\step()
        player = @input_source.player
        --if #Map.enemies_list(player) == 0
        --    player\direct_damage(player.stats.hp + 1)
        dir = nil
        @_fighter_action()
        if @_lpx == player.x and @_lpy == player.y
            @_n_same_square += 1
        else
            @_n_same_square = 0
        @_lpx, @_lpy = player.x, player.y
}

return M
