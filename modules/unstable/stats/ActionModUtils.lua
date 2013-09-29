local Actions = import "@Actions"
local ActionUtils = import ".ActionUtils"
local Attacks = import "@Attacks"
local ProjectileEffect = import ".ProjectileEffect"

local M = nilprotect {} -- Submodule

function M.add_attack_damage_and_effectiveness(action, clone, dam, eff)
    action = clone and table.deep_clone(action) or action
    local attacks = ActionUtils.get_nested_attacks(action)
    assert(#attacks == 1, "Action must have exactly one AttackEffect.")
    for sub_attack in values(attacks[1].sub_attacks) do
        sub_attack.base_effectiveness = sub_attack.base_effectiveness + dam
        sub_attack.base_damage = math.max(0, sub_attack.base_damage + eff)
    end
    return action
end

return M