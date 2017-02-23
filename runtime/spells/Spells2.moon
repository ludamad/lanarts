EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"

mon_title = (mon) -> if mon.unique then mon.name else "the #{mon.name}"

Data.spell_create {
    name: "Ludaze"
    spr_spell: "spr_spells.ludaze"
    description: "Dazes and poisons all enemies in sight." 
    mp_cost: 25
    cooldown: 0
    spell_cooldown: 800
    can_cast_with_held_key: true
    fallback_to_melee: false
    action_func: (caster, x, y) ->
        caster\add_effect("Ludaze", 30)
        play_sound "sound/ludaze.ogg"
        for mon in *(Map.enemies_list caster)
            if not Map.object_visible(mon)
                continue
            mon\add_effect("Dazed", 100)
            eff = mon\add_effect("Poison", 100)
            eff.poison_rate = 25
            eff.damage = caster.stats.magic / 2
            eff.power = caster.stats.magic * 0.3
            eff.magic_percentage = 1.0
        if caster\is_local_player()
            EventLog.add("You daze all enemies in sight!", {200,200,255})
        elseif caster.name == "Your ally"
            EventLog.add(caster.name .. " dazes all enemies in sight!", {200,200,255})
    autotarget_func: (caster) -> caster.x, caster.y
    prereq_func: (caster) -> 
        if not caster\has_effect("Berserk") and not caster\has_effect("Exhausted")  and not caster\has_effect("Ice Form")
            for mon in *(Map.enemies_list caster)
                if Map.object_visible(mon)
                    return true
            EventLog.add("No monsters in sight!", COL_PALE_RED)
            return false
        return false
}


Data.spell_create {
    name: "Ice Form"
    spr_spell: "spr_spells.iceform"
    description: "Initiates Ice Form, a powerful ability for safe dungeoneering, preventing attacks and spells, and lowering speed drastically, but providing near immunity for 10 seconds." 
    mp_cost: 40
    cooldown: 100
    spell_cooldown: 1600
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
        if not caster.can_rest
            EventLog.add("Ice Form requires perfect concentration!", {200,200,255})
            return false
        return not caster\has_effect("Berserk") and not caster\has_effect("Exhausted")  and not caster\has_effect("Ice Form")
}

-- TODO flip effect of fortification -- TODO whut?
Data.effect_create {
    name: "Fortification"
    init_func: (caster) =>
        @active_bonuses = {}
        @duration = 320
    stat_func: (obj, old, new) =>
        for k, v in pairs @active_bonuses -- Abuse that stat_func is called every frame
            if v == 0 
                EventLog.add("Your defence falls back down...", COL_PALE_BLUE)
                @active_bonuses[k] = nil
            else 
                new.defence += 2
                @active_bonuses[k] -= 1
    on_receive_melee_func: (attacker, defender, damage, attack_stats) =>
        if not @active_bonuses[attacker.id]
            EventLog.add("Your defence rises due to getting hit!", COL_PALE_BLUE)
            @active_bonuses[attacker.id] = @duration
        elseif @active_bonuses[attacker.id] < @duration
            @active_bonuses[attacker.id] = @duration
        return damage

}

Data.effect_create {
    name: "Ludaze"
    effected_sprite: "spr_spells.ludaze"
    can_use_rest: false
    effected_colour: {200, 200, 255, 200}
    fade_out: 10
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
        new.reduction += 20
        new.magic_reduction += 20
        new.resistance += 20
        new.magic_resistance += 20
        new.speed = math.min(new.speed, 0.5)
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
        min = math.min(@n_ramp, if @n_steps > @total_time / 2 then math.abs(@n_steps - @total_time) else @n_steps)
        alpha = (min / @n_ramp)
        alpha = math.max(0.2, math.min(alpha, 1))
        alpha *= @max_alpha
        xy = Display.to_screen_xy {x, y}
        Display.draw_circle(with_alpha(inner_col, alpha), xy, @range)
        Display.draw_circle_outline(with_alpha(outer_col, alpha), xy, @range, 2)
}

