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

-- Selectors are functions that uniquely determine effects.
-- Same-selector effects should be merged to guarantee uniqueness

function M.get_effect(action, selector)
    for v in values(action.effects) do
        if selector(v) then return v end
    end
    return nil
end

function M.get_prerequisite(action, selector)
    for v in values(action.prerequisites) do
        if selector(v) then return v end
    end
    return nil
end

return M