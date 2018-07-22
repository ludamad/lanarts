ProgrammableInputSource = require "input.ProgrammableInputSource"
GameState = require "core.GameState"
GlobalData = require "core.GlobalData"
World = require "core.World"
GameObject = require "core.GameObject"
PathFinding = require "core.PathFinding"
Map = require "core.Map"
ExploreUtils = require "tests.ExploreUtils"
World = require "core.World"
ObjectUtils = require "objects.ObjectUtils"

HARDCODED_AI_SEED = 12345678

-- BotInputSource method
initialize = () =>
    GlobalData = require "core.GlobalData"
    if GlobalData.ai_state
        load_from_global(@)
        return
    random_seed(HARDCODED_AI_SEED)
    @_lpx = 0
    @_lpy = 0
    @_ai_state = ExploreUtils.ai_state(@player)
    @_rng = require("mtwist").create(HARDCODED_AI_SEED)
    @_used_portals = {}
    @_queued = {}
    @_n_same_square = 0
    @_use_portal = false
    @player\gain_xp(100000)
    @player.stats.max_hp = 100000
    @player.stats.hp = 100000

    -- BotInputSource method
load_from_global = () =>
    GlobalData = require "core.GlobalData"
    for kev, value in pairs(GlobalData.ai_state or {})
        @[key] = value

-- BotInputSource method
save_to_global = () =>
    GlobalData = require "core.GlobalData"
    ai_state = {}
    for kev, value in pairs @
        ai_state[key] = value
    GlobalData.ai_state = ai_state

-- Return BotInputSource, using the above pseudo-methods
return newtype {
    parent: ProgrammableInputSource
    init: (player) =>
        ProgrammableInputSource.init(@, player)
        initialize(@)
    -- step: () =>
    --     @_simulate_game_input()
    --     @set_inputs {
    --         move_direction: {1, 0}
    --     }

    -- BotInputSource method
    step: () =>
        @_ai_state\step()
        player = @player
        --if #Map.enemies_list(player) == 0
        --    player\direct_damage(player.stats.hp + 1)
        dir = nil
        @_fighter_action()
        if @_lpx == player.x and @_lpy == player.y
            @_n_same_square += 1
        else
            @_n_same_square = 0
        @_lpx, @_lpy = player.x, player.y

    _goto_random_portal: () =>
        portals = {}
        for obj in *Map.objects_list(@player.map)
            if GameObject.get_type(obj) == "feature"
                append portals, obj
        portal = @_rng\random_choice(portals)
        @player.xy = portal.xy
        append @_queued, {1,1}
        @_use_portal = true
        --portal\on_player_interact(player)
    _should_end: () =>
        if not @input_source
            return false
        if os.getenv("LANARTS_OVERWORLD_TEST")and @_n_inputs > 10000
            return true
        player = @player
        if @_n_same_square < 100
            return false
        for obj in *Map.objects_list(player.map)
            --if GameObject.get_type(obj) == "item"
            --    return false
            if GameObject.get_type(obj) == "actor" and obj.team ~= player.team
                return false
        return true
    _try_move_action: (dx, dy) =>
        @set("move_direction", {dx, dy})
        return true

    _fighter_action: () =>
        USE_DASH = @_rng\randomf() < 0.01
        USE_KNOCK = @_rng\randomf() < 0.1
        USE_ITEM = @_rng\randomf() < 0.1
        SELL_ITEM = @_rng\randomf() < 0.1
        RANDOM_MOVE = @_rng\randomf() < 0.01
        dir = nil
        if not RANDOM_MOVE and not @_use_portal
            if USE_DASH
                @set("use_spell_slot", 2)
            elseif USE_KNOCK
                @set("use_spell_slot", 1)
            elseif USE_ITEM
                @set("use_item_slot", @_rng\random(0, 40))
            elseif SELL_ITEM and #@player\inventory() > 30
                @set("sell_item_slot", @_rng\random(0, 40))
        if not @_use_portal and @_attack_action() and not USE_DASH and not USE_KNOCK and not USE_ITEM
            dir = {0,0}
        if #@_queued > 0
            dir = @_queued[1]
            table.remove(@_queued, 1)
        if not dir and RANDOM_MOVE
            dir = {0,0}
            while dir[1] == 0 and dir[2] == 0
                dx = @_rng\random_choice {-1, 0, 1}
                dy = @_rng\random_choice {-1, 0, 1}
                dir = {dx, dy}
            if dir then for i=1,4 do append @_queued, dir
        if not dir
            dir = @_try_explore()
        if not dir
            dir = @_ai_state\get_next_direction()
            if dir then for i=1,4 do append @_queued, dir
        if not dir
            dir = @_ai_state\get_next_wander_direction()
            if dir then for i=1,4 do append @_queued, dir
        if dir
            @set("move_direction", dir)
        @_use_portal = false
        return true
    _attack_action: () =>
        -- Necromancer:
        player = @player
        enemies = Map.enemies_list(@player)
        for enemy in *enemies
            -- Cannot rest if an enemy is visible:
            if Map.object_visible(enemy, enemy.xy, @player)
                -- Necromancer:
                if player.class_name == 'Necromancer'
                    if player.stats.hp < player.stats.max_hp * 0.3 and not player\has_effect("Baleful Regeneration")
                        @set("use_spell_slot", 1)
                    else
                        @set("use_spell_slot", 0)
                elseif player.class_name == 'Fighter'
                    @set("should_use_weapon", true)
                else
                    @set("use_spell_slot", @_rng\random(0, 4))

                return true
        return false

    _try_rest_if_needed: () =>
        player = @player
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
        player = @player
        return player\direction_towards_unexplored()
    _n_same_square: 0
}
