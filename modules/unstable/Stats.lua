local HookSet = import "@HookSet"
local Cooldowns = import "@Cooldowns"
local Inventory = import "@Inventory"
local SpellsKnown = import "@SpellsKnown"

local M = {} -- Submodule

-- Create a skill table with the given values. Defaults are used for anything not provided.
function M.skills_create(--[[Optional]] params)
    local ret = {}
    if params then table.copy(params, ret) end
    return ret
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
        name = params.name,
        team = params.team,

        level = params.level or 1,

    	-- The 'core stats', hp & mp. The rest of the stats are defined in terms of aptitudes.
        hp = params.hp or params.max_hp,
        max_hp = params.max_hp or params.hp,
        hp_regen = params.hp_regen or 0,

        mp = params.mp or params.max_mp or 0,
        max_mp = params.max_mp or params.mp or 0,
        mp_regen = params.mp_regen or 0,

        inventory = params.inventory or Inventory.create(),
        aptitudes = M.aptitudes_create(params.aptitudes),
        skills = M.skills_create(params.skills),
        hooks = params.hooks or HookSet.create(),

        spells = params.spells or SpellsKnown.create(),
        abilities = params.abilities or {},

        -- Simple dictionary of all 'infinite resistances'. Applies whenever resistance applies.
        immunities = params.immunities or {},

        cooldowns = params.cooldowns or Cooldowns.create(),

        movement_speed = params.movement_speed or 2
    }
end

return M