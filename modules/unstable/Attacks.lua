local ResourceTypes = import "@ResourceTypes"
local StatMultiplierUtils = import "@StatMultiplierUtils"
local CooldownTypes = import "@stats.CooldownTypes"
local AttackResolution = import "@AttackResolution"
local LogUtils = import "lanarts.LogUtils"
local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

local function attack_on_use(self, attacker, target)
    local dmg = AttackResolution.damage_calc(self, attacker, target)

    StatContext.add_hp(target, -random_round(dmg))
    return dmg
end

M.CANNOT_REACH_MESSAGE = "You cannot reach!"

-- Not part of on_prereq to allow flexibly specifying target position
function M.xy_in_range(attack, attacker, target_radius, target_xy)
    local reach = attack.range + attacker.obj.radius + target_radius
    if reach < vector_distance(attacker.obj.xy, target_xy) then
        return false, M.CANNOT_REACH_MESSAGE
    end
    return true
end

function M.obj_in_range(attack, attacker, target)
    return M.xy_in_range(attack, attacker, target.obj.radius, target.obj.xy)
end

local DEFAULT_MELEE_RANGE = 10

-- Attacks follow the 'action' interface (on_use, optional on_prerequisite)
-- Create an attack with a single sub-attack
function M.attack_create(
    -- Base stats
    base_effectiveness, base_damage,
    -- Aptitude modifiers
    aptitude_multipliers,
    -- Weapon delay and related damage multiplier 
     --[[Optional]] delay, --[[Optional]] damage_multiplier,
     --[[Optional]] range)
    delay = delay or 1
    local m = StatMultiplierUtils.resolve_multiplier_set(aptitude_multipliers)
    return {
        sub_attacks = {{
            base_effectiveness = base_effectiveness, 
            base_damage = base_damage,

            effectiveness_multipliers = m[1], damage_multipliers = m[2],
            resistance_multipliers = m[3], defence_multipliers = m[4]
        }},

        damage_multiplier = damage_multiplier or delay,
        on_use = attack_on_use,
        range = range or DEFAULT_MELEE_RANGE
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
