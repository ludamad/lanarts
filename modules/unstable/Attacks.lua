local ResourceTypes = import "@ResourceTypes"

local M = nilprotect {} -- Submodule

local BASE_ATTACK_RATE = 45 -- frames, everything is compared to this rate

function M.to_mult_table(...)
    local ret = {}
    for i=1,select("#", ...),2 do
    	local trait = select(i, ...)
    	local value = select(i+1, ...)
    	ret[trait] = value
    end
    return ret
end

local function resolve_arg(arg)
    if type(arg) == "string" then return {[arg] = 1} end
    if #arg > 0 then
        local ret = {}
        for trait in values(arg) do
            ret[arg] = 1
        end
        return ret
    end
    return arg
end

-- Create an attack with a single sub-attack
function M.attack_create(
    -- Base stats
    base_effectiveness, base_damage,
    -- Aptitude modifiers 
    effectiveness_multipliers, damage_multipliers, resistance_multipliers, defence_multipliers,
    -- Weapon speed and related damage multiplier 
     --[[Optional]] speed, --[[Optional]] defence_modifier)
    speed = speed or 1
    return {
        sub_attacks = {{
            base_effectiveness = base_effectiveness, 
            base_damage = base_damage,

            effectiveness_multipliers = resolve_arg(effectiveness_multipliers), 
            damage_multipliers = resolve_arg(damage_multipliers),
            resistance_multipliers = resolve_arg(resistance_multipliers), 
            defence_multipliers = resolve_arg(defence_multipliers)
        }},

        cooldown = BASE_ATTACK_RATE * speed,
        defence_modifier = defence_modifier or speed
    }
end

function M.attack_context_create(obj, base, derived, attack)
    return {
        obj = obj,
        base = base,
        derived = derived,
        attack = attack
    }
end

return M