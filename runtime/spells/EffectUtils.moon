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
    TYPE_POWER_EFFECT[type] = "#{type}POWER"

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
            resists.Red += 3
            resists.Blue -= 1
            resists.Piercing += 1
        elseif type == "Black"
            resists.Black += 3
            resists.White -= 1
            resists.Slashing += 1
            resists.Piercing += 1
        elseif type == "White"
            resists.White += 3
            resists.Black -= 1
        elseif type == "Green"
            resists.Bludgeon += 1
            resists.Green += 3
            resists.Red -= 1
        elseif type == "Blue"
            resists.Slashing += 1
            resists.Green -= 1
            resists.Blue += 3
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
    -- Otherwise, resistances are capped between -3 and +3 (inclusive)
    resist = math.max(math.min(math.floor(raw_resist), 3), -2)
    return switch resist
        when 3 then 0.4
        when 2 then 0.6
        when 1 then 0.8
        when 0 then 1.0
        when -1 then 1.33
        when -2 then 1.66
        when -3 then 2
        else
            error("Out of range resist value #{resist}")

return {:get_effect_stat, :get_resistance, :get_power, :TYPES, :get_monster_resistances}
