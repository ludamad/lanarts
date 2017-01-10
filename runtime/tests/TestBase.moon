GameState = require "core.GameState"
GlobalData = require "core.GlobalData"
Map = require "core.Map"
sim = (k) ->
    Keyboard = require "core.Keyboard"
    print "SIMULATING ", k
    GameState._simulate_key_press(Keyboard[k])
user_input_capture = GameState.input_capture
user_input_handle = GameState.input_handle
M = nilprotect {
    _n_inputs: 0
    _past_item_stage: false
    _created_items: false
    _used_portals: {}
    -- Forwarding to handle events from a variety of modules, acts as a pseudo-require:
    intercept: (str) => 
        print(str)
        itable = nilprotect {}
        for k,v in pairs @
            itable[k] = (...) -> v(@, ...)
        return itable
    -- START EVENTS --
    player_has_won: () => false
    trigger_event: (event) =>
        print '*** EVENT:', event
    overworld_create: () =>
        O = require("maps.01_Overworld")
        V = require("maps.Vaults")
        -- return O.overworld_create()
        return O.test_vault_create(V.simple_room)
    -- END EVENTS --
    -- Default is to always want to simulate:
    should_simulate_input: () => not @_past_item_stage
    game_start: do_nothing
    simulate_menu_input: () =>
        Keyboard = require "core.Keyboard"
        switch @_n_inputs
            when 1 
                sim 'n'
            when 2 -- Mage
                sim 'TAB'
            when 3  -- Fighter
                sim 'TAB'
            when 4 -- Ranger
                sim 'TAB'
            when 5 -- Necro
                sim 'TAB'
            when 6
                sim 'ENTER'
        @_n_inputs += 1
    game_start: () =>
        GameState.input_handle = () ->
            if @should_simulate_input()
                GameState._input_clear()
                @simulate_game_input()
                GameState._trigger_events()
                return user_input_handle(false)
            return user_input_handle(true)
    simulate_game_input: () =>
        if not GlobalData.__test_initialized
            GlobalData.__test_initialized = true
            @_past_item_stage = false
            @_n_inputs = 0
            @_created_items = false
            @_lpx = 0
            @_lpy = 0
            @_key_spam = 'i' --random_choice {'y', 'i', 'o'}
            @_used_portals = {}
        World = require "core.World"
        if not @_created_items
            --dofile "debug_scripts/level3.lua"
            World.local_player\gain_xp(10000)
            @_created_items = true
        GameObject = require "core.GameObject"
        press_keys = true
        for k,v in pairs @_used_portals
            if v > 1600
                @_used_portals[k] = nil
            else
                @_used_portals[k] += 1
        if @_created_items
            player = World.local_player
            if #Map.enemies_list(player) == 0
                player\direct_damage(player.stats.hp + 1)
            local dx, dy
            --if @_n_inputs % 8 ~= 1
            --    dx, dy = player.x - @_lpx, player.y - @_lpy
            --else
            if true
                seen_some = {false}
                {dx, dy} = player\direction_towards_object (_, obj) -> 
                    obj_type = GameObject.get_type(obj) 
                    if obj_type == "item" or obj_type == "feature" and not @_used_portals[obj.id]
                        seen_some[1] = true
                        return true
                    return false
                if not seen_some[1]
                    {dx, dy} = player\direction_towards_unexplored()
                    -- @_past_item_stage = true
            if (@_lpx ~= player.x or @_lpy ~= player.y)
                collisions = Map.rectangle_collision_check(player.map, {player.x - 8, player.y - 8, player.x+8, player.y+8}, player)
                for col in *collisions
                    obj_type = GameObject.get_type(col) 
                    if obj_type == "item" or obj_type == "feature" and not @_used_portals[col.id]
                        if obj_type == "feature"
                            @_used_portals[col.id] = 0
                        dx, dy = 0,0
                        press_keys = false
            if dx < 0
                sim 'a'
            elseif dx > 0
                sim 'd'
            if dy < 0
                sim 'w'
            elseif dy > 0
                sim 's'
            if (@_lpx == player.x and @_lpy == player.y) and (dx == 0 and dy == 0)
                sim random_choice({'a', 'd', 'w', 's'})
            if chance(.1)
                sim random_choice({'a', 'w'})
            @_lpx, @_lpy = player.x, player.y
        if press_keys
            sim @_key_spam
        @_n_inputs += 1
}

GameState.input_capture = () ->
    if M\should_simulate_input()
        GameState._input_clear()
        M\simulate_menu_input()
        GameState._trigger_events()
        return true
    else 
        return user_input_capture()
return M
