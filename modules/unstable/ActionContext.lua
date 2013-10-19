-- Convenient framework for dealing with actions with respect to the context they were used in.

local StatContext = import "@StatContext"
local Actions = import ".Actions"
local ItemTraits = import "@items.ItemTraits"

local M = nilprotect {} -- Submodule

M.DIRECT_ACTION = "DIRECT_ACTION"
M.PROJECTILE_ACTION = "PROJECTILE_ACTION"

-- Create an empty action context
function M.action_context_create(action, --[[Optional]] user, --[[Optional]] source, --[[Optional]] type)
    return { 
        base = action, derived = table.deep_clone(action), 
        user = user, 
        source = source, -- The item, race etc that defined this action.
        type = type or M.DIRECT_ACTION -- While we store 'source', it is not enough to infer eg if the action was caused by a projectile hit
    }
end

function M.action_context_copy_base_to_derived(action_context)
    Actions.copy_action(action_context.base, action_context.derived)
end

function M.on_action(action_context, target)
    StatContext.on_event("on_action", action_context.user, action_context, target)
end

function M.can_use_action(action_context, target)
    return Actions.can_use_action(action_context.user, action_context.derived, target, action_context.source)
end

function M.use_action(action_context, target, --[[Optional]] ignore_death)
    local res = Actions.use_action(action_context.user, action_context.derived, target, action_context.source, ignore_death)
    if Actions.is_target_stat_context(target) then
        StatContext.on_event("on_hostile_action", target, action_context)
    end
    return res
end

function M.spell_context(action_context, spell)
    local weapon = weapon or StatContext.get_equipped_item(user, ItemTraits.WEAPON)
    local action, source = self.unarmed_action, self.race or self -- Default
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(self._context, weapon)
        source = weapon
        action = ProficiencyPenalties.apply_attack_modifier(weapon.action_wield, modifier)
    end
    return ActionContext.action_context_create(action, self:stat_context(), source)
end

function M.weapon_context(user, --[[Optional]] weapon)
    local obj = user.obj
    weapon = weapon or StatContext.get_equipped_item(user, ItemTraits.WEAPON)
    if not weapon then return obj:unarmed_action_context() end
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(self._context, weapon)
        source = weapon
        action = ProficiencyPenalties.apply_attack_modifier(weapon.action_wield, modifier)
    end
    return ActionContext.action_context_create(action, self:stat_context(), source)
end

-- Lookup all effects of a certain type.
function M.get_all_effects(action_context, type) 
    return Actions.get_all_effects(action_context.derived, type)
end
function M.get_all_prerequisites(action_context, type) 
    return Actions.get_all_prerequisites(action_context.derived, type)
end
function M.get_effect(action_context, type) 
    return Actions.get_effect(action_context.derived, type) 
end
function M.reset_effect(action_context, type, --[[Optional]] new_effect) 
    return Actions.reset_effect(action_context.derived, type, new_effect)
end
function M.reset_prerequisite(action_context, type, --[[Optional]] new_prereq) 
    return Actions.reset_prerequisite(action_context.derived, type, new_prereq)
end
function M.get_prerequisite(action_context, type)
    return Actions.get_prerequisite(action_context.derived, type) 
end

return M