-- Effects:

-------  START EQUIPMENT SLOT STACKED EFFECTS --------
-- Hack because effects are not currently easily stacked otherwise:
for equip_slot in *{"", "Amulet", "Ring", "Belt", "Weapon", "Legwear"}
    Data.effect_create {
        name: "#{equip_slot}HealthGainOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                if caster\is_local_player()
                    EventLog.add("You regain health for killing!", COL_PALE_BLUE)
                caster\heal_hp(3 + caster.stats.level)
                @kill_tracker += 1
    }
    Data.effect_create {
        name: "#{equip_slot}ManaGainOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                if caster\is_local_player()
                    EventLog.add("You regain mana for killing!", COL_PALE_BLUE)
                caster\heal_mp(3 + caster.stats.level)
                @kill_tracker += 1
    }

    Data.effect_create {
        name: "PossiblySummonCentaurOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                if chance(0.05)
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                    play_sound "sound/summon.ogg"
                    monster = "Centaur Hunter"
                    if not (caster\has_effect "Summoning")
                        eff = caster\add_effect("Summoning", 20)
                        eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                        eff.duration = 5
                @kill_tracker += 1
    }

    Data.effect_create {
        name: "PossiblySummonStormElementalOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                if chance(0.05)
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                    play_sound "sound/summon.ogg"
                    monster = "Storm Elemental"
                    if not (caster\has_effect "Summoning")
                        eff = caster\add_effect("Summoning", 20)
                        eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                        eff.duration = 5
                @kill_tracker += 1
    }


    Data.effect_create {
        name: "PossiblySummonGolemOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                if chance(0.05)
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                    play_sound "sound/summon.ogg"
                    monster = "Golem"
                    if not (caster\has_effect "Summoning")
                        eff = caster\add_effect("Summoning", 20)
                        eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                        eff.duration = 5
                @kill_tracker += 1
    }
-------  STOP EQUIPMENT SLOT STACKED EFFECTS --------

Data.effect_create {
    name: "Summoning"
    category: "Summon"
    effected_colour: COL_PALE_RED
    fade_out: 10
    --stat_func: (effect, obj, old, new) ->
    --    new.speed *= 2
    init_func: (caster) =>
        @n_steps = 0
        @delay = 1
        @duration = @time_left
        @on_summon = do_nothing
        caster.summoned or= {}
    step_func: (caster) =>
        @n_steps += 1
        if @n_steps == @delay
            ability = SpellObjects.SummonAbility.create {monster: @monster, :caster, xy: {caster.x, caster.y}, duration: @duration, on_summon: @on_summon}
            GameObject.add_to_level(ability)
}

Data.effect_create {
    name: "Reviving"
    init_func: (player) =>
        -- No op for now
    finish_func: (player) =>
        player.is_ghost = false
        player.stats.hp = 1
    draw_func: (player, top_left_x, top_left_y) =>
        xy = Display.to_screen_xy player.xy
        Fonts.small\draw({origin: Display.CENTER}, xy, "#{math.ceil @time_left / 60}")
}

Data.effect_create {
    name: "Fear Aura"
    category: "Aura"
    effected_colour: {200, 200, 255}
    fade_out: 100
    effected_sprite: "spr_spells.cause_fear"
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @max_alpha = 0.4
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for mon in *(Map.enemies_list caster)
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
        {:hpregen} = obj\effective_stats()
        obj\heal_hp(hpregen * 20)
}

Data.effect_create {
    name: "Healing Aura"
    category: "Aura"
    effected_sprite: "spr_amulets.healing"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @max_alpha = 0.4
    step_func: (caster) =>
        AuraBase.step(@, caster)
        for ally in *Map.allies_list(caster)
            if ally\has_effect("Healing")
                continue
            dist = vector_distance({ally.x, ally.y}, {caster.x, caster.y})
            if dist < @range
                ally\add_effect("Healing", 200)
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
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Dazed")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Dazed", 100)
    draw_func: (caster, top_left_x, top_left_y) =>
        AuraBase.draw(@, COL_PALE_YELLOW, COL_YELLOW, caster.x, caster.y)
}

