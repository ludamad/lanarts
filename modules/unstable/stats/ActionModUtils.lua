local Actions = import "@Actions"
local ActionContext = import "@ActionContext"
local ActionUtils = import ".ActionUtils"
local Attacks = import "@Attacks"
local ProjectileEffect = import ".ProjectileEffect"

local M = nilprotect {} -- Submodule

local function add_dam_and_eff(action, dam, eff)
    local attacks = ActionUtils.get_nested_attacks(action)
    assert(#attacks == 1, "Action must have exactly one AttackEffect.")
    local sub_attack = attacks[1].sub_attacks[1]
    sub_attack.base_effectiveness = sub_attack.base_effectiveness + dam
    sub_attack.base_damage = math.max(0, sub_attack.base_damage + eff)
end

function M.add_attack_damage_and_effectiveness(action, _, dam, eff)
    local context = ActionContext.action_context_create(action)
    add_dam_and_eff(context.derived, dam, eff)
    return context.derived
end
function M._add_attack_damage_and_effectiveness(action_context, dam, eff, --[[Optional]] permanent)
    add_dam_and_eff(action_context.derived, dam, eff)
    if permanent then
        add_dam_and_eff(action_context.base, dam, eff)
    end
end

local function add_subattack(action, sub_attack)
    local attacks = ActionUtils.get_nested_attacks(action)
    assert(#attacks == 1, "Action must have exactly one AttackEffect.")
    table.insert(attacks[1].sub_attacks, sub_attack)
end

function M.add_subattack(action_context, subattack, --[[Optional]] permanent)
    add_subattack(action_context.derived, subattack)
    if permanent then
        add_subattack(action_context.base, subattack)
    end
end

return M
