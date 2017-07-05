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
            resists.Red += 5
            resists.Green += 2
            resists.Blue -= 2
            resists.Black -= 1
            resists.Piercing += 2
        elseif type == "Black"
            resists.Black += 5
            resists.Green += 2
            resists.White -= 2
            resists.Slashing += 2
            resists.Piercing += 2
        elseif type == "White"
            resists.White += 5
            resists.Blue += 2
            resists.Black -= 2
            resists.Red -= 1
        elseif type == "Green"
            resists.Bludgeon += 2
            resists.Green += 5
            resists.Blue += 2
            resists.Red -= 2
            resists.Black -= 1
        elseif type == "Blue"
            resists.Slashing += 2
            resists.Blue += 5
            resists.Red += 2
            resists.Green -= 2
            resists.White -= 1
    return resists

get_effect_stat = (obj, type, default = 0) ->
    assert(obj and type and default)
    if obj\has_effect(type)
        return obj\get_effect(type)\_get_value()
    return default

-- Get power of a certain type
get_power = (obj, type) ->
    return get_effect_stat(obj, TYPE_POWER_EFFECT[type])

-- Returns a modifier, 0 signifying complete resistance, 1 signifying no resistance, and amounts > 1 signifying weakness
get_resistance = (obj, type) ->
    raw_resist = get_effect_stat(obj, TYPE_RESIST_EFFECT[type])
    -- Values over 100 or -100 signify extremes (extreme weakness and complete resistance)
    if raw_resist <= -100
        return 10
    if raw_resist >= 100
        return 0
    -- Otherwise, resistances are capped between -5 and +5 (inclusive)
    resist = math.max(math.min(math.floor(raw_resist), 5), -5)
    return math.pow(2, -(2*resist) / 5) -- from 0.25 to 4x

-- Default implementation of projectile hit function, in Lua
hit_func = (type, attacker, target, atkstats) ->
    {:power, :damage, :magic_percentage} = atkstats
    power += get_power(attacker, type)
    damage *= get_resistance(target, type)
    -- TODO take 'attacker' argument
    return target\damage(damage, power, magic_percentage, attacker)

--        when 3 then 0.4
--        when 2 then 0.6
--        when 1 then 0.8
--        when 0 then 1.0
--        when -1 then 1.33
--        when -2 then 1.66
--        when -3 then 2
--        else
--            error("Out of range resist value #{resist}")
--
return {:get_effect_stat, :get_resistance, :get_power, :TYPES, :get_monster_resistances}