Data.effect_create {
    name: "Sap Aura"
    category: "Aura"
    fade_out: 100
    init_func: (caster) =>
        AuraBase.init(@, caster)
        if caster.is_enemy
            @range = 90
    step_func: (caster) =>
        AuraBase.step(@, caster)
        if caster.is_enemy and not Map.object_visible(caster)
            return
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Sapped")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Sapped", 35)
                mon.stats.mp = math.max(0, mon.stats.mp - 10)
                if mon\is_local_player() 
                    EventLog.add("Your MP is drained!", {200,200,255})
                elseif not mon.is_enemy
                    EventLog.add(mon.name .. "'s MP is drained!", {200,200,255})
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        AuraBase.draw(@, COL_PALE_BLUE, COL_BLUE, caster.x, caster.y)
}


Data.effect_create {
    name: "Pain Aura"
    category: "Aura"
    effected_sprite: "spr_spells.greaterpain"
    fade_out: 50
    init_func: (caster) =>
        AuraBase.init(@, caster)
        @mp_gain = 10
        if caster.is_enemy
            @range = 90
    step_func: (caster) =>
        AuraBase.step(@, caster)
        if @animation_only
            return
        if caster.is_enemy and not Map.object_visible(caster)
            return
        for mon in *(Map.enemies_list caster)
            if mon\has_effect("Pained")
                continue
            dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < @range
                mon\add_effect("Pained", 50)
                play_pained_sound()
                caster\add_effect("Pained", 50)
                if mon\damage(random(4,15) * 2 + caster.stats.magic * 2, random(2,5) + caster.stats.magic * 0.2, 1.0, 2.0)
                    {:stats} = caster
                    caster\gain_xp_from(mon)
                    {:max_hp} = mon\effective_stats()
                    if caster\has_effect("AmuletGreatPain")
                        caster\heal_hp(max_hp * 2/ 16)
                    else
                        caster\heal_hp(max_hp / 16)
                    --stats.mp = math.min(stats.max_mp, stats.mp + @mp_gain)
                    ---- Summon zombies by probability!?
                    --if caster\is_local_player() 
                    --    EventLog.add("You drain the enemy's life force as MP!", {200,200,255})
                    --elseif not caster.is_enemy
                    --    EventLog.add(caster.name .. " drains the enemy's life force as MP!", {200,200,255})
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        if @animation_only 
            @max_alpha /= 4
        elseif not caster.is_enemy and not caster\has_effect "Pained"
            @max_alpha /= 2
        AuraBase.draw(@, COL_PALE_RED, COL_RED, caster.x, caster.y)
}

-- TODO separate into ItemEffects.moon

Data.effect_create {
    name: "VampiricWeapon"
    on_melee_func: (attacker, defender, damage, attack_stats) =>
        if attacker\is_local_player() 
            EventLog.add("You steal the enemy's life!", {200,200,255})
        attacker\heal_hp(damage / 4)
        return damage
}


Data.effect_create {
    name: "Abolishment"
    stat_func: (effect, obj, old, new) ->
        new.strength += math.ceil(new.magic / 3)
        new.magic = 0
}

Data.effect_create {
    name: "EnemyHyperProjectile"
    stat_func: (effect, obj, old, new) ->
        new.ranged_cooldown_multiplier *= 0.1
}

Data.effect_create {
    name: "KnockbackWeapon"
    on_melee_func: (attacker, defender, damage, attack_stats) =>
        if defender\has_effect("Thrown")
            return
        if chance(.1)
            thrown = defender\add_effect("Thrown", 20)
            thrown.angle = vector_direction({attacker.x, attacker.y}, {defender.x, defender.y})
        return damage
}

