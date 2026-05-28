EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"
DataW = require "DataWrapped"
TypeEffectUtils = require "spells.TypeEffectUtils"
BonusesUtils = require "items.BonusesUtils"
ImpactFX = require "ui.ImpactFX"
ProceduralEffects = require "ui.ProceduralEffects"

damage_source_name = (attacker, defender) ->
    source = "Unattributed"
    if attacker
        if attacker == defender
            source = "Self"
        elseif attacker.name and attacker.name ~= ""
            source = attacker.name
    return source

DataW.effect_create {
    name: "BalanceDamageTracker"
    apply_func: (obj, args) =>
        @telemetry = args and args.telemetry
        @damage_by_source_key = (args and args.damage_by_source_key) or "damage_taken_by_source"
        @events_by_source_key = (args and args.events_by_source_key) or "damage_events_by_source"
        @damage_total_key = (args and args.damage_total_key) or "damage_taken_total"
        @event_total_key = (args and args.event_total_key) or "damage_event_total"
    on_receive_damage_func: (attacker, defender, damage) =>
        telemetry = @telemetry
        if not telemetry or not damage or damage <= 0
            return damage
        source = damage_source_name(attacker, defender)
        damage_by_source = telemetry[@damage_by_source_key] or {}
        events_by_source = telemetry[@events_by_source_key] or {}
        telemetry[@damage_by_source_key] = damage_by_source
        telemetry[@events_by_source_key] = events_by_source
        damage_by_source[source] = (damage_by_source[source] or 0) + damage
        events_by_source[source] = (events_by_source[source] or 0) + 1
        telemetry[@damage_total_key] = (telemetry[@damage_total_key] or 0) + damage
        telemetry[@event_total_key] = (telemetry[@event_total_key] or 0) + 1
        return damage
}

DataW.effect_create {
    name: "BalanceDamageDealtTracker"
    apply_func: (obj, args) =>
        @telemetry = args and args.telemetry
        @attacker_team = args and args.attacker_team
        @damage_by_target_key = (args and args.damage_by_target_key) or "damage_dealt_by_target"
        @events_by_target_key = (args and args.events_by_target_key) or "damage_dealt_events_by_target"
        @damage_by_source_key = (args and args.damage_by_source_key) or "damage_dealt_by_source"
        @events_by_source_key = (args and args.events_by_source_key) or "damage_dealt_events_by_source"
        @damage_total_key = (args and args.damage_total_key) or "damage_dealt_total"
        @event_total_key = (args and args.event_total_key) or "damage_dealt_event_total"
    on_receive_damage_func: (attacker, defender, damage) =>
        telemetry = @telemetry
        if not telemetry or not damage or damage <= 0
            return damage
        if @attacker_team ~= nil and (not attacker or attacker.team ~= @attacker_team)
            return damage

        source = damage_source_name(attacker, defender)
        target = if defender and defender.name and defender.name ~= "" then defender.name else "Unattributed"
        damage_by_target = telemetry[@damage_by_target_key] or {}
        events_by_target = telemetry[@events_by_target_key] or {}
        damage_by_source = telemetry[@damage_by_source_key] or {}
        events_by_source = telemetry[@events_by_source_key] or {}
        telemetry[@damage_by_target_key] = damage_by_target
        telemetry[@events_by_target_key] = events_by_target
        telemetry[@damage_by_source_key] = damage_by_source
        telemetry[@events_by_source_key] = events_by_source

        damage_by_target[target] = (damage_by_target[target] or 0) + damage
        events_by_target[target] = (events_by_target[target] or 0) + 1
        damage_by_source[source] = (damage_by_source[source] or 0) + damage
        events_by_source[source] = (events_by_source[source] or 0) + 1
        telemetry[@damage_total_key] = (telemetry[@damage_total_key] or 0) + damage
        telemetry[@event_total_key] = (telemetry[@event_total_key] or 0) + 1
        return damage
}

DataW.effect_create {
    name: "Poison"
    effected_sprite: "spr_effects.poison"
    poison_rate: 35
    effected_colour: {100, 255, 100}
    fade_out: 4
    can_use_rest: false
    stat_func: (obj, old, new) =>
        new.defence = math.max(0, new.defence - 5)
        new.willpower = math.max(0, new.willpower - 3)
        if not obj.is_enemy
            new.speed /= 2
    apply_func: (obj, args) =>
        @steps = 0
        @attacker = assert args.attacker
        @damage, @power = assert(args.damage), assert(args.power)
        @magic_percentage = args.magic_percentage or 0.0
        @poison_rate = assert args.poison_rate
    step_func: (obj) =>
        if @steps < @poison_rate
            @steps += 1
        else
            @steps = 0
            obj\damage(@damage / 2, @power, @magic_percentage, @attacker, (TypeEffectUtils.get_resistance obj, "Green"))
}

DataW.effect_create {
    name: "Berserk"
    exhausted_duration: 300
    can_use_rest: false
    can_use_spells: true
    can_use_stairs: true
    fade_out: 5
    effected_colour: {255,160,160}
    effected_sprite: "berserk_effected"
    on_melee_func: (attacker, defender, damage) =>
        -- Give some extra time for simply hitting enemies in melee
        -- Favours high melee speed??
        if attacker.stats.level >= 2
            EventLog.add("Your rage grows from combat...", {200,200,255})
            @time_left = math.min(@max_time * 1.5, @time_left + 5)
        
        if chance(.1)
            defender\add_effect("Stun", 20 + attacker.stats.level)
        return damage
    stat_func: (obj, old, new) =>
        new.strength += obj.stats.level
        -- Greater defences during berserk
        new.defence = math.max(0, new.defence + obj.stats.level)
        new.willpower = math.max(0, new.willpower + obj.stats.level)
        new.melee_cooldown_multiplier /= 1.25
        if obj\has_effect("AmuletBerserker")
            new.hpregen += 8 / 60 -- Per 60 frames
        if new.speed < 6
            new.speed = math.max(new.speed * 1.25, 6)
        obj\reset_rest_cooldown()
    apply_func: (obj, time_left) =>
        @kill_tracker = obj.kills
        @max_time = math.max(@max_time or 0, time_left)
        @extensions = 0
    remove_func: (obj) =>
        obj\add_effect("Exhausted", @exhausted_duration)
        for _ in screens()
            if obj\is_local_player()
                play_sound "sound/exhausted.ogg"
                EventLog.add("You are now exhausted...", {255,200,200})
    step_func: (obj) =>
        if @extensions >= 10
            for _ in screens()
                if obj\is_local_player()
                    EventLog.add("Your rage is tickled... but cannot grow further.", {200,200,255})
            return
        diff = math.max(obj.kills - @kill_tracker, 0)
        for i=1,diff
            @time_left = math.min(@max_time * 1.5, @time_left + 20)
            for _ in screens()
                if obj\is_local_player()
                    EventLog.add("Your rage grows from killing...", {200,200,255})
                    play_sound "sound/swish-11.wav"
                if settings.verbose_output
                    EventLog.add("Killed Enemy, berserk time_left = " .. @time_left)
            if obj\has_effect("AmuletBerserker") and chance(0.05) then
                play_sound "sound/summon.ogg"
                eff = obj\add_effect("Summoning", 20)
                eff.monster = "Centaur Hunter"
                eff.duration = 5
            --stats = obj\effective_stats()
            -- TODO moving towards HP regen++ instead of hp heal
            --obj\heal_hp(10 + stats.defence * 2)
            @extensions += 1
        @kill_tracker = obj.kills
}

