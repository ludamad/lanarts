local ResourceTypes = import "@ResourceTypes"
local StatMultiplierUtils = import "@StatMultiplierUtils"
local CooldownTypes = import "@stats.CooldownTypes"

local M = nilprotect {} -- Submodule

-- Create an attack with a single sub-attack
function M.attack_create(
    -- Base stats
    base_effectiveness, base_damage,
    -- Aptitude modifiers
    aptitude_multipliers, action_cooldown,
    -- Weapon delay and related damage multiplier 
     --[[Optional]] delay, --[[Optional]] damage_multiplier)
    delay = delay or 1
    local m = StatMultiplierUtils.resolve_multiplier_set(aptitude_multipliers)
    return {
        sub_attacks = {{
            base_effectiveness = base_effectiveness, 
            base_damage = base_damage,

            effectiveness_multipliers = m[1], damage_multipliers = m[2],
            resistance_multipliers = m[3], defence_multipliers = m[4]
        }},

        action_cooldown = action_cooldown,
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
