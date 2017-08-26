EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"
DataW = require "DataWrapped"
require "spells.DefineFireSpells"
require "spells.DefineWhiteSpells"

INFINITE_DURATION = 2^30 -- For all intents and purposes
mon_title = (mon) -> if mon.unique then mon.name else "the #{mon.name}"

-- MINOR MISSILE

-- Basic typeless mage spell
DataW.spell_create {
    name: "Minor Missile"
    spr_spell: "minor missile"
    description: "A low cost neutral bolt of energy. Hits a single target. The bolt can bounce off walls safely."
    projectile: {
        speed: 7.25
        damage_multiplier: 0.75
        can_wall_bounce: true
    }
    mp_cost: 4,
    cooldown: 35
}

-- MAGIC ARROW

DataW.spell_create {
    name: "Magic Arrow"
    description: "Allows you to create an arrow of pure energy, requires a bow. Does well against strudy opponents."
    spr_spell: "magic arrow"
    mp_cost: 20
    projectile: {
        speed: 7.25
        types: {"Piercing"}
        spr_attack: "crystal spear"
        range: 150
        speed: 9
        power: {base: {0, 0}, magic: 0.5, strength: 0.5}
    }
    cooldown: 30
    resist_modifier: 0.5
    prereq_func: () => 
        return @stats.weapon_type == "bows" 
}

-- FIRE BOLT

DataW.spell_create {
    name: "Fireball",
    spr_spell: "fire ball",
    description: "A great, strong bolt of fire. Hits a single target.",
    projectile: {
        types: {"Red"}
        speed: 5
        damage_multiplier: 2.00
    }
    mp_cost: 30,
    cooldown: 35
}

-- POISON CLOUD

DataW.spell_create {
    name: "Mephitize",
    spr_spell: "spr_spells.cloud",
    description: "A debilitating ring of clouds that cause damage as well as reduced defenses and speed over time.",
    projectile: {
        speed: 2
        cooldown: 105
        types: {"Green"}
        on_hit_func: (target, atkstats) =>
            target\add_effect("Poison", {
                poison_rate: 25
                time_left: 100,
                attacker: @caster
                damage: atkstats.damage,
                power: atkstats.power,
                magic_percentage: atkstats.magic_percentage,
                physical: atkstats.physical
            })
    }
    mp_cost: 20
    cooldown: 35
    spell_cooldown: 800
}

-- FEAR CLOUD

DataW.spell_create {
    name: "Trepidize"
    spr_spell: "spr_spells.cause_fear"
    description: "An insidious apparition that instills the fear of death in enemies it hits."
    projectile: {
        speed: 8
        on_hit_func: (target, atkstats) =>
            target\add_effect("Fear", 100) 
    }
    mp_cost: 0
    cooldown: 35
    spell_cooldown: 1600
}

DataW.spell_create {
    name: "Water Bolt",
    description: "A short range frosty bolt of cold. Hits a single target.",
    types: {"Blue"}
    spr_spell: "spr_effects.iceball",
    projectile: {
        speed: 5
        damage_multiplier: 1.35
    }
    mp_cost: 10,
    cooldown: 35
}
DataW.projectile_create {
    name: "Skullthrow"
    weapon_class: "magic"
    spr_spell: "spr_spells.skullthrow"
    types: {"Black"}
    range: 300
    damage_type: {magic: 0.5, physical: 0.5}
    speed: 7.25
    can_wall_bounce: true
    mp_cost: 4

    cooldown: 35
    on_hit_func: (target, atkstats) =>
        target\add_effect("Poison", {
            time_left: 100,
            damage: atkstats.damage,
            attacker: @caster, 
            power: atkstats.power,
            poison_rate: 25,
            magic_percentage: atkstats.magic_percentage,
            physical: atkstats.physical
        })
}

DataW.spell_create {
    name: "Ludaze"
    types: {"Black"}
    spr_spell: "spr_spells.ludaze"
    -- types: {"Dark"}
    damage_type: {magic: 0.5, physical: 0.5}
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
            if not Map.object_visible(mon, mon.xy, caster)
                continue
            mon\add_effect("Dazed", 100)
            target\add_effect("Poison", {
                time_left: 100,
                attacker: @caster, 
                damage: caster.stats.magic / 2
                power: caster.stats.magic * 0.3
                poison_rate: 25,
                magic_percentage: 1,
            })
        for _ in screens()  
            if caster\is_local_player()
                EventLog.add("You daze all enemies in sight!", {200,200,255})
            elseif caster.name == "Your ally"
                EventLog.add(caster.name .. " dazes all enemies in sight!", {200,200,255})
    autotarget_func: (caster) -> caster.x, caster.y
    prereq_func: (caster) -> 
        if not caster\has_effect("Berserk") and not caster\has_effect("Exhausted")  and not caster\has_effect("Ice Form")
            for mon in *(Map.enemies_list caster)
                if Map.object_visible(mon, mon.xy, caster)
                    return true
            --for _ in screens()
            EventLog.add("No monsters in sight!", COL_PALE_RED)
            return false
        return false
}