Data.effect_create {
    name: "PoisonedWeapon"
    on_melee_func: (attacker, defender, damage, attack_stats) =>
        if defender\has_effect("Poison")
            return
        if (if attacker.is_enemy then chance(.25) else chance(.1))
            eff = defender\add_effect("Poison", 100)
            eff.poison_rate = 25
            eff.damage = attack_stats.damage
            eff.power = attack_stats.power
            eff.magic_percentage = attack_stats.magic_percentage
        return damage
}

Data.effect_create {
    name: "FearWeapon"
    on_melee_func: (attacker, defender, damage, attack_stats) =>
        if defender\has_effect("Fear")
            return
        if chance(.1)
            eff = defender\add_effect("Fear", 150)
        return damage
}

Data.effect_create {
    name: "ConfusingWeapon"
    on_melee_func: (attacker, defender, damage, attack_stats) =>
        if defender\has_effect("Dazed")
            return
        if chance(.1)
            eff = defender\add_effect("Dazed", 70)
        return damage
}

Data.effect_create {
    name: "StopOnceInRange"
    -- For centaurs predominantly, coded in C++
}

Data.effect_create {
    name: "Encumbered"
    stat_func: (obj, old, new) =>
        new.speed -= 1
}

Data.effect_create {
    name: "Encumbered"
    stat_func: (obj, old, new) =>
        new.speed -= 1
}

Data.spell_create {
    name: "Baleful Regeneration",
    spr_spell: "spr_spells.regeneration",
    description: "You tap into necromancy to very quickly bind your wounds, until your are fully healed or its power runs out. Takes longer to run out the more willpower you have.",
    -- TODO have an interface for displaying custom stats, -- description_draw_func ?? Maybe directly call the draw API, but have positioning handled.
    -- TODO then have an easy mechanism for telling if the text is hovered over and letting the formula be displayed if so.
    mp_cost: 0,
    cooldown: 35,
    can_cast_with_held_key: false,
    fallback_to_melee: false,
    spell_cooldown: 800
    prereq_func: (caster) ->
        if caster.stats.hp == caster\effective_stats().max_hp
            return false
        return not caster\has_effect("Baleful Regeneration") and not caster\has_effect("Exhausted")
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
        {:willpower} = caster\effective_stats()
        -- Level 1 willpower is expected to be 7, and any additional willpower grants half a second of cooldown time.
        -- At level 7, this is an extra 3 seconds of cooldown, plus items, leading to ~double expected time.
        caster\add_effect("Baleful Regeneration", 60 * 6 + (willpower - 7) * 30)
        if caster\is_local_player()
            EventLog.add("You start to regenerate quickly!", {200,200,255})
            --play_sound "sound/Jingle_Win_Synth/Jingle_Win_Synth_00.ogg"
            play_sound "sound/berserk.ogg"
        else
            EventLog.add(caster.name .. " starts to regenerate quickly!", {200,200,255})
}

Data.effect_create {
    name: "Baleful Regeneration",
    effected_sprite: "spr_amulets.i-regeneration"
    effected_colour: {200,255,200}
    step_func: (caster) =>
        {:hp, :max_hp, :hpregen} = caster\effective_stats()
        if hp >= max_hp
            caster\remove_effect("Baleful Regeneration")
        else
            regen_rate = hpregen * (if caster\has_effect("AmuletGreatPain") then 29 else 14)
            caster\heal_hp(regen_rate)
}

Data.effect_create {
    name: "Spiky"
    on_receive_melee_func: (attacker, defender, damage, attack_stats) =>
        if attacker\direct_damage(damage * 0.25)
            defender\gain_xp_from(attacker)
        if defender.is_local_player and defender\is_local_player()
            EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
        return damage
}

-- These all are implemented by checks in the code:
Data.effect_create {
    name: "AmuletBerserker"
}

Data.effect_create {
    name: "AmuletGreatPain"
}

Data.effect_create {
    name: "AmuletGreaterFire"
}

