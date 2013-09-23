-- Actions consist of a 'target_type', 'prerequisites' list, and 'effects' list.
-- the 'Attacks' & 'StatRequirements' submodules define some building blocks for effects.

local M = nilprotect {} -- Submodule

-- Useful for monster AI and auto-targetting.
M.TARGET_NONE = "target_none" -- NOTE: Sets up 'user' as 'target'!

-- Position targetting (eg projectile spells)
M.TARGET_HOSTILE_POSITION = "target_hostile_position" 
M.TARGET_FRIENDLY_POSITION = "target_friendly_position"

-- Object targetting (eg melee attacks)
M.TARGET_HOSTILE = "target_hostile"
M.TARGET_FRIENDLY = "target_friendly"

M.TARGET_TYPES = {
    M.TARGET_NONE, M.TARGET_HOSTILE_POSITION, M.TARGET_FRIENDLY_POSITION,
    M.TARGET_HOSTILE, M.TARGET_FRIENDLY
}

function M.can_use_action(user, action, target)
    for prereq in values(action.prerequisites) do
        if not prereq:check(user, target) then
            return false
        end
    end
    return true
end

function M.use_action(user, action, target)
    assert(M.can_use_action(user, action, target))

    for effect in values(action.effects) do
        if not effect:apply(user, target) then
            return false
        end
    end
    return true
end

return M