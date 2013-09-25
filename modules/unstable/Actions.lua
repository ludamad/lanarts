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

function M.can_use_action(user, action, target)
    for prereq in values(action.prerequisites) do
        if not prereq:check(user, target) then
            return false, action 
        end
    end
    return true
end

function M.use_action(user, action, target)
    assert(M.can_use_action(user, action, target))

    for effect in values(action.effects) do
        effect:apply(user, target)
    end
    return true
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
    for v in values(action.effects) do
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

-- Lookup the unique prerequisite of a given type.
-- Errors if multiple matching prerequisites exist!
function M.get_prerequisite(action, type)
    local prereqs = M.get_all_prerequisites(action, type)
    assert(#prereqs <= 1)
    return prereqs[1]
end

return M