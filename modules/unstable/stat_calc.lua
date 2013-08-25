local M = nilprotect {} -- Submodule

--- Resolves the attack stats for a given battle context
-- @param attacker the attacker context
-- @param defender the defender context
-- @return the final attack damage, delta resistance
--function 

--- Resolve derived stats after equipment and status hooks
-- @param stats the actor's equipment and other stats
-- @return the derived stats

--- Calculates the sum t1[k] * t2[k] for every key 'k' that exists in t1 and t2  
local function table_dot_product(t1, t2)
    local sum = 0
    for k,v1 in pairs(t1) do
        local v2 = t2[k]
        if v2 ~= nil then
            sum = sum + (v1 * v2)
        end
    end
    return sum
end

local RESISTANCE_MULTIPLE_INTERVAL = 20

local function defensive_stats(attack, aptitudes)
    local resistance = table_dot_product(attack.resistance_types, aptitudes.resistance)
    local defence = table_dot_product(attack.defence_types, aptitudes.defence)
    return defence, resistance
end

local function offensive_stats(attack, aptitudes)
    local damage = table_dot_product(attack.damage_types, aptitudes.damage)
    local effectiveness = table_dot_product(attack.effectiveness_types, aptitudes.effectiveness)
    return damage, effectiveness
end

--- Determine the damage done by an attack
-- @param attacker the attacker StatContext
-- @param target the target StatContext
function M.damage_calc(attack, attacker, target)
    local damage, effectiveness = offensive_stats(attack, target.derived.aptitudes)
    local defence, resistance = defensive_stats(attack, attacker.derived.aptitudes)

    return (damage - defence) * (1.0 + (effectiveness - resistance) / RESISTANCE_MULTIPLE_INTERVAL)
end

return M