TYPE_RESIST_EFFECT = {}
TYPE_POWER_EFFECT = {}
TYPES = {
    "Red"
    "Blue"
    "Green"
    "White"
    "Black"
    "Slashing"
    "Piercing"
    "Bludgeon"
}
for type in *TYPES
    TYPE_RESIST_EFFECT[type] = "#{type}Resist"
    TYPE_POWER_EFFECT[type] = "#{type}Power"

get_monster_resistances = (types) ->
    resists = {
        "Red": 0
        "Blue": 0
        "Green": 0
        "White": 0
        "Black": 0
        "Slashing": 0
        "Piercing": 0
        "Bludgeon": 0
    }
    for type in *types
        if type == "Red"
            --resists.Red += 5
            --resists.Green += 2
            --resists.Blue -= 1
            --resists.Black -= 1
            resists.Piercing += 5
        elseif type == "Black"
            --resists.Black += 5
            --resists.Green += 2
            --resists.White -= 1
            --resists.Slashing += 4
            --resists.Piercing += 4
            --resists.Bludgeon += 4
            resists.Slashing += 5
        elseif type == "White"
            --resists.White += 5
            --resists.Blue += 2
            --resists.Black -= 1
            --resists.Red -= 1
            resists.Bludgeon += 5
        elseif type == "Green"
            --resists.Bludgeon += 2
            --resists.Green += 5
            --resists.Blue += 2
            --resists.Red -= 1
            --resists.Black -= 1
            resists.Slashing += 5
        elseif type == "Blue"
            --resists.Slashing += 2
            --resists.Blue += 5
            --resists.Red += 4
            --resists.Green -= 1
            --resists.White -= 1
            resists.Piercing += 5
    return resists

get_effect_stat = (obj, type, default = 0) ->
    assert(obj and type and default)
    if obj\has_effect(type)
        return obj\get_effect(type)\value()
    return default

COLORS = {'White','Black','Green','Red','Blue'}

get_bonus = (obj, bonus) ->
    if not table.contains COLORS, bonus
        return 0
    eff = obj\get_effect(bonus)
    return if eff then eff.n_derived else 0

-- Get power of a certain type
get_power = (obj, type) ->
    return get_effect_stat(obj, TYPE_POWER_EFFECT[type]) + get_bonus(obj, "#{type}Bonus")

-- Returns a modifier, 0 signifying complete resistance, 1 signifying no resistance, and amounts > 1 signifying weakness
get_resistance = (obj, type) ->
    raw_resist = get_effect_stat(obj, TYPE_RESIST_EFFECT[type])
    -- Values over 100 or -100 signify extremes (extreme weakness and resistance)
    if raw_resist <= -100
        return 1.5
    if raw_resist >= 100
        return 0.5
    -- Otherwise, resistances are capped between -5 and +5 (inclusive)
    resist = math.max(math.min(math.floor(raw_resist), 5), -5)
    -- from 0.75 to 1.25x
    return 1 - resist / 10

-- Default implementation of projectile hit function, in Lua
hit_func = (type, attacker, target, atkstats) ->
    {:power, :damage, :magic_percentage} = atkstats
    power += get_power(attacker, type)
    type_multiplier = get_resistance(target, type)
    -- TODO take 'attacker' argument
    return target\damage(damage, power, magic_percentage, attacker, type_multiplier)

return {:get_effect_stat, :get_resistance, :get_power, :TYPES, :get_monster_resistances}
