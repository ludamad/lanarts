EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
Map = require "core.Map"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"

Data.spell_create {
    name: "Ice Form"
    spr_spell: "spr_spells.iceform"
    description: "A very powerful ability for safe exploration. Initiates Ice Form, preventing attacks and spells, lowering speed drastically, but providing near immunity for 10 seconds." 
    mp_cost: 10
    cooldown: 100
    can_cast_with_held_key: false
    fallback_to_melee: false
    action_func: (caster, x, y) ->
        caster\add_effect("Ice Form", 300)
        if caster\is_local_player()
            EventLog.add("You enter a glacial state!", {200,200,255})
        elseif caster.name == "Your ally"
            EventLog.add(caster.name .. " enters a glacial state!", {200,200,255})
    autotarget_func: (caster) -> caster.x, caster.y
    prereq_func: (caster) -> 
        return not caster\has_effect("Berserk") and not caster\has_effect("Exhausted")  and not caster\has_effect("Ice Form")
}

Data.effect_create {
    name: "Ice Form"
    effected_sprite: "spr_spells.iceform"
    can_use_rest: false
    can_use_spells: false
    can_use_stairs: true -- Important!!
    can_use_weapons: false
    effected_colour: {200, 200, 255}
    stat_func: (effect, obj, old, new) ->
        new.defence += 20
        new.willpower += 20
        new.reduction += 20
        new.magic_reduction += 20
        new.mp, new.max_mp = 1,1
        new.speed /= 2
    fade_out: 55
}

-- Methods for implementing Aura's.
-- No extension mechanism, just called explicitly.
AuraBase = {
    init: () =>
        @n_steps = 0
        @total_time = @time_left
        @max_alpha = 0.15
        @n_ramp = 25
    step: () =>
        assert @range ~= nil, "'range' must be assigned to Aura effects."
    draw: (inner_col, outer_col, x, y) =>
        @n_steps += 1
        min = math.min(@n_ramp, math.max(@n_steps, math.abs(@n_steps - @time_left)))
        alpha = (min / @n_ramp) * 2
        if alpha > 1.0
            -- Wrap around the alpha:
            alpha = 1 - (alpha - 1) 
        alpha = math.max(0.2, math.min(alpha, 1))
        alpha *= @max_alpha
        xy = Display.to_screen_xy {x, y}
        Display.draw_circle(with_alpha(inner_col, alpha), xy, @range)
        Display.draw_circle_outline(with_alpha(outer_col, alpha), xy, @range, 2)
}

-- TODO
-- Data.effect_create Aura {
--     name: "Health Aura"
--     category: "Aura"
--     init_func: (caster) =>
--         AuraBase.init(@, caster)
--         @hp_gain = 2
--     step_func: (caster) =>
--         AuraBase.step(@, caster)
--         for mon in *(Map.monsters_list() or {})
--             if mon\has_effect("Healed")
--                 continue
--             eff_range = mon.target_radius + caster.target_radius + @range
--             dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
--             if dist < eff_range
--                 mon\add_effect("Pained", 50)
--                 if mon\damage(random(4,15) * 2 + caster.stats.magic * 2, random(6,10) + caster.stats.magic * 0.2, 1.0, 2.0)
--                     {:stats} = caster
--                     stats.mp = math.min(stats.max_mp, stats.mp + @mp_gain)
--                     -- Summon zombies by probability!?
--                     if caster\is_local_player() 
--                         EventLog.add("You drain the enemy's life force as MP!", {200,200,255})
--                     else
--                         EventLog.add(caster.name .. " drains the enemy's life force as MP!", {200,200,255})
--     draw_func: (caster, top_left_x, top_left_y) =>
--         Aura.draw(@, COL_PALE_RED, COL_RED, caster.x, caster.y)
-- }

Data.effect_create {
    name: "Fear Aura"
    category: "Aura"
    effected_colour: {200, 200, 255}
    fade_out: 100
    effected_sprite: "spr_spells.cause_fear"
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @max_alpha = 0.9
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for mon in *(Map.monsters_list() or {})
            if mon\has_effect("Fear")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Fear", 300)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_GRAY, COL_BLACK, caster.x, caster.y)
}

Data.effect_create {
    name: "Healing"
    category: "Aura"
    effected_sprite: "spr_amulets.healing"
    fade_out: 25
    step_func: (obj) =>
        obj\heal_hp(20 / 60)
}

Data.effect_create {
    name: "Healing Aura"
    category: "Aura"
    effected_sprite: "spr_amulets.healing"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @max_alpha = 0.9
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for p in *(Map.players_list() or {})
            if p\has_effect("Healing")
                continue
            dist = vector_distance({p.x, p.y}, {caster.x, caster.y})
            if dist < @range
                p\add_effect("Healing", 200)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_GRAY, COL_WHITE, caster.x, caster.y)
}

Data.effect_create {
    name: "Daze Aura"
    category: "Aura"
    effected_sprite: "spr_amulets.light"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for mon in *(Map.monsters_list() or {})
            if mon\has_effect("Dazed")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Dazed", 200)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_PALE_YELLOW, COL_YELLOW, caster.x, caster.y)
}

Data.effect_create {
    name: "Pain Aura"
    category: "Aura"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @mp_gain = 10
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for mon in *(Map.monsters_list() or {})
            if mon\has_effect("Pained")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Pained", 50)
                if mon\damage(random(4,15) * 2 + caster.stats.magic * 2, random(6,10) + caster.stats.magic * 0.2, 1.0, 2.0)
                    {:stats} = caster
                    stats.mp = math.min(stats.max_mp, stats.mp + @mp_gain)
                    -- Summon zombies by probability!?
                    if caster\is_local_player() 
                        EventLog.add("You drain the enemy's life force as MP!", {200,200,255})
                    else
                        EventLog.add(caster.name .. " drains the enemy's life force as MP!", {200,200,255})
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_PALE_RED, COL_RED, caster.x, caster.y)
}

-- TODO separate into ItemEffects.moon

Data.effect_create {
    name: "VampiricWeapon"
    on_melee_func: (attacker, defender, damage, will_die, attack_stats) =>
        if attacker\is_local_player() 
            EventLog.add("You steal the enemy's life!", {200,200,255})
        attacker\heal_hp(damage / 10)
}


Data.effect_create {
    name: "PoisonedWeapon"
    on_melee_func: (attacker, defender, damage, will_die, attack_stats) =>
        if defender\has_effect("Poison")
            return
        if chance(.25)
            eff = defender\add_effect("Poison", 100)
            eff.poison_rate = 25
            eff.damage = attack_stats.damage
            eff.power = attack_stats.power
            eff.magic_percentage = attack_stats.magic_percentage
}

Data.effect_create {
    name: "FearWeapon"
    on_melee_func: (attacker, defender, damage, will_die, attack_stats) =>
        if defender\has_effect("Fear")
            return
        if chance(.1)
            eff = defender\add_effect("Fear", 100)
}

Data.effect_create {
    name: "ConfusingWeapon"
    on_melee_func: (attacker, defender, damage, will_die, attack_stats) =>
        if defender\has_effect("Dazed")
            return
        if chance(.1)
            eff = defender\add_effect("Dazed", 100)
}
