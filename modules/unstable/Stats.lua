local HookSet = import "@HookSet"
local CooldownSet = import "@CooldownSet"
local Inventory = import "@Inventory"
local SpellsKnown = import "@SpellsKnown"
local Attacks = import "@Attacks"
local SkillType = import "@SkillType"

local M = {} -- Submodule

-- Create a skill table with the given values. Defaults are used for anything not provided.
function M.skills_create(--[[Optional]] params, --[[Optional]] add_skills)
    local ret = {}
    if params then table.copy(params, ret) end
    if add_skills then
        for skill in values(SkillType.list) do
            local has_already = false
            for s in values(ret) do
                if s.type == skill then
                    has_already = true
                    break
                end 
            end
            if not has_already then
                table.insert(ret, skill:on_create())
            end
        end
    end
    return ret
end

-- Create an aptitude table with the given values. Defaults are used for anything not provided.
function M.aptitudes_create(--[[Optional]] params)
    params = params and table.deep_clone(params) or {}
    return {
        -- Each table is associated with a trait
        effectiveness = params.effectiveness or {},
        resistance = params.resistance or {},
        damage = params.damage or {},
        defence = params.defence or {}
    }
end

local function clone_if_exists(v)
    return v and table.deep_clone(v) or nil
end

-- Create stats with defaults, or copy over from other stats
function M.stats_create(--[[Optional]] params, --[[Optional]] add_skills)
    local C = clone_if_exists
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

        inventory = C(params.inventory) or Inventory.create(),
        aptitudes = M.aptitudes_create(params.aptitudes),
        skills = M.skills_create(params.skills, add_skills),
        hooks = C(params.hooks) or HookSet.create(),

        spells = C(params.spells) or SpellsKnown.create(),
        abilities = C(params.abilities) or {},
        unarmed_attack = params.unarmed_attack or Attacks.ZERO_DAMAGE_ATTACK,

        cooldowns = C(params.cooldowns) or CooldownSet.create(),

        movement_speed = params.movement_speed or 2
    }
end

function M.get_skill(stats, skill_type)
    for skill_slot in values(stats.skills) do
        if skill_slot.type == skill_type then
            return skill_slot
        end
    end
    assert(false)
end

return M