DataW.effect_create {
    name: "Exhausted"
    can_use_rest: false
    effected_colour: {120,120,120}
    effected_sprite: "exhausted_effected"
    stat_func: (obj, old, new) =>
        new.speed = new.speed / 2
        new.defence = math.max(0, new.defence - 3)
        new.willpower = math.max(0, new.willpower - 3)
        new.cooldown_mult = new.cooldown_mult * 1.25
        obj\reset_rest_cooldown()
    remove_func: (obj) =>
        for _ in screens()
            if obj\is_local_player()
                EventLog.add("You are no longer exhausted.", {200,200,255})
}
DataW.effect_create {
    name: "Expedited"
    stat_func: () =>
        if new.speed < 6
            new.speed = math.max(new.speed * 1.5, 6)
        new.ranged_cooldown_multiplier /= 1.5
    can_use_rest: false
    finish_func: (obj) =>
        for _ in screens()
            if obj\is_local_player()
                EventLog.add("You are no longer expedited.", {255,200,200})
    effected_colour: {220,220,255}
    effected_sprite: "haste effected"
}

DataW.effect_create {
    name: "Charge"
    can_use_spells: false
    stat_func: (obj, old, new) =>
        new.defence += 5
        new.strength += 5
        new.speed = 0
    remove_func: (obj) =>
        @callback(obj)
    effected_colour: {255,0,0}
    effected_sprite: "charge_effected"
    fade_out: 0
}

DataW.effect_create {
    name: "Haste"
    effected_colour: {160,160,255,240}
    stat_func: (obj, old, new) =>
        new.speed += 2
        new.strength += 2
        new.magic += 2
        new.defence += 4
        new.willpower += 4
        new.cooldown_mult /= 1.25
}

DataW.effect_create {
    name: "Stun"
    effected_sprite: "spr_effects.pain_mirror"
    effected_colour: {200,200,200}
    duration: 15
    stat_func: (obj, old, new) =>
        new.speed = 0
        obj.stats.attack_cooldown = 2
    fade_out: 5
}

DataW.effect_create {
    name: "Thrown"
    effected_sprite: "spr_effects.pain_mirror"
    effected_colour: {200,200,200}
    duration: 15
    apply_func: (obj) =>
        @steps = 0
    step_func: (obj) =>
        increment = @duration - @steps
        if @steps <= @duration
            x = math.floor(obj.x + math.cos(@angle) * increment )
            y = math.floor(obj.y + math.sin(@angle) * increment )
            if not Map.object_solid_check(obj, {x,y})
                obj.x, obj.y = x, y
        @steps += 1
    stat_func: (obj, old, new) =>
        new.speed = 0
        obj.stats.attack_cooldown = 2
    fade_out: 5
}

--# TODO: Asleep enemies dont attack or move until woke
--#    name: "Asleep"
--#    effected_sprite: spr_effects.sleeping
--#    fade_out: 5

DataW.effect_create {
    name: "Pained"
    effected_sprite: "spr_spells.pain"
    effected_colour: {100,100,100}
    fade_out: 15
}

DataW.effect_create {
    name: "Sapped"
    effected_sprite: "spr_effects.manatake"
    fade_out: 15
}

DataW.effect_create {
    name: "Dazed"
    effected_sprite: "spr_effects.may_stab_brand"
    fade_out: 5
    -- Effect in CPP
}


DataW.effect_create {
    name: "AlwaysAttack"
    -- Effect in CPP
}

DataW.effect_create {
    name: "Slowed"
    effected_sprite: "spr_effects.slowed"
    fade_out: 5
    stat_func: (obj, old, new) =>
        new.melee_cooldown_multiplier *= 2
        new.ranged_cooldown_multiplier *= 2
        new.spell_cooldown_multiplier *= 2
        new.speed /= 2
}

DataW.effect_create {
    name: "Regeneration"
    effected_sprite: "spr_amulets.i-regeneration"
    effected_colour: {200,255,200} --RGB
    -- Rest of effect in CPP code
    stat_func: (obj, old, new) =>
        if obj\has_effect("AmuletGreatPain")
            new.hpregen *= 30
        else
            new.hpregen *= 15
        if obj.stats.hp == new.max_hp
            obj\remove_effect("Regeneration")
    fade_out: 55
}

DataW.effect_create {
    name: "Fear"
    effected_sprite: "spr_effects.fleeing"
    effected_colour: {200,255,200}
    -- Rest of effect in CPP code
    stat_func: (obj, old, new) =>
        new.speed *= 1.25
    fade_out: 55
}

--DataW.weapon_poison = tosprite("spr_effects.weapon_poison")
--DataW.weapon_confusion = tosprite("spr_effects.weapon_confusion")
--DataW.weapon_vampirism = tosprite("spr_effects.weapon_vampirism")

M = nilprotect {}
M._poison = tosprite("spr_weapons.i-venom")

M._fire_resist = tosprite("spr_effects.i-r-fire-big")
M._ice_resist = tosprite("spr_effects.i-r-cold-big")
M._generic_resist = tosprite("spr_effects.i-protection-big")
M._poison_resist = tosprite("spr_effects.i-r-poison-big")
M._black_resist = tosprite("spr_effects.black-resist")
M._storm_resist = tosprite("spr_effects.shock-resist")

M._fire_power = tosprite("spr_effects.i-fire-big")
M._ice_power = tosprite("spr_effects.i-cold-big")
M._black_power = tosprite("spr_effects.black-power")
M._storm_power = tosprite("spr_effects.shock")
M._generic_power = tosprite("spr_effects.sword")
M._poison_power = tosprite("spr_effects.i-poison-big")


M._confusion = tosprite("spr_weapons.i-confusion")
M._vampirism = tosprite("spr_weapons.i-vampirism")
M._fleeing = tosprite("spr_effects.fleeing")
M._fear = tosprite("spr_effects.i-life-protection")

draw_console_effect = (sprite, text, xy, color = COL_PALE_YELLOW) ->
    sprite\draw {
        origin: Display.LEFT_CENTER
    }, {xy[1], xy[2] + 4}
    font_cached_load(settings.font, 10)\draw {
        :color
        origin: Display.LEFT_CENTER
    }, {xy[1] + Map.TILE_SIZE + 4, xy[2]}, text

draw_weapon_console_effect = (player, sprite, text, xy, color = COL_PALE_YELLOW) ->
    player.weapon_sprite\draw {
        origin: Display.LEFT_CENTER
    }, {xy[1], xy[2] + 4}
    draw_console_effect(sprite, text, xy, color)

DataW.effect_create {
    name: "FearWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._fear, "+10% chance fear", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Fear")
            return damage
        if chance(.1)
            eff = defender\add_effect("Fear", 150)
        return damage
}

DataW.effect_create {
    name: "ConfusingWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._confusion, "+10% chance daze", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Dazed")
            return damage
        if chance(.1 * @n_derived)
            eff = defender\add_effect("Dazed", 70)
        return damage
}


DataW.effect_create {
    name: "StopOnceInRange"
    -- For centaurs predominantly, coded in C++
}

DataW.effect_create {
    name: "Encumbered"
    stat_func: (obj, old, new) =>
        new.speed -= 1
}

DataW.effect_create {
    name: "Baleful Regeneration",
    effected_sprite: "spr_amulets.i-regeneration"
    effected_colour: {200,255,200}
    step_func: (caster) =>
        {:hp, :max_hp, :hpregen} = caster\effective_stats()
        if hp >= max_hp
            caster\remove_effect("Baleful Regeneration")
        else
            regen_rate = hpregen * (if caster\has_effect("AmuletGreatPain") then 21 else 10)
            caster\heal_hp(regen_rate)
}