DataW.spell_create {
    name: "Ice Form"
    types: {"Blue"}
    spr_spell: "spr_spells.iceform"
    description: "Initiates Ice Form, a powerful ability for safe dungeoneering, preventing attacks and spells, and lowering speed drastically, but providing near immunity for 10 seconds." 
    mp_cost: 40
    cooldown: 100
    damage_type: {magic: 0.5, physical: 0.5}
    spell_cooldown: 1600
    can_cast_with_held_key: false
    fallback_to_melee: false
    action_func: (caster, x, y) ->
        caster\add_effect("Ice Form", 300)
        for _ in screens()
            if caster\is_local_player()
                EventLog.add("You enter a glacial state!", {200,200,255})
            elseif caster.name == "Your ally"
                EventLog.add(caster.name .. " enters a glacial state!", {200,200,255})
    autotarget_func: (caster) -> caster.x, caster.y
    prereq_func: (caster) -> 
        if not caster.can_rest
            --for _ in screens()
            EventLog.add("Ice Form requires perfect concentration!", {200,200,255})
            return false
        return not caster\has_effect("Berserk") and not caster\has_effect("Exhausted")  and not caster\has_effect("Ice Form")
}

DataW.spell_create {
    name: "Baleful Regeneration",
    types: {"Black"}
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
        for _ in screens()
            if caster\is_local_player()
                EventLog.add("You start to regenerate quickly!", {200,200,255})
                --play_sound "sound/Jingle_Win_Synth/Jingle_Win_Synth_00.ogg"
                play_sound "sound/berserk.ogg"
            else
                EventLog.add(caster.name .. " starts to regenerate quickly!", {200,200,255})
}

