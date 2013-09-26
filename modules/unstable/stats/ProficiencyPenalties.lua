local Actions = import "@Actions"
local Attacks = import "@Attacks"
local M = nilprotect {} -- Submodule

local ATTACK_EFF_PENALTY = 4
local ATTACK_DAM_PENALTY = 2

-- Creates a new attack, with an applied penalty
function M.apply_attack_modifier(action, fail_percent)
    local penalty_multiplier = math.min(1.0, fail_percent * 4)
    local new_action = table.deep_clone(action)
    local new_atk = Actions.get_effect(new_action, Attacks.AttackEffect)
    for sub_attack in values(new_atk.sub_attacks) do
        sub_attack.base_effectiveness = sub_attack.base_effectiveness - math.ceil(penalty_multiplier * ATTACK_EFF_PENALTY)
        sub_attack.base_damage = math.max(0, sub_attack.base_damage - math.ceil(penalty_multiplier * ATTACK_DAM_PENALTY))
    end
    return new_action
end

return M