for {resist_sprite, power_sprite, type, color} in *{
    {M._poison_resist, M._poison_power, "Green", {226, 66, 244}}
    {M._fire_resist, M._fire_power, "Red", {244, 72, 66}}
    {M._ice_resist, M._ice_power, "Blue", COL_WHITE}
    {M._storm_resist, M._storm_power, "White", COL_BLUE}
    {M._black_resist, M._black_power, "Black", COL_BLUE}
    -- Swords
    {M._generic_resist, M._generic_power, "Slashing", {66, 222, 188}}
    {M._generic_resist, M._generic_power, "Bludgeon", {66, 244, 194}}
    --
    {M._generic_resist, M._generic_power, "Piercing", {66, 244, 214}}
} do
    color = COL_WHITE -- OVERRIDE HACK FOR NOW
    -- Simply a stat variable:
    DataW.additive_effect_create {
        name: "#{type}Resist"
        key: "resist" -- Additive effect, accessed with @value().
        console_draw_func: (player, get_next) =>
            text = "#{type} Resist"
            res = if @resist < 0 then @resist else "+"..@resist
            draw_console_effect(resist_sprite, "#{res} #{text}", get_next(), if @resist >= 1 then color else COL_PALE_RED)
    }
    -- Adds power specifically to attacks of this type
    -- Subform of
    -- TODO separate sprite
    DataW.additive_effect_create {
        name: "#{type}Power"
        key: "power" -- Additive effect, accessed with @value().
        console_draw_func: (player, get_next) =>
            text = "#{type} Power"
            res = if @power < 0 then @power else "+"..@power
            draw_console_effect(power_sprite, "#{res} #{text}", get_next(), if @power >= 0 then color else COL_PALE_RED)
    }
DataW.additive_effect_create {
    name: "PoisonedWeapon"
    key: "poison_percentage" -- Additive effect, accessed with @value().
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._poison, "+#{math.floor(@poison_percentage * 100)}% chance poison", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Poison")
            return damage
        power = TypeEffectUtils.get_power(attacker, "Green")
        resist = TypeEffectUtils.get_resistance(defender, "Green")
        poison_chance = @value()
        if chance(poison_chance * resist)
            eff = defender\add_effect "Poison", {
                time_left: 100
                poison_rate: 25
                :attacker
                damage:  damage * (1 + power/5)
                power: attacker\effective_stats().magic + power
                type_resistance: resist
                magic_percentage: 1.0
            }
        return damage
}

DataW.additive_effect_create {
    name: "Spiky"
    key: "recoil_percentage" -- Additive effect, accessed with @value().
    console_draw_func: (player, get_next) =>
        draw_console_effect(tosprite("spr_spells.spectral_weapon"), "+#{math.floor(@recoil_percentage * 100)}% melee recoil damage", get_next())
    on_receive_melee_func: (attacker, defender, damage) =>
        percentage_recoil = @value()
        attacker\direct_damage(damage * percentage_recoil, defender)
        for _ in screens()
            if defender.is_local_player and defender\is_local_player()
                EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
}

-- These all are implemented by checks in the code:
DataW.effect_create {
    name: "AmuletBerserker"
}

DataW.effect_create {
    name: "AmuletGreatPain"
}

DataW.effect_create {
    name: "AmuletGreaterFire"
}

DataW.effect_create {
    name: "DiesOutsideOfSummonerRange"
    init_func: (summon) =>
        @summoner = false -- TODO think about allowing initial state to be passed to effects!
        -- TODO but OTOH I like the current system.
    step_func: (summon) =>
        assert @summoner, "No summoner set on DiesOutsideOfSummonerRange!"
        if not Map.object_visible summon, summon.xy, @summoner
            summon\direct_damage(10)
}

DataW.effect_create {
    -- Dies at the end of this effect, or on the first attack.
    name: "DiesOnEndOrFirstAttack"
    init_func: () =>
        @n_animations = rand_range(0, 16)
    die: (obj) =>
        BonusesUtils.create_animation @, obj, "spr_bonuses.ghost", 0.5
        obj\die()
    remove_func: (obj) =>
        @die obj
    on_melee_func: (obj, defender, damage) =>
        @die obj
        return damage
    on_projectile_func: (obj, defender, xy) =>
        @die obj
}


DataW.effect_create {
    name: "Summoner"
    init_func: (caster) =>
        @n_summons = 0
    step_func: (caster) =>
        caster.summoned or= OrderedDict()
        @n_summons = 0
        for mon, time in strictpairs caster.summoned
            if not mon.destroyed
                event_log("Desummoning monster (%s, id=%d) at (%.2f, %.2f)", mon.name, math.max(0, mon.id), mon.x, mon.y)
                time_out = (if mon.name == "Spectral Beast" then (time > 600) else (time > 1200)) -- All others are permanents
                diff_floor = (caster.map ~= mon.map)
                if time_out or diff_floor
                    mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                caster.summoned[mon] = nil
            else
                caster.summoned[mon] += 1
                @n_summons += 1
}

-- TODO flip effect of fortification -- TODO whut?
DataW.effect_create {
    name: "Fortification"
    console_draw_func: (player, get_next) =>
        draw_console_effect(tosprite("spr_spells.statue_form"), "Gain defence when hit", get_next())
    init_func: (caster) =>
        @active_bonuses = OrderedDict()
        @duration = 320
    stat_func: (obj, old, new) =>
        for k, v in strictpairs @active_bonuses -- Abuse that stat_func is called every frame
            if v == 0
                for _ in screens()
                    EventLog.add("Your defence falls back down...", COL_PALE_BLUE)
                @active_bonuses[k] = nil
            else
                new.defence += 2
                @active_bonuses[k] -= 1
    on_receive_damage_func: (attacker, defender, damage) =>
        attacker_key = if attacker and attacker.id then attacker.id else "environment"
        if not @active_bonuses[attacker_key]
            for _ in screens()
                EventLog.add("Your defence rises due to getting hit!", COL_PALE_BLUE)
            @active_bonuses[attacker_key] = @duration
        elseif @active_bonuses[attacker_key] < @duration
            @active_bonuses[attacker_key] = @duration
        return damage

}

DataW.effect_create {
    name: "Ludaze"
    effected_sprite: "spr_spells.ludaze"
    can_use_rest: false
    effected_colour: {200, 200, 255, 200}
    fade_out: 10
}

DataW.effect_create {
    name: "Ice Form"
    effected_sprite: "spr_spells.iceform"
    can_use_rest: false
    can_use_spells: false
    can_use_stairs: true -- Important!!
    can_use_weapons: false
    effected_colour: {200, 200, 255}
    stat_func: (effect, obj, old, new) ->
        new.resistance += 100
        new.magic_resistance += 100
        new.speed = math.min(new.speed, 0.5)
    fade_out: 55
}

-------  START EQUIPMENT SLOT STACKED EFFECTS --------
-- Hack because effects are not currently easily stacked otherwise:
for equip_slot in *{"", "Armour", "Amulet", "Ring", "Belt", "Weapon", "Legwear"}
    DataW.effect_create {
        name: "#{equip_slot}HealthGainOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                for _ in screens()
                    if caster\is_local_player()
                        EventLog.add("You regain health for killing!", COL_PALE_BLUE)
                caster\heal_hp(5 + caster.stats.level)
                play_sound "sound/swish-10.wav"
                BonusesUtils.create_animation {n_animations: 0}, caster, "spr_effects.health", 0.5
                @kill_tracker += 1
    }
    DataW.effect_create {
        name: "#{equip_slot}ManaGainOnKill"
        category: "EquipEffect"
        init_func: (caster) =>
            @kill_tracker = caster.kills
        step_func: (caster) =>
            while caster.kills > @kill_tracker
                for _ in screens()
                    if caster\is_local_player()
                        EventLog.add("You regain mana for killing!", COL_PALE_BLUE)
                caster\heal_mp(5 + caster.stats.level)
                play_sound "sound/swish-10.wav"
                BonusesUtils.create_animation {n_animations: 0}, caster, "spr_effects.mana", 0.5
                @kill_tracker += 1
    }

DataW.effect_create {
    name: "PossiblySummonCentaurOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("spr_enemies.humanoid.centaur"), "Can summon after kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        if caster\has_effect "Summoning"
            return
        while caster.kills > @kill_tracker
            if chance(0.03 * @n_derived)
                for _ in screens()
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                play_sound "sound/summon.ogg"
                eff = caster\add_effect("Summoning", 5)
                eff.monster = "Centaur Hunter"
                eff.duration = 5
            @kill_tracker += 1
}