Data.effect_create {
    name: "DiesOutsideOfSummonerRange"
    init_func: (summon) =>
        @summoner = false -- TODO think about allowing initial state to be passed to effects!
        -- TODO but OTOH I like the current system. 
    step_func: (summon) =>
        assert @summoner, "No summoner set on DiesOutsideOfSummonerRange!"
        if not Map.object_visible summon, summon.xy, @summoner
            summon\direct_damage(10)
}

Data.effect_create {
    name: "Summoner"
    init_func: (caster) =>
        @n_steps = 0
        @n_summons = 0
    step_func: (caster) =>
        caster.summoned or= @summoned or {}
        @summoned or= caster.summoned
        @n_summons = 0
        for mon, time in pairs caster.summoned
            if not mon.destroyed
                time_out = (if mon.name == "Spectral Beast" then (time > 600) else (time > 4800)) -- All others are permanents
                diff_floor = (caster.map ~= mon.map)
                if time_out or diff_floor
                    mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                caster.summoned[mon] = nil
            else
                caster.summoned[mon] += 1
                @n_summons += 1
}

EFFECT_FOREVER = 2^30
-- Requires kills from the necromancer, in the form of mana.
Data.spell_create {
    name: "Summon Dark Aspect",
    spr_spell: "spr_spells.summon",
    description: "You summon a dark companion, at the cost of health and mana. The companion is stronger depending on the caster's willpower.",
    --description: "You summon a dark companion, at the cost of health and mana. The companion is stronger depending on the caster's willpower. Dies quickly outside of summoner view.",
    mp_cost: 10,
    cooldown: 55,
    can_cast_with_held_key: false,
    fallback_to_melee: false,
    spell_cooldown: 200
    prereq_func: (caster) ->
        if caster.stats.hp < 55
            if caster\is_local_player() 
                EventLog.add("You do not have enough health!", {200,200,255})
            return false
        if not caster\has_effect "Necromancer"
            if caster\is_local_player() 
                EventLog.add("You must be a necromancer to cast this spell!", {200,200,255})
            return false
        amount = math.max 1, math.ceil((caster\effective_stats().willpower - 7) / 2)
        {:n_summons} = caster\get_effect("Summoner")
        if n_summons >= amount
            if caster\is_local_player() 
                EventLog.add("You cannot currently control more than #{amount} aspects!", {200,200,255})
            return false
        return not caster\has_effect("Exhausted") and not (caster\has_effect "Summoning")
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
        play_sound "sound/summon.ogg"
        monster = "Spectral Beast"
        if not (caster\has_effect "Summoning")
            caster\direct_damage(35)
            eff = caster\add_effect("Summoning", 20)
            eff.on_summon = (obj) ->
                -- -- Make sure this monster cannot live outside its summoner's range for very long:
                --eff = obj\add_effect("DiesOutsideOfSummonerRange", EFFECT_FOREVER)
                -- eff.summoner = caster
                -- Buff this monster based on caster's willpower:
                obj.stats.hp += math.floor(caster\effective_stats().willpower * 5)
                obj.stats.max_hp += math.floor(caster\effective_stats().willpower * 5)
                obj.stats.strength += math.floor(caster\effective_stats().willpower / 2)
                obj.stats.magic += math.floor(caster\effective_stats().willpower / 2)
                obj.stats.defence += math.floor(caster\effective_stats().willpower / 2)
                obj.stats.willpower += math.floor(caster\effective_stats().willpower / 2)
            eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
            eff.duration = 5
}


