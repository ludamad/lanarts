-- Actions consist of a 'target_type', 'prerequisites' list, and 'effects' list.
-- They are the main building blocks of spells, items, etc

local M = nilprotect {} -- Submodule

M.TARGET_TYPES = {
-- Useful for monster AI and auto-targetting.
    "TARGET_NONE", -- NOTE: Sets up 'user' as 'target'!
-- Position targetting (eg projectile spells)
    "TARGET_HOSTILE_POSITION",
    "TARGET_FRIENDLY_POSITION",
-- Object targetting (eg melee attacks)
    "TARGET_HOSTILE",
    "TARGET_FRIENDLY"
}

-- Expose target types:
for v in values(M.TARGET_TYPES) do
    M[v] = v
end

function M.can_use_action(user, action, target, --[[Optional]] action_source)
    if action.on_prerequisite then
        local ok, problem = action.on_prerequisite(action_source, user, target)
        if not ok then return false, problem end 
    end
    for prereq in values(action.prerequisites) do
        local ok, problem = prereq:check(user, target)
        if not ok then return false, problem end 
    end
    return true
end

function M.use_action(user, action, target, --[[Optional]] action_source)
    assert(M.can_use_action(user, action, target, action_source))
    local ret
    if action.on_use then
        ret = action.on_use(action_source, user, target) and ret 
    end

    for effect in values(action.effects) do
        ret = effect:apply(user, target) and ret
    end
    return ret
end

-- Lookup all effects of a certain type.
function M.get_all_effects(action, type)
    local effects = {}
    for v in values(action.effects) do
        if getmetatable(v) == type then table.insert(effects,v) end
    end
    return effects
end

-- Lookup all prerequisites of a certain type.
function M.get_all_prerequisites(action, type)
    local prereqs = {}
    for v in values(action.prerequisites) do
        if getmetatable(v) == type then table.insert(prereqs,v) end
    end
    return prereqs
end

-- Lookup the unique effect of a given type.
-- Errors if multiple matching effects exist!
function M.get_effect(action, type)
    local effects = M.get_all_effects(action, type)
    assert(#effects <= 1)
    return effects[1]
end

-- Replace the unique effect of a given type.
-- Errors if multiple matching effects exist!
function M.set_effect(action, type, new_effect)
    local effects = M.get_all_effects(action, type)
    assert(#effects <= 1)
    for effect in values(effects) do 
        table.remove_occurrences(action.effects, effect)
    end
    table.insert(action.effects, new_effect)
end

-- Lookup the unique prerequisite of a given type.
-- Errors if multiple matching prerequisites exist!
function M.get_prerequisite(action, type)
    local prereqs = M.get_all_prerequisites(action, type)
    assert(#prereqs <= 1)
    return prereqs[1]
end

return M