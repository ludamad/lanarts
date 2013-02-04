local POWER_MULTIPLE_INTERVAL = 50;

function damage_multiplier(power, resistance)
    local powdiff = power - resistance
    local intervals = powdiff / POWER_MULTIPLE_INTERVAL

    if intervals < 0 then
        --100% / (1+intervals)
        return 1.0 / (1.0 - intervals)
    else
        --100% + 100% * intervals
        return 1.0 + intervals
    end
end

function basic_damage_formula(attacker, defender)
    local mult = damage_multiplier(attacker.power, defender.resistance)
    local base = attacker.damage - defender.reduction * attacker.resist_modifier

    if (base < 0) then
        return 0
    end

    return mult * base
end


local function F(str)
    return loadstring('return function(x) return ' .. str .. ' end')()
end

function damage_simulate(hp, power, damage, reduction, resistance)
    hp = F(hp)
    power, damage = F(power), F(damage)
    reduction, resistance = F(reduction), F(resistance)
    return function(x) 
        local hp = hp(x)
        local attacker = { power = power(x), damage = damage(x), resist_modifier = 1.0}
        local defender = { reduction = reduction(x), resistance = resistance(x)}
    
        return hp / basic_damage_formula(attacker, defender)
    end
end

functions_to_plot = { 
    {
         damage_simulate( 'x * 40 + 20', 'x*2', 'x', 'x/2', 'x' ), 
         COL_RED 
    },
    {
        function() return 50 end,
        COL_BLACK
    }
}