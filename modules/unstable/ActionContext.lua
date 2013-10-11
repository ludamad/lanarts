-- Convenient framework for dealing with actions with respect to the context they were used in.

local Actions = import ".Actions"

local M = nilprotect {} -- Submodule

M.DIRECT_ACTION = "DIRECT_ACTION"
M.PROJECTILE_ACTION = "PROJECTILE_ACTION"

function M.action_context_create(action, --[[Optional]] user, --[[Optional]] source, --[[Optional]] type)
    return { 
        base = action, derived = table.deep_clone(action), 
        user = user, 
        source = source, -- The item, race etc that defined this action.
        type = type or M.DIRECT_ACTION -- While we store 'source', it is not enough to infer eg if the action was caused by a projectile hit
    }
end

function M.on_action(user, action_context, target)
    user.derived.inventory:perform("on_action", user, action_context, target)
    user.derived.hooks:perform("on_action", user, action_context, target)
end

function M.can_use_action(action_context, target)
    return Actions.can_use_action(action_context.user, action_context.derived, target, action_context.source)
end

function M.use_action(action_context, target, --[[Optional]] ignore_death)
    return Actions.use_action(action_context.user, action_context.derived, target, action_context.source, ignore_death)
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
