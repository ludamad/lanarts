local HookSet = import "@HookSet"

local M = {} -- Submodule

-- Create a skill table with the given values. Defaults are used for anything not provided.
function M.skills_create(--[[Optional]] params)
    params = params or {}
    return {
        ranged = params.ranged or 0,
        melee = params.melee or 0
    }
end

-- Create an aptitude table with the given values. Defaults are used for anything not provided.
function M.aptitudes_create(--[[Optional]] params)
    params = params or {}
    return {
        -- Each table is associated with a trait
        effectiveness = params.effectiveness or {},
        resistance = params.resistance or {},
        damage = params.damage or {},
        defence = params.defence or {}
    }
end

-- Create stats with defaults, or copy over from other stats
function M.stats_create(--[[Optional]] params)
    params = params or {}
    return {
        hp = params.hp or params.max_hp or 0,
        max_hp = params.max_hp or params.hp or 0,
        mp = params.mp or params.max_mp or 0,
        max_mp = params.max_mp or params.mp or 0,
        inventory = params.inventory or {},
        aptitudes = M.aptitudes_create(params.aptitudes),
        skills = M.skills_create(params.skills),
        hooks = params.hooks or HookSet.create()
    }
end

return M