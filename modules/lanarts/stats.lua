local HookSet = import "@HookSet"

local M = {} -- submodule

-- Create an aptitude table with defaults, or copy over from another aptitude table
function M.aptitudes_create(--[[Optional]] params)
    params = params or {}
    return {
        attack_damage = params.attack_damage or 0,
        magic_damage = params.magic_damage or 0,

        attack_resistance = params.attack_resistance or 0,
        magic_resistance = params.magic_resistance or 0,

        melee = params.melee or 0,
        ranged = params.ranged or 0,
        spells = params.spells or 0,
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
        aptitudes = params.aptitudes or M.aptitudes_create {},
        hooks = HookSet.create()
    }
end

-- A pair of derived and base stats, with convenience methods
local DerivedStats = newtype()
function DerivedStats:init(--[[Optional]] stats)
    self.base = M.stats_create(stats)
    self.derived = M.stats_create(stats)
end

-- Prepare the stat set for deriving
function DerivedStats:copy_base_to_derived()
    table.deep_copy(self.derived, self.base)
end

-- Derive stats, without regard to a battle context
function DerivedStats:derive()
end

-- Derive stats, according to a battle context

DerivedStats.derived_stats_create = DerivedStats.create

-- Return submodule
return M
