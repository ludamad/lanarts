local Actions = import "@Actions"
local Attacks = import "@Attacks"
local ActionModUtils = import ".ActionModUtils"
local M = nilprotect {} -- Submodule

local ATTACK_EFF_PENALTY = 4
local ATTACK_DAM_PENALTY = 2

-- Creates a new attack, with an applied penalty
function M.apply_attack_modifier(action, fail_percent)
    local penalty_multiplier = math.min(1.0, fail_percent * 4)
    return ActionModUtils.add_attack_damage_and_effectiveness(
        action, --[[Clone]] true,
        -math.ceil(penalty_multiplier * ATTACK_EFF_PENALTY), 
        -math.ceil(penalty_multiplier * ATTACK_DAM_PENALTY)
    )
end

return M