-- Requires kills from the necromancer, in the form of mana.
DataW.spell_create {
    name: "Summon Dark Aspect",
    types: {"Black"}
    spr_spell: "spr_spells.summon",
    description: "You summon a dark companion, at the cost of health and mana. The companion is stronger depending on the caster's willpower.",
    --description: "You summon a dark companion, at the cost of health and mana. The companion is stronger depending on the caster's willpower. Dies quickly outside of summoner view.",
    mp_cost: 10,
    cooldown: 45,
    can_cast_with_held_key: false,
    fallback_to_melee: false,
    spell_cooldown: 200
    autotarget_func: (caster) -> caster.x, caster.y
    prereq_func: (caster) ->
        if caster.stats.hp < 55
            if caster\is_local_player() 
                EventLog.add("You do not have enough health!", {200,200,255})
            return false
        if not caster\has_effect "Necromancer"
            if caster\is_local_player() 
                EventLog.add("You must be a necromancer to cast this spell!", {200,200,255})
            return false
        amount = math.max 1, math.ceil((caster\effective_stats().willpower - 4) / 2)
        {:n_summons} = caster\get_effect("Summoner")
        if n_summons >= amount
            if caster\is_local_player() 
                EventLog.add("You cannot currently control more than #{amount} aspects!", {200,200,255})
            return false
        return not caster\has_effect("Exhausted") and not (caster\has_effect "Summoning")
    action_func: (caster, x_unused, y_unused) ->
        play_sound "sound/summon.ogg"
        monster = "Spectral Beast"
        if not (caster\has_effect "Summoning")
            caster\direct_damage(35)
            eff = caster\add_effect("Summoning", 5)
            eff.on_summon = (obj) ->
                -- -- Make sure this monster cannot live outside its summoner's range for very long:
                --eff = obj\add_effect("DiesOutsideOfSummonerRange", INFINITE_DURATION)
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

-- Dash Attack
DataW.spell_create {
    name: "Dash Attack",
    types: {"Green"}
    spr_spell: "expedite",
    description: "Dash in a straight line, hitting all enemies in your path. Bounces if you hit a wall, doing increasing damage." -- Can still perform abilities while dashing.",
    --description: "You summon a dark companion, at the cost of health and mana. The companion is stronger depending on the caster's willpower. Dies quickly outside of summoner view.",
    mp_cost: 0
    cooldown: 30
    can_cast_with_held_key: false
    fallback_to_melee: true
    spell_cooldown: 10
    action_func: (x, y) =>
        effect = @add_effect "Dash Attack", {time_left: 20, angle: vector_direction(@xy, {x,y})}
        for _ in screens()
            if @is_local_player()
                EventLog.add("You dash valiantly forward!", {200,200,255})
    prereq_func: () =>
        return not @has_ranged_weapon()
    autotarget_func: () =>
        {dx, dy} = @last_moved_direction
        return @x + dx * 32, @y + dy * 32
}

-- Link of Loyalty
DataW.spell_create {
    name: "Link of Loyalty",
    types: {"Black"}
    spr_spell: "spr_spells.forgelink",
    description: "You summon a linked companion near an enemy, sending them to immediate combat but taking damage whenever they take damage. The type of companion depends on your amulet's summoning aspect.",
    mp_cost: 20
    cooldown: 0
    can_cast_with_held_key: false
    fallback_to_melee: false
    spell_cooldown: 40
    prereq_func: (caster) ->
        {:n_summons} = caster\get_effect("Summoner")
        if n_summons >= 2 -- caster.stats.level
            for _ in screens()
                if caster\is_local_player() 
                    EventLog.add("You cannot currently control more than #{2} aspects!", {200,200,255})
            return false
        return not caster\has_effect("Exhausted") and not (caster\has_effect "Summoning")
    action_func: (caster, x, y) ->
        play_sound "sound/lifelink.ogg"
        monster = "Cloud Elemental" -- TODO based on amulet
        if not (caster\has_effect "Summoning")
            eff = caster\add_effect("Summoning", 20)
            eff.summon_xy = {x, y}
            eff.on_summon = (obj) ->
                -- Buff this monster based on caster's willpower:
                --obj.stats.strength += math.floor(caster\effective_stats().willpower / 2)
                --obj.stats.magic += math.floor(caster\effective_stats().willpower / 2)
                --obj.stats.defence += math.floor(caster\effective_stats().willpower / 2)
                --obj.stats.willpower += math.floor(caster\effective_stats().willpower / 2)
                lifelink = obj\add_effect("Lifelink", INFINITE_DURATION)
                lifelink.linker = caster
                append caster\get_effect("Lifelinker").links, obj
            eff.monster = monster
            eff.duration = 5
}

-- Unlink
-- TODO AOE link all enemies?
DataW.spell_create {
    name: "Unlink",
    types: {"Black"}
    spr_spell: "spr_spells.unlink",
    description: "You break the link with all life-linked monsters. Monsters you have summoned are returned to their domain. Heals a small amount of health per monster."
    mp_cost: 0
    cooldown: 0
    can_cast_with_held_key: false
    fallback_to_melee: false
    spell_cooldown: 40
    prereq_func: (caster) ->
        {:n_summons} = caster\get_effect("Summoner")
        if n_summons == 0
            for _ in screens()
                if caster\is_local_player() 
                    EventLog.add("You have no lifelinked monsters!", {200,200,255})
            return false
        return not caster\has_effect("Exhausted") and not (caster\has_effect "Summoning")
    action_func: (caster, x, y) ->
        eff = caster\get_effect("Lifelinker")
        play_sound "sound/unlink.ogg"
        -- Unlink:
        links, eff.links = eff.links, {}
        -- Kill:
        for link in *links
            if link.destroyed
                continue
            link\direct_damage(link.stats.hp + 1)
            caster\heal_hp(25)

    autotarget_func: (caster) -> caster.x, caster.y
}


---- Link of Loyalty
--DataW.spell_create {
--    name: "Skull Trap",
--    spr_spell: "spr_effects.deathnote",
--    description: "You summon skull bombs.",
--    mp_cost: 20
--    cooldown: 0
--    can_cast_with_held_key: true
--    fallback_to_melee: true
--    spell_cooldown: 40
--    prereq_func: (caster) ->
--        return not caster\has_effect("Exhausted") and not (caster\has_effect "Summoning")
--    action_func: (caster, x, y) ->
--        play_sound "sound/lifelink.ogg"
--        {cx, cy} = caster.xy
--        points = {
--            {cx - 8, cy -8}
--            {cx + 8, cy -8}
--            {cx - 8, cy +8}
--            {cx + 8, cy +8}
--        }
--        obj = SpellObjects.SpellSpikes.create { points points, point_index = i, caster = caster, duration = 500}
--}

