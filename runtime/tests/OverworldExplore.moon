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

PROGRESSION = () ->
    idx = 0
    fs = {}
    init = () ->
        for item, attributes in pairs items
            if attributes.is_randart
                continue -- TODO test all these much later. mostly boring effects
            if attributes.spr_item == 'none' -- Not meant to be picked up
                continue
            append fs, () ->
                random_seed(1000) -- TEST_SEED + math.random() * 1000000 * 2)
                P = require "maps.Places"
                return P.create_isolated {
                    label: item
                    template: P.SimpleRoom
                    items: {[item]: 1}
                    enemies: {Sheep: 10}
                    spawn_players: true
                }

        for enemy, attributes in pairs enemies
            --if not enemy\match "Dragon"
            --    continue
            append fs, () ->
                random_seed(1000) -- TEST_SEED + math.random() * 1000000 * 2)
                P = require "maps.Places"
                return P.create_isolated {
                    label: enemy
                    template: P.SimpleRoom
                    items: {["Haste Scroll"]: 1}
                    enemies: {[enemy]: 10}
                    spawn_players: true
                }
    --    () ->
    --        random_seed(1000) -- TEST_SEED + math.random() * 1000000 * 2)
    --        O = require("maps.01_Overworld")
    --        V = require("maps.Vaults")
    --        -- return O.overworld_create()
    --        return O.test_vault_create(V.simple_room)
    --}
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
    --overworld_create: () =>
    --    O = require("maps.01_Overworld")
    --    V = require("maps.Vaults")
    --    -- return O.overworld_create()
    --    return O.test_vault_create(V.simple_room)
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
            should_continue = user_gamestate_step(...)
            if GameState.frame % 125 == 2
                --state = {}
                --for k,v in pairs(@)
                --    state[k] = v
                @input_source = false
                @_lpx = 0
                @_lpy = 0
                @_ai_state = false
                @_last_object = false
                @_used_portals = {}
                @_queued = {}
                GameState.save("saves/test-save.save")
                GameState.load("saves/test-save.save")
                -- Copy over state:W
                GlobalData = require("core.GlobalData")
                GlobalData.__test_initialized = false
                --for k,v in pairs state
                --    @[k] = v
            if @_should_end()
                GlobalData.__test_initialized = false
                @_n_inputs = 0 -- Reset menu state TODO refactor
                @_n_same_square = 0
                @input_source = false
                GameState.lazy_reset()
                return true
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
                    @input_source\set("use_spell_slot", 1)
                    @input_source\set("should_use_weapon", true)
                return
    _try_collect_items: () =>
        player = @input_source.player
        -- If we did not hold still last frame and are on an item, try to pick it up:
        if (@_lpx ~= player.x or @_lpy ~= player.y)
            collisions = Map.rectangle_collision_check(player.map, {player.x - 8, player.y - 8, player.x+8, player.y+8}, player)
            for col in *collisions
                if col == @_last_object
                    @_last_object = false
                obj_type = GameObject.get_type(col) 
                if obj_type == "item"
                    @_attack_action() -- No reason not to auto-attack while collecting
                    return true -- Hold still
                elseif obj_type == "feature" and not @_used_portals[col.id]
                    @_used_portals[col.id] = 0
                    return true -- Hold still
        -- If we did not have an item to pick up, try to path towards an object:
        closest = {false, math.huge}
        {dx, dy} = player\direction_towards_object (_, obj) ->
            obj_type = GameObject.get_type(obj)
            local matches
            if @_last_object
                matches = (obj == @_last_object)
            else
                matches = (obj_type == "item" or obj_type == "feature" and not @_used_portals[obj.id])
            if matches
                -- Allow in filter:
                dist = vector_distance(obj.xy, player.xy)
                if dist < closest[2]
                    closest[1],closest[2] = obj, dist
                return true
            -- Disallow in filter:
            return false
        @_last_object = closest[1]
        if @_try_move_action(dx, dy)
            @_attack_action() -- Handle attack action wherever we resolved move action 
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
        {dx, dy} = player\direction_towards_unexplored()
        if (dx ~=0 or dy ~= 0) and @_try_move_action(dx, dy)
            @_attack_action() -- Handle attack action wherever we resolved move action 
            return true
        return false
    _n_same_square: 0
    simulate_game_input: () =>
        if not GlobalData.__test_initialized
            GlobalData.__test_initialized = true
            random_seed(HARDCODED_AI_SEED)
            @_lpx = 0
            @_lpy = 0
            @_ai_state = ExploreUtils.ai_state(@input_source.player)
            @_last_object = false
            @_rng = require("mtwist").create(HARDCODED_AI_SEED)
            @_used_portals = {}
            @_queued = {}
            @_n_same_square = 0
            @input_source.player\gain_xp(100000)
            @input_source.player.stats.max_hp = 100000
            @input_source.player.stats.hp = 100000
        @_ai_state\step()
        player = @input_source.player
        --if #Map.enemies_list(player) == 0
        --    player\direct_damage(player.stats.hp + 1)
        dir = nil
        if #@_queued > 0
            dir = @_queued[1]
            @_try_move_action(dir[1], dir[2])
            table.remove(@_queued, 1)
        if not dir and not @_try_explore() then
            dir = @_ai_state\get_next_direction()
            if dir
                @_try_move_action(dir[1], dir[2])
                @_attack_action()
            if not dir and not @_try_rest_if_needed()
                dir = @_ai_state\get_next_wander_direction()
                if dir
                    @_try_move_action(dir[1], dir[2])
                @_attack_action()
            for i=1,4
                append @_queued, dir
        if @_lpx == player.x and @_lpy == player.y
            @_n_same_square += 1
        else
            @_n_same_square = 0
        @input_source\set("use_item_slot", @_rng\random(0, 20))
        @_lpx, @_lpy = player.x, player.y
}

return M
