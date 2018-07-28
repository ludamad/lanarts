-- Make an random piece of equipment
-- Equipment is created using various generation schemes, 
-- and then is created based on selection from score range.

DataWrapped = require "DataWrapped"
TYPES = require("spells.TypeEffectUtils").TYPES

_STAT_BONUSES = {
    enchantment: (level) =>
        for i=1,level
            @enchantment += @rng\random(-1, 3)
        @enchantment = math.max(@enchantment, 1)
    mp: (level) => 
        for i=1,level
            @stat_bonuses.mp += @rng\random(-25, 25)
    hp: (level) => 
        for i=1,level
            @stat_bonuses.hp += @rng\random(-25, 25)
    mpregen: (level) => 
        for i=1,level
            @stat_bonuses.mpregen += @rng\random(-300, 300) / 100 / 60
    hpregen: (level) => 
        for i=1,level
            @stat_bonuses.hpregen += @rng\random(-300, 300) / 100 / 60
    spell_velocity_multiplier: (level) => 
        for i=1,level
            @stat_bonuses.spell_velocity_multiplier += @rng\randomf(-0.15, 0.15)
}
for stat in *{"strength", "magic", "defence", "willpower"}
    _STAT_BONUSES[stat] = (level) => 
        for i=1,level
            @stat_bonuses[stat] += @rng\random(-3, 4)

_STAT_MULTIPLIER_BONUSES = {}
for stat in *{"magic_cooldown_multiplier" , "melee_cooldown_multiplier", "ranged_cooldown_multiplier"}
    _STAT_MULTIPLIER_BONUSES[stat] = (level) =>
        for i=1,level
            @stat_multipliers[stat] += @rng\randomf(-0.15, 0.15)

_EFFECT_BONUSES = {
    -- Append the effect to add, as well as the perceived 'tier' of this effect
    -- Higher tier effects effect the final scoring of the randart quadratically.
    Fortification: (level) => append @effects, {"Fortification", 1} -- Fixed value
    Spiky: (level) =>
        recoil_percentage = 0
        for i =1,level
            recoil_percentage += @rng\randomf(-0.05, 0.10)  -- Value proportional to recoil_percentage
        recoil_percentage = math.max(0.05, recoil_percentage)
        append @effects, {{"Spiky", {:recoil_percentage}}, recoil_percentage / 0.05}
    PossiblySummonCentaurOnKill: (level) => append @effects, {"PossiblySummonCentaurOnKill", 2.5} -- Fixed value
    PossiblySummonGolemOnKill: (level) => append @effects, {"PossiblySummonGolemOnKill", 2.5} -- Fixed value
    PossiblySummonStormElementalOnKill: (level) => append @effects, {"PossiblySummonStormElementalOnKill", 2} -- Fixed value
    VampiricWeapon: (level) => append @effects, {"VampiricWeapon", 1} -- Fixed value
    ConfusingWeapon: (level) => append @effects, {"ConfusingWeapon", 1} -- Fixed value
    KnockbackWeapon: (level) => append @effects, {"KnockbackWeapon", 1} -- Fixed value
    PoisonedWeapon: (level) =>
        poison_percentage = 0
        for i =1,level
            poison_percentage += @rng\randomf(-0.05, 0.10)  -- Value proportional to recoil_percentage
        poison_percentage = math.max(0.05, poison_percentage)
        append @effects, {{"PoisonedWeapon", {:poison_percentage}}, poison_percentage / 0.05}
    RandomSpell: (level) =>
        spell = @rng\random_choice {
            "Water Bolt"
            "Minor Missile"
            --"Mephitize",
            --"Trepidize",
            --"Regeneration",
            --"Berserk",
            --"Blink",
            --"Magic Arrow",
            --"Chain Lightning",
            --"Power Strike",
            --"Pain",
            --"Greater Pain",
            --"Fear Strike",
            --"Expedite",
            --"Wallanthor",
            --"Call Spikes",
            --"Luminos",
            "Healing Aura",
            --"Ice Form",
        }
        if @_added_bonuses[spell]
            return
        @_added_bonuses[spell] = true
        append @spells, {spell, 1}
}

for type in *TYPES
    _EFFECT_BONUSES["#{type}Resist"] = (level) =>
        if @rng\randomf() < 0.2
            @resistances[type] -= 1
        else
            @resistances[type] += 1
            if @rng\randomf() * level > 1.7
                @resistances[type] += 1

            for i=1,level
                @resistances[type] += @rng\random(-1, 2)
    _EFFECT_BONUSES["#{type}Power"] = (level) =>
        for i=1,level
            @powers[type] += @rng\random(-2,3)
        @powers[type] = math.max(@powers[type], 0)

