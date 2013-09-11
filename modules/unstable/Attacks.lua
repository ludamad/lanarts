local ResourceTypes = import "@ResourceTypes"
local StatMultiplierUtils = import "@StatMultiplierUtils"

local M = nilprotect {} -- Submodule

local BASE_ATTACK_RATE = 45 -- frames, everything is compared to this rate

-- Create an attack with a single sub-attack
function M.attack_create(
    -- Base stats
    base_effectiveness, base_damage,
    -- Aptitude modifiers
    multipliers, 
    -- Weapon delay and related damage multiplier 
     --[[Optional]] delay, --[[Optional]] damage_multiplier)
    delay = delay or 1
    multipliers = StatMultiplierUtils.resolve_multiplier_set(multipliers)
    return {
        sub_attacks = {{
            base_effectiveness = base_effectiveness, 
            base_damage = base_damage,

            effectiveness_multipliers = multipliers[1],
            damage_multipliers = multipliers[2],
            resistance_multipliers = multipliers[3], 
            defence_multipliers = multipliers[4]
        }},

        cooldown = BASE_ATTACK_RATE * delay,
        damage_multiplier = damage_multiplier or delay
    }
end

function M.attack_add_effectiveness_and_damage(attack, eff, dam)
    for sub_attack in values(attack.sub_attacks) do
        sub_attack.base_effectiveness = sub_attack.base_effectiveness + eff
        sub_attack.base_damage = sub_attack.base_damage + dam
    end
    return attack
end

function M.attack_copy_and_add(attack, eff, dam)
    return M.attack_add_effectiveness_and_damage(table.deep_clone(attack), eff, dam)
end

function M.attack_context_create(obj, base, derived, attack)
    return {
        obj = obj,
        base = base,
        derived = derived,
        attack = attack
    }
end

M.ZERO_DAMAGE_ATTACK = M.attack_create(0,0,{dup({},4)})

return M
