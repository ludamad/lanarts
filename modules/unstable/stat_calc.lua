local M = nilprotect {} -- Submodule

--- Resolves the attack stats for a given battle context
-- @param attacker the attacker context
-- @param defender the defender context
-- @return the final attack damage, delta resistance
--function 

--- Resolve derived stats after equipment and status hooks
-- @param stats the actor's equipment and other stats
-- @return the derived stats

local RESISTANCE_MULTIPLE_INTERVAL = 20

local function defensive_stats(attack, aptitudes)
    local resistance = table.dot_product(attack.resistance_types, aptitudes.resistance)
    local defence = table.dot_product(attack.defence_types, aptitudes.defence)
    return defence, resistance
end

local function offensive_stats(attack, aptitudes)
    local damage = table.dot_product(attack.damage_types, aptitudes.damage)
    local effectiveness = table.dot_product(attack.effectiveness_types, aptitudes.effectiveness)
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