_BONUSES = {}
for bonuses in *{_STAT_BONUSES, _STAT_MULTIPLIER_BONUSES, _EFFECT_BONUSES}
    for k,v in pairs bonuses
        _BONUSES[k] = v

Bonuses = newtype {
    init: (rng, level, base) =>
        @rng = assert rng
        @level = assert level
        @base = assert base
        -- Bonus state
        @stat_bonuses = {
            mp: 0
            hp: 0
            hpregen: 0
            mpregen: 0
            strength: 0
            defence: 0
            willpower: 0
            magic: 0
            spell_velocity_multiplier: 1.0
        }
        @stat_multipliers = {
            magic_cooldown_multiplier: 1.0
            melee_cooldown_multiplier: 1.0
            ranged_cooldown_multiplier: 1.0
        }
        @effects = {}
        @enchantment = 0 -- For weapons only for now
        @powers = {}
        @resistances = {}
        for type in *TYPES
            @powers[type] = 0
            @resistances[type] = 0
        @spells = {}
        -- Internally tracks added bonuses
        @_added_bonuses = {}
    -- Take a bonus context object from add_bonus
    add_bonus: (bonus, level = @level) =>
        if @_added_bonuses[bonus]
            return false
        _BONUSES[bonus](@, level)
        @_added_bonuses[bonus] = true
        return true

    -- Return whether this is a valid bonus object for this equipment score range
    validate: (min_score, max_score) =>
        -- Normalize into units of 'around one core stat point'
        dims = {
            @stat_bonuses.mp / 20
            @stat_bonuses.hp / 20
            @stat_bonuses.hpregen / (2.0 / 60)
            @stat_bonuses.mpregen / (2.0 / 60)
            @stat_bonuses.strength
            @stat_bonuses.defence
            @stat_bonuses.willpower
            @stat_bonuses.magic
            @enchantment * 2
            (1 - @stat_bonuses.spell_velocity_multiplier)  / 0.25
        }
        for k, v in pairs @stat_multipliers
            append dims, -(1 - v) / 0.05 -- For every 0.05 we take off, have one bonus dim
        n_resists = 0
        for k, v in pairs @resistances
            if v ~= 0 then n_resists += 1
            append dims, v
        n_powers = 0
        for k, v in pairs @powers
            if v ~= 0 then n_powers += 1
            append dims, v 
        for {effect, v} in *@effects
            append dims, v
        for {spell, v} in *@spells
            append dims, v
        effective_bonuses = 0
        penalties = 0
        score = 0
        for dim in *dims
            sign = 1
            if dim <= 0 -- Penalties subtract half as much as the corresponding buff
                --dim /= 2
                sign = -1
            if math.abs(dim) > 0.25
                effective_bonuses += 1
            if dim < 0
                penalties += 1
            score += sign * dim * dim * 100

        if n_resists + n_powers > 2 -- reject
            return false
        if penalties > 1 -- reject
            return false
        if score < min_score or score > max_score
            return false
        if effective_bonuses < 2 -- reject
            return false
        -- reject if not within the right score:
        return true, score

    create_definer_object: (name, description) =>
        -- Ensure all tables present
        base = table.deep_clone(@base)
        base.name = name
        base.description = description
        -- Handle @stat_bonuses
        base.stat_bonuses or= {}
        for k, v in pairs @stat_bonuses 
            base.stat_bonuses[k] or= 0
            base.stat_bonuses[k] += v
        -- Handle @stat_multipliers
        for k, v in pairs @stat_multipliers 
            base[k] or= 1
            base[k] *= v
        -- TODO add base types to weapon?
        -- Handle @effects
        base.effects_granted or= {}
        for {effect,v} in *@effects
            append base.effects_granted, effect
        -- Handle @powers and @resistances
        DataWrapped.power_effects(@powers, base.effects_granted)
        DataWrapped.resistance_effects(@resistances, base.effects_granted)
        -- Handle @spells
        base.spells_granted or= {}
        for {spell,v} in *@spells
            append base.spells_granted, spell
        if @enchantment > 0
            for i=1,2
                base.damage.base[i] = math.ceil(base.damage.base[i] * (1 + @enchantment * 0.2))
            base.name = "+#{@enchantment} #{base.name}"
        return base
}

return {:Bonuses}