DataW.effect_create {
    name: "SummonMummyOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("spr_enemies.undead.mummy"), "Summons help every kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        if caster\has_effect "Summoning"
            return
        while caster.kills > @kill_tracker
            for _ in screens()
                EventLog.add("A mummy is summoned due to your graceful killing!!", COL_PALE_BLUE)
            play_sound "sound/summon.ogg"
            eff = caster\add_effect("Summoning", 5)
            eff.monster = "Mummy"
            eff.duration = 5
            @kill_tracker += 1
}

DataW.effect_create {
    name: "PossiblySummonStormElementalOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("storm elemental"), "Can appear after a kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        if caster\has_effect "Summoning"
            return
        while caster.kills > @kill_tracker
            if chance(0.05 * @n_derived)
                for _ in screens()
                    EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
                play_sound "sound/summon.ogg"
                monster = "Storm Elemental"
                eff = caster\add_effect("Summoning", 5)
                eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                eff.duration = 5
            @kill_tracker += 1
}

DataW.effect_create {
    name: "PossiblySummonGolemOnKill"
    --console_draw_func: (player, get_next) =>
    --    draw_console_effect(tosprite("golem"), "Can appear after a kill", get_next())
    category: "EquipEffect"
    init_func: (caster) =>
        @kill_tracker = caster.kills
    step_func: (caster) =>
        while caster.kills > @kill_tracker
            if chance(0.03 * @n_derived)
                for _ in screens()
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

DataW.effect_create {
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
        @summon_xy = false
        @start_map = caster.map
        caster.summoned or= OrderedDict()
    step_func: (caster) =>
        @n_steps += 1
        if @n_steps == @delay
            if caster.map == @start_map
                ability = SpellObjects.SummonAbility.create {monster: @monster, :caster, xy: @summon_xy or caster.xy, duration: @duration, on_summon: @on_summon}
                GameObject.add_to_level(ability)
}

DataW.effect_create {
    name: "Reviving"
    remove_func: (player) =>
        player.is_ghost = false
        player.stats.hp = player\effective_stats().max_hp -- / 2 TODO should ghosts have half HP?
    draw_func: (player, top_left_x, top_left_y) =>
        xy = Display.to_screen_xy player.xy
        Fonts.small\draw({origin: Display.CENTER}, xy, "#{math.ceil @time_left / 60}")
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

DataW.effect_create {
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

DataW.effect_create {
    name: "Healing"
    category: "Aura"
    effected_sprite: "spr_amulets.healing"
    fade_out: 25
    step_func: (obj) =>
        {:hpregen} = obj\effective_stats()
        obj\heal_hp(hpregen * 20)
}

DataW.effect_create {
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

DataW.effect_create {
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

DataW.effect_create {
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
                for _ in screens()
                    if mon\is_local_player()
                        EventLog.add("Your MP is drained!", {200,200,255})
                    elseif not mon.is_enemy
                        EventLog.add(mon.name .. "'s MP is drained!", {200,200,255})
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        AuraBase.draw(@, COL_PALE_BLUE, COL_BLUE, caster.x, caster.y)
}

DataW.effect_create {
    name: "Pain Aura Anim Hack"
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
    draw_func: (caster, top_left_x, top_left_y) =>
        @max_alpha = 0.35
        if @animation_only
            @max_alpha /= 4
        elseif not caster.is_enemy and not caster\has_effect "Pained"
            @max_alpha /= 2
        AuraBase.draw(@, COL_PALE_RED, COL_RED, caster.x, caster.y)
}

DataW.effect_create {
    name: "Pain Aura"
    category: "Aura"
    effected_sprite: "spr_spells.greaterpain"
    fade_out: 50
    init_func: (caster) =>
        AuraBase.init(@, caster)
    apply_func: (caster, args) =>
        @mp_gain = 10
        @damage = args.damage
        @range = args.range
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
                stats = caster\effective_stats()
                damage, power = @damage, random(2,5) + stats.magic
                power = power + TypeEffectUtils.get_power(caster, "Black")
                type_multiplier = TypeEffectUtils.get_resistance(mon, "Black")
                if mon\damage(damage, power, 1, caster, type_multiplier) then
                    {:stats} = caster
                    {:max_hp} = mon\effective_stats()
                    @damage += 5 -- Damage creep
                    @time_left += 100 -- Effect creep
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

DataW.effect_create {
    name: "VampiricWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._vampirism, "Heal +25% dealt", get_next())
    on_melee_func: (attacker, defender, damage) =>
        for _ in screens()
            if attacker\is_local_player()
                EventLog.add("You steal the enemy's life!", {200,200,255})
        attacker\heal_hp(damage / 4 * @n_derived)
        return damage
}


DataW.effect_create {
    name: "Abolishment"
    stat_func: (effect, obj, old, new) ->
        new.strength += math.ceil(new.magic / 3)
        new.magic = 0
}

DataW.effect_create {
    name: "EnemyHyperProjectile"
    stat_func: (effect, obj, old, new) ->
        new.ranged_cooldown_multiplier *= 0.1
}

DataW.effect_create {
    name: "KnockbackWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._fleeing, "+10% chance of knockback", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Thrown")
            return damage
        if chance(.1 * @n_derived)
            thrown = defender\add_effect("Thrown", 20)
            thrown.angle = vector_direction({attacker.x, attacker.y}, {defender.x, defender.y})
        return damage
}

DataW.effect_create {
    name: "StunWeapon"
    console_draw_func: (player, get_next) =>
        draw_weapon_console_effect(player, M._fleeing, "+10% chance of stun", get_next())
    on_melee_func: (attacker, defender, damage) =>
        if defender\has_effect("Stun")
            return damage
        if chance(.1 * @n_derived)
            thrown = defender\add_effect("Stun", 20)
            thrown.angle = vector_direction({attacker.x, attacker.y}, {defender.x, defender.y})
        return damage
}

share_damage = (target, damage, min_health) ->
    -- TODO
    min_health = 0
    if target.stats.hp - damage < min_health
        damage = math.max(0, target.stats.hp - min_health)
    target.stats.hp -= damage

LAST_WARNING = GameVar.create("LAST_WARNING", -math.huge)
for name in *{"Ranger", "Fighter", "Templar", "Rogue", "Green Mage", "Black Mage", "Necromancer", "Stormcaller", "Pyrocaster", "Blue Mage"}
    DataW.effect_create {
        :name
        stat_func: (obj, old, new) =>
            if name ~= "Fighter"
                new.melee_cooldown_multiplier *= 1.25
            if name ~= "Ranger" or name ~= "Fighter"
                new.ranged_cooldown_multiplier *= 1.25

        init_func: (caster) =>
            @kill_tracker = caster.kills
            @links = {}
            @n_animations = 0
            LAST_WARNING\set -math.huge -- HACK
        step_func: (caster) =>
            -- Keep state for doing summons in a stateful effect:
            if caster\has_effect "Summoner"
                caster\add_effect "Summoner", 2 -- Keep effect from dying
            else
                eff = caster\add_effect "Summoner", 2
                eff.duration = 30
            while caster.kills > @kill_tracker
                --if name == "Necromancer"
                --    for _ in screens()
                --        if caster\is_local_player()
                --            EventLog.add("You gain mana for killing!", COL_PALE_BLUE)
                --    caster\heal_mp(5)
                for {:instance, :class_name} in *World.players
                    if class_name == "Necromancer"
                        for _ in screens()
                            if instance\is_local_player()
                                EventLog.add("You feed on the dying monster's energy!", COL_PALE_BLUE)
                        istats = instance.stats
                        if istats.mp < istats.max_mp
                            BonusesUtils.create_animation @, instance, "spr_bonuses.mana", 0.5
                        instance\heal_mp(10)
                @kill_tracker += 1
        -- TODO reconsider whether Necro needs corrosive skin
        --on_receive_melee_func: (attacker, caster, damage) =>
        --    if name == "Necromancer"
        --        attacker\direct_damage(damage * 0.33, caster)
        --        for _ in screens()
        --            if caster\is_local_player()
        --                EventLog.add("Your corrosive flesh hurts #{attacker.name} as you are hit!", COL_PALE_BLUE)
        --    if GameState.frame > LAST_WARNING\get() + 100 and not caster.is_ghost
        --        low_num = 50
        --        if name == "Necromancer"
        --            low_num = 30
        --        if caster.stats.hp - damage <= low_num or caster.stats.hp - damage <= caster\effective_stats().max_hp * 0.1
        --            LAST_WARNING\set GameState.frame
        --            play_sound "sound/allyislow1c.ogg"
        --    return damage
    }

DICE_ROLL_INTERVAL = 240
DICE_BUFF_DURATION = 150
DICE_ROLL_ANIMATION = 84
DICE_ROLL_REVEAL_FRAMES = 18
DICE_ROLL_FINAL_REVEAL_FRAMES = 8
DICE_ROLL_TICK_PERIOD = 4
DICE_MINOR_COOLDOWN_TRIM = 45
DICE_ROLL_LOCK_STAGE_FRAMES = {14, 34, 58, DICE_ROLL_ANIMATION - DICE_ROLL_FINAL_REVEAL_FRAMES}
DICE_ROLL_LOCK_LABELS = {"PIPS", "TUMBLE", "BANK", "FACE"}
DICE_ROLL_SURGE_STAGE_FRAMES = {8, 22, 38, 56, 72}
DICE_ROLL_SURGE_LABELS = {"SHAKE", "RATTLE", "SPIN", "DROP", "LOCK"}
DICE_ROLL_LOCK_TOTAL = 4
DICE_ROLL_SURGE_TOTAL = 5
DICE_INITIAL_ROLL_DELAY = DICE_ROLL_INTERVAL - DICE_BUFF_DURATION - DICE_ROLL_ANIMATION
DICE_FULL_BAG_MASK = 63
DICE_TWO_PI = math.pi * 2
DICE_FACE_BITS = {1, 2, 4, 8, 16, 32}
DICE_ROLLS = {
    [1]: {kind: "fleet", label: "Fleet step", short: "SPD", color: COL_PALE_GREEN}
    [2]: {kind: "guard", label: "Guard stance", short: "GRD", color: COL_PALE_BLUE}
    [3]: {kind: "edge", label: "Killing edge", short: "DMG", color: COL_PALE_RED}
    [4]: {kind: "focus", label: "Flow state", short: "CDR", color: COL_PALE_YELLOW}
    [5]: {kind: "dash", label: "Dash ready", short: "DASH", color: COL_WHITE}
    [6]: {kind: "jackpot", label: "Power ready", short: "PWR", color: COL_PALE_YELLOW}
}

dicebinder_increment = (effect, key, amount=1) ->
    effect[key] = (effect[key] or 0) + amount

dicebinder_face_available = (mask, face) ->
    bit = DICE_FACE_BITS[face] or 0
    bit > 0 and math.floor(mask / bit) % 2 >= 1

dicebinder_bag_count = (mask) ->
    count = 0
    for face=1,6
        count += 1 if dicebinder_face_available(mask, face)
    count

dicebinder_remove_from_bag = (mask, face) ->
    bit = DICE_FACE_BITS[face] or 0
    if bit > 0 and dicebinder_face_available(mask, face)
        return mask - bit
    mask

dicebinder_mask_without_faces = (mask, faces) ->
    reduced = mask
    for face in *faces
        reduced = dicebinder_remove_from_bag(reduced, face)
    reduced

dicebinder_mask_only_faces = (mask, faces) ->
    filtered = 0
    for face in *faces
        if dicebinder_face_available(mask, face)
            filtered += DICE_FACE_BITS[face] or 0
    filtered

dicebinder_roll_lock_stage = (elapsed) ->
    stage = 0
    for idx, frame in ipairs DICE_ROLL_LOCK_STAGE_FRAMES
        if elapsed >= frame
            stage = idx
    stage

dicebinder_roll_surge_stage = (elapsed) ->
    stage = 0
    for idx, frame in ipairs DICE_ROLL_SURGE_STAGE_FRAMES
        if elapsed >= frame
            stage = idx
    stage

dicebinder_next_fair_roll = (effect, avoid_face=0) ->
    effect.pending_guard_kind = "none"
    mask = effect.bag_mask or DICE_FULL_BAG_MASK
    remaining = dicebinder_bag_count(mask)
    refilled = false
    if remaining <= 0
        mask = DICE_FULL_BAG_MASK
        remaining = 6
        refilled = true
        dicebinder_increment(effect, "bag_refills")
    pick_mask = mask
    pick_remaining = remaining
    if refilled and avoid_face and avoid_face > 0 and remaining > 1 and dicebinder_face_available(mask, avoid_face)
        pick_mask = dicebinder_remove_from_bag(mask, avoid_face)
        pick_remaining = remaining - 1
        dicebinder_increment(effect, "anti_repeat_rerolls")
    if (effect.current_high_streak or 0) >= 2
        guarded_mask = dicebinder_mask_without_faces(pick_mask, {5, 6})
        guarded_remaining = dicebinder_bag_count(guarded_mask)
        if guarded_remaining > 0
            pick_mask = guarded_mask
            pick_remaining = guarded_remaining
            effect.pending_guard_kind = "bank"
            dicebinder_increment(effect, "high_streak_breaks")
    elseif (effect.current_utility_streak or 0) >= 3
        guarded_mask = dicebinder_mask_only_faces(pick_mask, {5, 6})
        guarded_remaining = dicebinder_bag_count(guarded_mask)
        if guarded_remaining > 0
            pick_mask = guarded_mask
            pick_remaining = guarded_remaining
            effect.pending_guard_kind = "surge"
            dicebinder_increment(effect, "low_streak_breaks")
    pick = math.random(1, pick_remaining)
    seen = 0
    for face=1,6
        if dicebinder_face_available(pick_mask, face)
            seen += 1
            if seen == pick
                effect.bag_mask = dicebinder_remove_from_bag(mask, face)
                return face
    effect.bag_mask = DICE_FULL_BAG_MASK - 1
    1

dicebinder_near_miss_roll = (roll) ->
    direction = if math.random(1, 2) == 1 then -1 else 1
    near = roll + direction
    if near < 1
        near = 2
    elseif near > 6
        near = 5
    near

dicebinder_visible_roll = (effect) ->
    if (effect.rolling_time or 0) <= DICE_ROLL_FINAL_REVEAL_FRAMES and effect.pending_roll and effect.pending_roll > 0
        return effect.pending_roll
    if (effect.rolling_time or 0) <= DICE_ROLL_REVEAL_FRAMES and effect.near_miss_roll and effect.near_miss_roll > 0
        return effect.near_miss_roll
    if effect.preview_roll and effect.preview_roll > 0
        return effect.preview_roll
    1

dicebinder_record_refresh = (effect, caster, spell_name, total_key, effective_key) ->
    dicebinder_increment(effect, total_key)
    if caster\spell_cooldown(spell_name) > 0
        dicebinder_increment(effect, effective_key)

dicebinder_trim_cooldown = (effect, caster, spell_name, frames, total_key, effective_key) ->
    current = caster\spell_cooldown(spell_name) or 0
    return 0 unless current > 0
    shaved = math.min(current, frames)
    caster\set_spell_cooldown(spell_name, math.max(0, current - frames))
    dicebinder_increment(effect, total_key, shaved)
    dicebinder_increment(effect, effective_key)
    shaved

dicebinder_record_roll = (effect, caster, roll, entry, previous_roll) ->
    dicebinder_increment(effect, "rolls_total")
    dicebinder_increment(effect, "roll_#{roll}_count")
    if roll >= 5
        dicebinder_increment(effect, "high_rolls")
        effect.current_high_streak = (effect.current_high_streak or 0) + 1
        effect.longest_high_streak = math.max(effect.longest_high_streak or 0, effect.current_high_streak)
        effect.current_utility_streak = 0
    else
        dicebinder_increment(effect, "utility_rolls")
        effect.current_high_streak = 0
        effect.current_utility_streak = (effect.current_utility_streak or 0) + 1
        effect.longest_utility_streak = math.max(effect.longest_utility_streak or 0, effect.current_utility_streak)
    if previous_roll and previous_roll > 0
        if previous_roll == roll
            dicebinder_increment(effect, "repeat_rolls")
        else
            dicebinder_increment(effect, "roll_variety_switches")
    if roll == 5
        dicebinder_record_refresh(effect, caster, "Dash Attack", "dash_refreshes", "dash_effective_refreshes")
    elseif roll == 6
        dicebinder_increment(effect, "jackpot_rolls")
        dicebinder_record_refresh(effect, caster, "Dash Attack", "dash_refreshes", "dash_effective_refreshes")
        dicebinder_record_refresh(effect, caster, "Power Strike", "power_refreshes", "power_effective_refreshes")

dicebinder_emit_roll_fx = (caster, roll, entry) ->
    major = roll >= 5
    ImpactFX.world_burst {
        map: caster.map
        xy: caster.xy
        color: entry.color
        label: "D6 #{roll}"
        radius: (if major then 82 else 58 + roll * 4)
        duration: (if major then 46 else 30 + roll * 2)
    }

    pip_count = math.max(2, math.min(6, roll))
    spread = 26 + roll * 5
    for idx=1,pip_count
        angle = (GameState.frame or 0) * 0.037 + idx * math.pi * 2 / pip_count
        ImpactFX.world_burst {
            map: caster.map
            xy: {
                caster.x + math.cos(angle) * spread
                caster.y + math.sin(angle) * spread
            }
            color: entry.color
            radius: (if major then 28 else 18)
            duration: 18 + roll * 3
            drift: 0.18
        }

    if roll >= 4
        ImpactFX.screen_flash {
            map: caster.map
            color: entry.color
            alpha: (if roll == 6 then 0.24 elseif roll == 5 then 0.16 else 0.10)
            duration: (if roll == 6 then 16 else 12)
        }

dicebinder_emit_roll_lock_fx = (effect, caster, stage, roll, entry) ->
    dicebinder_increment(effect, "roll_lock_cues")
    pip_count = math.min(6, math.max(2, stage * 2))
    dicebinder_increment(effect, "roll_suspense_pips", pip_count)
    color = if stage >= 3 then entry.color else COL_WHITE
    label = DICE_ROLL_LOCK_LABELS[stage] or "LOCK"
    radius = 26 + stage * 14
    ImpactFX.world_burst {
        map: caster.map
        xy: caster.xy
        color: color
        label: label
        radius: 42 + stage * 14
        duration: 18 + stage * 4
        drift: 0.18
    }
    frame = GameState.frame or 0
    for idx=1,pip_count
        angle = frame * 0.061 + idx * DICE_TWO_PI / pip_count + stage * 0.42
        ImpactFX.world_burst {
            map: caster.map
            xy: {
                caster.x + math.cos(angle) * radius
                caster.y + math.sin(angle) * radius
            }
            color: color
            radius: 12 + stage * 3
            duration: 14 + stage * 4
            drift: 0.08
        }
    if stage >= 3
        ImpactFX.screen_flash {
            map: caster.map
            color: entry.color
            alpha: if roll >= 5 then 0.10 else 0.055
            duration: 7
        }

dicebinder_emit_roll_surge_fx = (effect, caster, stage, roll, entry) ->
    dicebinder_increment(effect, "roll_surge_cues")
    color = if stage >= 4 then entry.color else COL_WHITE
    label = DICE_ROLL_SURGE_LABELS[stage] or "ROLL"
    radius = 30 + stage * 12
    ImpactFX.world_burst {
        map: caster.map
        xy: caster.xy
        color: color
        label: label
        radius: radius
        duration: 14 + stage * 4
        drift: 0.10
    }
    frame = GameState.frame or 0
    pip_count = math.min(6, 2 + stage)
    for idx=1,pip_count
        angle = frame * (0.09 + stage * 0.010) + idx * DICE_TWO_PI / pip_count
        ImpactFX.world_burst {
            map: caster.map
            xy: {
                caster.x + math.cos(angle) * (radius * 0.62)
                caster.y + math.sin(angle) * (radius * 0.62)
            }
            color: color
            radius: 9 + stage * 2
            duration: 11 + stage * 3
            drift: 0.06
        }
    if stage >= 5
        ImpactFX.screen_flash {
            map: caster.map
            color: entry.color
            alpha: if roll >= 5 then 0.12 else 0.06
            duration: 8
        }

dicebinder_emit_roll_guard_fx = (effect, caster) ->
    guard = effect.roll_guard_kind or "none"
    return if guard == "none"
    dicebinder_increment(effect, "roll_guard_cues")
    label = if guard == "surge" then "SURGE" else "BANK"
    color = if guard == "surge" then COL_PALE_YELLOW else COL_PALE_BLUE
    ImpactFX.world_burst {
        map: caster.map
        xy: caster.xy
        color: color
        label: label
        radius: if guard == "surge" then 84 else 66
        duration: 34
        drift: 0.12
    }
    ImpactFX.screen_flash {
        map: caster.map
        color: color
        alpha: if guard == "surge" then 0.08 else 0.045
        duration: 8
    }

dicebinder_update_roll_lock = (effect, caster, elapsed) ->
    stage = dicebinder_roll_lock_stage(elapsed)
    previous = effect.roll_lock_stage or 0
    return unless stage > previous
    roll = dicebinder_visible_roll(effect)
    entry = DICE_ROLLS[roll] or DICE_ROLLS[1]
    for lock_stage=previous + 1, stage
        dicebinder_emit_roll_lock_fx(effect, caster, lock_stage, roll, entry)
    effect.roll_lock_stage = stage

dicebinder_update_roll_surge = (effect, caster, elapsed) ->
    stage = dicebinder_roll_surge_stage(elapsed)
    previous = effect.roll_surge_stage or 0
    return unless stage > previous
    roll = dicebinder_visible_roll(effect)
    entry = DICE_ROLLS[roll] or DICE_ROLLS[1]
    for surge_stage=previous + 1, stage
        dicebinder_emit_roll_surge_fx(effect, caster, surge_stage, roll, entry)
    effect.roll_surge_stage = stage

dicebinder_emit_roll_tick_fx = (effect, caster, elapsed) ->
    return unless elapsed > 0 and elapsed % DICE_ROLL_TICK_PERIOD == 0
    total = math.max(1, effect.rolling_total or DICE_ROLL_ANIMATION)
    pct = math.max(0, math.min(1, elapsed / total))
    roll = dicebinder_visible_roll(effect)
    entry = DICE_ROLLS[roll] or DICE_ROLLS[1]
    pip_bonus = if pct > 0.65 then 1 else 0
    pip_count = math.max(3, math.min(6, roll + pip_bonus))
    radius = 24 + pct * 44
    burst_radius = 10 + pct * 12
    frame = GameState.frame or 0
    dicebinder_increment(effect, "roll_buildup_fx")
    for idx=1,pip_count
        angle = frame * (0.10 + pct * 0.05) + idx * DICE_TWO_PI / pip_count + pct * 1.4
        ImpactFX.world_burst {
            map: caster.map
            xy: {
                caster.x + math.cos(angle) * radius
                caster.y + math.sin(angle) * radius
            }
            color: entry.color
            radius: burst_radius
            duration: 10 + math.floor(pct * 9)
            drift: 0.10
        }
        if pct > 0.52 and idx % 2 == 0
            counter_angle = -angle + pct * 2.1
            ImpactFX.world_burst {
                map: caster.map
                xy: {
                    caster.x + math.cos(counter_angle) * (radius * 0.58)
                    caster.y + math.sin(counter_angle) * (radius * 0.58)
                }
                color: entry.color
                radius: burst_radius * 0.72
                duration: 8 + math.floor(pct * 8)
                drift: 0.06
            }
    if (effect.rolling_time or 0) <= DICE_ROLL_FINAL_REVEAL_FRAMES and not effect.roll_reveal_fx_emitted
        effect.roll_reveal_fx_emitted = true
        dicebinder_increment(effect, "roll_reveal_cues")
        ImpactFX.screen_flash {
            map: caster.map
            color: entry.color
            alpha: if roll == 6 then 0.14 elseif roll == 5 then 0.10 else 0.05
            duration: 8
        }

dicebinder_resolve_roll = (effect, caster) ->
    previous_roll = effect.last_roll or 0
    roll = if effect.pending_roll and effect.pending_roll > 0 then effect.pending_roll else dicebinder_next_fair_roll(effect, previous_roll)
    entry = DICE_ROLLS[roll]
    effect.last_roll = roll
    effect.roll_color = entry.color
    effect.buff_kind = entry.kind
    effect.buff_time = DICE_BUFF_DURATION
    effect.pending_roll = 0
    effect.near_miss_roll = 0
    effect.roll_guard_kind = "none"
    effect.preview_roll = roll
    effect.rolling_time = 0
    effect.next_roll = math.max(1, (effect.roll_interval or DICE_ROLL_INTERVAL) - DICE_ROLL_ANIMATION)
    dicebinder_record_roll(effect, caster, roll, entry, previous_roll)

    if roll == 4
        dicebinder_trim_cooldown(effect, caster, "Dash Attack", DICE_MINOR_COOLDOWN_TRIM, "cooldown_trim_frames", "dash_trim_effective")
        dicebinder_trim_cooldown(effect, caster, "Power Strike", math.floor(DICE_MINOR_COOLDOWN_TRIM * 0.75), "cooldown_trim_frames", "power_trim_effective")
    elseif roll == 5
        caster\set_spell_cooldown("Dash Attack", 0)
    elseif roll == 6
        caster\set_spell_cooldown("Power Strike", 0)
        caster\set_spell_cooldown("Dash Attack", 0)
        caster\heal_hp(4 + caster.stats.level)
        caster\heal_mp(8 + caster.stats.level)

    dicebinder_increment(effect, "roll_finale_fx")
    dicebinder_emit_roll_fx(caster, roll, entry)

    for _ in screens()
        if caster\is_local_player()
            EventLog.add("Skill die #{roll}: #{entry.label}.", entry.color)

dicebinder_start_roll = (effect, caster) ->
    effect.pending_roll = dicebinder_next_fair_roll(effect, effect.last_roll or 0)
    effect.roll_guard_kind = effect.pending_guard_kind or "none"
    effect.near_miss_roll = dicebinder_near_miss_roll(effect.pending_roll)
    effect.preview_roll = math.random(1, 6)
    effect.rolling_time = DICE_ROLL_ANIMATION
    effect.rolling_total = DICE_ROLL_ANIMATION
    effect.roll_reveal_fx_emitted = false
    effect.roll_lock_stage = 0
    effect.roll_surge_stage = 0
    effect.roll_spin_seed = math.random(1, 360)
    effect.roll_color = COL_WHITE
    dicebinder_increment(effect, "roll_animations_total")
    ImpactFX.world_burst {
        map: caster.map
        xy: caster.xy
        color: COL_WHITE
        label: "D6"
        radius: 72
        duration: DICE_ROLL_ANIMATION
    }
    dicebinder_emit_roll_guard_fx(effect, caster)

dicebinder_step_roll_animation = (effect, caster) ->
    dicebinder_increment(effect, "roll_animation_frames")
    previous_shown = dicebinder_visible_roll(effect)
    effect.rolling_time = math.max(0, (effect.rolling_time or 0) - 1)
    elapsed = (effect.rolling_total or DICE_ROLL_ANIMATION) - effect.rolling_time
    period = if elapsed < 24 then 2 elseif elapsed < 48 then 3 else 5
    if effect.rolling_time <= DICE_ROLL_FINAL_REVEAL_FRAMES
        effect.preview_roll = if effect.pending_roll and effect.pending_roll > 0 then effect.pending_roll else effect.preview_roll or 1
    elseif effect.rolling_time <= DICE_ROLL_REVEAL_FRAMES
        effect.preview_roll = if effect.near_miss_roll and effect.near_miss_roll > 0 then effect.near_miss_roll else effect.preview_roll or 1
    elseif elapsed % period == 0
        spin_seed = effect.roll_spin_seed or 1
        jump = 1 + ((spin_seed + elapsed) % 5)
        effect.preview_roll = ((effect.preview_roll or 0) + jump - 1) % 6 + 1
    if dicebinder_visible_roll(effect) ~= previous_shown
        dicebinder_increment(effect, "roll_face_swaps")
    dicebinder_update_roll_surge(effect, caster, elapsed)
    dicebinder_update_roll_lock(effect, caster, elapsed)
    dicebinder_emit_roll_tick_fx(effect, caster, elapsed)
    if effect.rolling_time <= 0
        dicebinder_resolve_roll(effect, caster)

DataW.effect_create {
    name: "Dicebinder"
    stat_func: (obj, old, new) =>
        new.ranged_cooldown_multiplier *= 1.25
        if @buff_time and @buff_time > 0
            switch @buff_kind
                when "fleet"
                    new.speed += 0.7
                when "guard"
                    new.defence += 2
                    new.willpower += 2
                when "edge"
                    new.strength += 2
                    new.melee_cooldown_multiplier *= 0.97
                when "focus"
                    new.spell_cooldown_multiplier *= 0.84
                when "dash"
                    new.speed += 1.0
                    new.melee_cooldown_multiplier *= 0.96
                when "jackpot"
                    new.speed += 0.45
                    new.melee_cooldown_multiplier *= 0.90
                    new.spell_cooldown_multiplier *= 0.90
    init_func: (caster) =>
        @roll_interval = DICE_ROLL_INTERVAL
        @next_roll = DICE_INITIAL_ROLL_DELAY
        @last_roll = 0
        @roll_color = COL_PALE_BLUE
        @buff_kind = "none"
        @buff_time = 0
        @pending_roll = 0
        @pending_guard_kind = "none"
        @roll_guard_kind = "none"
        @near_miss_roll = 0
        @preview_roll = 0
        @rolling_time = 0
        @rolling_total = DICE_ROLL_ANIMATION
        @roll_reveal_fx_emitted = false
        @roll_lock_stage = 0
        @roll_surge_stage = 0
        @roll_spin_seed = 1
        @bag_mask = DICE_FULL_BAG_MASK
        @bag_refills = 0
        @anti_repeat_rerolls = 0
        @roll_animations_total = 0
        @roll_animation_frames = 0
        @roll_buildup_fx = 0
        @roll_face_swaps = 0
        @roll_reveal_cues = 0
        @roll_lock_cues = 0
        @roll_surge_cues = 0
        @roll_finale_fx = 0
        @roll_suspense_pips = 0
        @roll_guard_cues = 0
        @cooldown_trim_frames = 0
        @dash_trim_effective = 0
        @power_trim_effective = 0
        @rolls_total = 0
        @high_rolls = 0
        @utility_rolls = 0
        @current_high_streak = 0
        @longest_high_streak = 0
        @current_utility_streak = 0
        @longest_utility_streak = 0
        @high_streak_breaks = 0
        @low_streak_breaks = 0
        @repeat_rolls = 0
        @roll_variety_switches = 0
        @dash_refreshes = 0
        @dash_effective_refreshes = 0
        @power_refreshes = 0
        @power_effective_refreshes = 0
        @jackpot_rolls = 0
        @buff_active_frames = 0
        for roll=1,6
            @["roll_#{roll}_count"] = 0
    step_func: (caster) =>
        if @buff_time and @buff_time > 0
            dicebinder_increment(@, "buff_active_frames")
            dicebinder_increment(@, "#{@buff_kind}_frames") if @buff_kind and @buff_kind ~= "none"
            @buff_time -= 1
        if @rolling_time and @rolling_time > 0
            dicebinder_step_roll_animation(@, caster)
            return
        @next_roll -= 1
        if @next_roll <= 0
            dicebinder_start_roll(@, caster)
    draw_func: (caster) =>
        rolling = @rolling_time and @rolling_time > 0
        shown_roll = if rolling
            dicebinder_visible_roll(@)
        else @last_roll or 0
        entry = DICE_ROLLS[shown_roll]
        rolling_label = if rolling
            guard = @roll_guard_kind or "none"
            elapsed = (@rolling_total or DICE_ROLL_ANIMATION) - (@rolling_time or 0)
            if guard == "surge" and elapsed <= 22 then "SURGE"
            elseif guard == "bank" and elapsed <= 22 then "BANK"
            elseif (@rolling_time or 0) <= DICE_ROLL_FINAL_REVEAL_FRAMES and entry then entry.short
            elseif (@rolling_time or 0) <= DICE_ROLL_REVEAL_FRAMES then "DROP"
            else "ROLL"
        else nil
        ProceduralEffects.draw_die_badge caster, {
            roll: shown_roll
            remaining: @next_roll or 0
            interval: @roll_interval or DICE_ROLL_INTERVAL
            color: (rolling and entry and entry.color) or @roll_color or COL_PALE_BLUE
            label: rolling_label or (entry and entry.short) or ""
            buff_remaining: @buff_time or 0
            buff_duration: DICE_BUFF_DURATION
            rolling: rolling
            rolling_remaining: @rolling_time or 0
            rolling_total: @rolling_total or DICE_ROLL_ANIMATION
            lock_stage: @roll_lock_stage or 0
            lock_total: DICE_ROLL_LOCK_TOTAL
            surge_stage: @roll_surge_stage or 0
            surge_total: DICE_ROLL_SURGE_TOTAL
        }
}

-- Move in a direction, attacking everyone along the way
DataW.effect_create {
    name: "Dash Attack"
    effected_sprite: "spr_effects.i-stealth"
    can_use_rest: false
    effected_colour: {200, 200, 255, 255}
    fade_out: 10
    init_func: (caster) =>
        if Map.object_visible(caster)
            play_sound "sound/swish-12.wav"
        @steps = 0
        @n_hits = 0
        @attacked = {} -- No one attacked at first
    apply_func: (caster, args) =>
        @dir_vector = {math.cos(args.angle) * 16, math.sin(args.angle) * 16}
    step_func: (caster) =>
        @steps += 1
        -- Move forward best as possible:
        dxy = @dir_vector
        if caster.configure_dir
            dxy = caster\configure_dir(dxy)
        if dxy[1] == @dir_vector[1] and dxy[2] == @dir_vector[2]
            @dir_vector = GameObject.simulate_bounce(caster, @dir_vector)
            dxy = @dir_vector
        xy = {caster.x + dxy[1], caster.y + dxy[2]}
        if Map.object_tile_check(caster, xy)
            if Map.object_visible(caster)
                play_sound "sound/door.ogg"
            caster\remove_effect "Dash Attack"
            return
        caster.xy = xy
        if caster\has_ranged_weapon()
            return
        -- Attack nearby monsters:
        for mon in *Map.enemies_list(caster)
            dist = vector_distance(mon.xy, caster.xy)
            if dist < caster.target_radius + mon.target_radius --+ caster.weapon_range
                if @attacked[mon.id]
                    continue
                @attacked[mon.id] = true
                for _ in screens()
                    if caster\is_local_player()
                        EventLog.add("You strike as you pass!", {200,200,255})
                @n_hits += 1
                caster\melee(mon, 1.0) -- math.max(0.1, 1.0 / @n_hits))
    draw_func: (caster) =>
        for i=1,math.min(@steps, 12)
            xy = {
                caster.x - @dir_vector[1] * i
                caster.y - @dir_vector[2] * i
            }
            screen_xy = Display.to_screen_xy(xy)
            caster.sprite\draw({color: {255,255,255, 200 - 30 * i}, origin: Display.CENTER}, screen_xy)
    stat_func: (caster, old, new) =>
        new.speed = 0
        --caster.stats.attack_cooldown = 2
        new.strength += caster.stats.level
        new.defence += caster.stats.level
        new.willpower += caster.stats.level
}

enemy_init = (enemy) -> nil

enemy_step = (enemy) -> nil

enemy_berserker_init = (enemy) ->
    enemy.berserk_time = 500

enemy_berserker_step = (enemy) ->
    enemy.berserk_time = enemy.berserk_time - 1
    if enemy.berserk_time <= 0 and Map.object_visible(enemy)
        enemy\add_effect("Berserk", 300)
        enemy.berserk_time = 500

DataW.effect_create {
    name: "Enraging"
    apply_func: (mon) =>
        @damage_tracker = 0
        @damage_interval = mon.stats.max_hp / 3
        @next_hp_threshold = mon.stats.max_hp - @damage_interval
    on_receive_damage_func: (attacker, mon, dmg) =>
        @damage_tracker += dmg
        new_hp = mon.stats.hp - dmg
        if new_hp < @next_hp_threshold
            for _ in screens()
                EventLog.add((if mon.unique then "" else "The ") .. mon.name .. " gets mad!", {255,255,255})
            mon\add_effect("Charging", 100)
            @next_hp_threshold -= @damage_interval
        return dmg
}

DataW.effect_create {
    name: "Charging"
    stat_func: (mon, old, new) =>
        new.speed *= if mon.name == "Gragh" then 2.5 else 4
        if @n_steps > @n_ramp
            new.melee_cooldown_multiplier *= if mon.name == "Gragh" then 0.75 else 0.5
        if mon.name ~= "Gragh"
            new.hpregen *= 2
    effected_sprite: "spr_effects.i-loudness"
    effected_colour: {255,0,0}
    fade_out: 15
    apply_func: (mon) =>
        @n_steps = 0
        @n_ramp = 10
    step_func: (mon) =>
        if @n_steps % 60 == 0
            if Map.object_visible(mon)
                play_sound "sound/wavy.ogg"
        @n_steps += 1
    on_melee_func: (mon, defender, damage) =>
        do return nil
        if @n_steps > @n_ramp
            thrown = defender\add_effect("Thrown", 10)
            thrown.angle = vector_direction({mon.x, mon.y}, {defender.x, defender.y})
            for _ in screens()
                if not mon.is_enemy and mon\is_local_player()
                    EventLog.add("The " .. defender.name .." is thrown back!", {200,200,255})
                elseif not defender.is_enemy and defender\is_local_player()
                    EventLog.add("You are thrown back!", {200,200,255})
            -- mon\remove_effect("Charging")
        return damage
}

DataW.effect_create {
    name: "DebugMonsterEvalEffect"
    init_func: () =>
        @defender_count = 0
        @attacker_count = 0
        @melee_count = 0
        @projectile_count = 0
        @receive_melee_count = 0
    on_defend_func: () =>
        @defender_count += 1
    on_attack_func: () =>
        @attacker_count += 1
    --stat_func: () =>
    --attack_stat_func: () =>
    --step_func: () ->

    --console_draw_func,
    --remove_func,
    --remove_derived_func,
    --apply_derived_func,
    --apply_buff_func,
    on_melee_func: () =>
        @melee_count += 1
    on_projectile_func: () =>
        @projectile_count += 1
    on_receive_melee_func: () =>
        @receive_melee_count += 1
    --on_equip_func,
    --on_identify_func,
    --on_uncurse_func,
    --on_gain_spell_func,
}

return M