for name in *{"Ranger", "Fighter", "Necromancer", "Mage"}
    Data.effect_create {
        :name
        stat_func: (obj, old, new) =>
            if name == "Mage" and obj.stats.level == 1
                new.spell_cooldown_multiplier *= 1.1
            if name == "Mage" and obj.stats.level == 2
                new.spell_cooldown_multiplier *= 1.05
            if name == "Necromancer" or name == "Mage" or name == "Ranger"
                new.melee_cooldown_multiplier *= 1.5

        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            -- Keep state for doing summons in a stateful effect:
            if caster\has_effect "Summoner"
                caster\add_effect "Summoner", 2 -- Keep effect from dying
            else
                eff = caster\add_effect "Summoner", 2
                eff.duration = 30
            while caster.kills > @kill_tracker
                if name == "Necromancer"
                    if caster\is_local_player()
                        EventLog.add("You gain mana for killing!", COL_PALE_BLUE)
                    caster\heal_mp(5)
                for {:instance, :class_name} in *World.players
                    if instance ~= caster and class_name == "Necromancer"
                        if instance\is_local_player()
                            EventLog.add("You gain mana from the carnage!", COL_PALE_BLUE)
                        instance\heal_mp(2)
                @kill_tracker += 1
        on_receive_melee_func: (attacker, defender, damage, attack_stats) =>
            if name == "Necromancer"
                if attacker\direct_damage(damage * 0.33)
                    defender\gain_xp_from(attacker)
                if defender\is_local_player()
                    EventLog.add("Your corrosive flesh hurts #{mon_title attacker} as you are hit!", COL_PALE_BLUE)
            return damage
    }

-- Move in a direction, attacking everyone along the way
Data.effect_create {
    name: "Dash Attack"
    effected_sprite: "spr_effects.pain_mirror"
    can_use_rest: false
    effected_colour: {200, 200, 255, 255}
    fade_out: 10
    init_func: (caster) =>
        play_sound "sound/equip.ogg"
        @steps = 0
        @attacked = {} -- No one attacked at first
    step_func: (caster) =>
        @steps += 1
        -- Move forward: 
        xy = {
            caster.x + math.cos(@angle) * 16
            caster.y + math.sin(@angle) * 16
        }
        if Map.object_tile_check(caster, xy)
            play_sound "sound/door.ogg"
            caster\remove_effect "Dash Attack"
            return
        caster.xy = xy
        if caster\has_ranged_weapon()
            return
        -- Attack nearby monsters:
        for mon in *Map.enemies_list(caster)
            dist = vector_distance(mon.xy, caster.xy)
            if dist < caster.target_radius + mon.target_radius + caster.weapon_range
                if @attacked[mon.id]
                    continue
                @attacked[mon.id] = true
                if caster\is_local_player()
                    EventLog.add("You strike as you pass!", {200,200,255})
                caster\melee(mon)
    draw_func: (caster) =>
        for i=1,math.min(@steps, 12)
            xy = {
                caster.x - math.cos(@angle) * 16 * i
                caster.y - math.sin(@angle) * 16 * i
            }
            screen_xy = Display.to_screen_xy(xy)
            caster.sprite\draw({color: {255,255,255, 200 - 30 * i}, origin: Display.CENTER}, screen_xy)
    stat_func: (caster, old, new) =>
        new.speed = 0
        caster.stats.attack_cooldown = 2
        new.strength += 2
        new.defence += 5
        new.willpower += 5
}

-- Dash Attack
Data.spell_create {
    name: "Dash Attack",
    spr_spell: "expedite",
    description: "Dash in a straight line, hitting all enemies in your path. Stops if you hit a wall. Can still perform normal attacks and spells while dashing.",
    --description: "You summon a dark companion, at the cost of health and mana. The companion is stronger depending on the caster's willpower. Dies quickly outside of summoner view.",
    mp_cost: 0
    cooldown: 0
    can_cast_with_held_key: true
    fallback_to_melee: true
    spell_cooldown: 800
    action_func: (x, y) =>
        effect = @add_effect "Dash Attack", 15
        effect.angle = vector_direction(@xy, {x,y})
        if @is_local_player()
            EventLog.add("You dash valiantly forward!", {200,200,255})
    prereq_func: () =>
        return not @has_ranged_weapon()
    autotarget_func: () =>
        {dx, dy} = @last_moved_direction
        return @x + dx * 32, @y + dy * 32
}
