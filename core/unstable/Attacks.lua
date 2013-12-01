local ResourceTypes = import "@ResourceTypes"
local StatMultiplierUtils = import "@StatMultiplierUtils"
local CooldownTypes = import "@stats.CooldownTypes"
local AttackResolution = import "@AttackResolution"
local LogUtils = import "lanarts.LogUtils"
local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

M.AttackEffect = newtype()

function M.AttackEffect:apply(attacker, target)
    assert(target.base and target.derived, "Stat context expected as target parameter (probably wrong target type)")
    local dmg = AttackResolution.damage_calc(self, attacker, target)
    StatContext.add_hp(target, -random_round(dmg))
    LogUtils.event_log_resolved(attacker.obj, "{The }$You deal{s} " ..dmg .. " damage!", COL_GREEN)
    LogUtils.event_log_resolved(target.obj, "{The }$You [have]{has} " .. math.ceil(target.base.hp) .. "HP left.", COL_PALE_BLUE)
    return dmg
end

-- Attacks follow the 'action' interface (on_use, optional on_prerequisite)
-- Create an attack with a single sub-attack
function M.AttackEffect:init(
        -- Base stats
        base_effectiveness, base_damage,
        -- Aptitude modifiers
        aptitude_multipliers,
        -- Weapon delay and related damage multiplier 
         --[[Optional]] delay, --[[Optional]] damage_multiplier
    )
    delay = delay or 1
    local m = StatMultiplierUtils.resolve_multiplier_set(aptitude_multipliers)
    self.sub_attacks = {{
        base_effectiveness = base_effectiveness, 
        base_damage = base_damage,

        effectiveness_multipliers = m[1], damage_multipliers = m[2],
        resistance_multipliers = m[3], defence_multipliers = m[4]
    }}

    self.damage_multiplier = damage_multiplier or delay
end

M.attack_create = M.AttackEffect.create

function M.attack_add_effectiveness_and_damage(attack, eff, dam)
    for sub_attack in values(attack.sub_attacks) do
        sub_attack.base_effectiveness = sub_attack.base_effectiveness + eff
        sub_attack.base_damage = sub_attack.base_damage + dam
    end
    return attack
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