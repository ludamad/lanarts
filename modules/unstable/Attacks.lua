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

local function resolve_multiplier(arg)
    if type(arg) == "string" then return {[arg] = 1} end
    if #arg > 0 then
        local ret = {}
        for trait in values(arg) do
            ret[trait] = 1
        end
        return ret
    end

    return arg
end

local function resolve_multiplier_table(args)
    -- This is already a multiplier-table
    if type(arg) == "table" and type(arg[1]) == "table" then
        assert(#arg == 4, "'attack_create' expects a set of 4 attack multipliers")
        local copy = {}
        for val in values(args) do
            table.insert(copy, resolve_multiplier(val))
        end
        return copy
    end
    -- Else
    return {dup(resolve_multiplier(args), 4)}
end

-- Create an attack with a single sub-attack
function M.attack_create(
    -- Base stats
    base_effectiveness, base_damage,
    -- Aptitude modifiers
    multipliers, 
    -- Weapon speed and related damage multiplier 
     --[[Optional]] speed, --[[Optional]] defence_modifier)
    speed = speed or 1
    multipliers = resolve_multiplier_table(multipliers)
    return {
        sub_attacks = {{
            base_effectiveness = base_effectiveness, 
            base_damage = base_damage,

            effectiveness_multipliers = multipliers[1],
            damage_multipliers = multipliers[2],
            resistance_multipliers = multipliers[3], 
            defence_multipliers = multipliers[4]
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

M.ZERO_DAMAGE_ATTACK = M.attack_create(0,0,{dup({},4)})

return M
