local M = nilprotect {} -- Submodule

local function make_allowable(eff,dam,res,def)
    return {effectiveness=eff, damage=dam, resistance=res, defence=def}
end

local ALL = make_allowable(dup(true,4))
local EFFECTIVENESS = make_allowable(true, dup(false,3))
local RESISTANCE_AND_EFFECTIVENESS = make_allowable(true, false, true, false)

M.allowed_aptitudes = {
-- TRAINABLE APTITUDES
-- Main combat proficiencies
    MELEE = ALL,
    RANGED = ALL,
    MAGIC = ALL,

-- Weapon types
    PIERCING = ALL,
    SLASHING = ALL,
    BLUNT = ALL,

-- Spell types
    ENCHANTMENTS = ALL,
    CURSES = ALL,
    FORCE = ALL,
    SUMMONING = EFFECTIVENESS,

-- Misc proficiencies
    ARMOUR = EFFECTIVENESS, -- Ability to use armour
    WILLPOWER = EFFECTIVENESS, -- Ability to resist spell effects
    FORTITUDE = EFFECTIVENESS, -- Ability to resist physical abilities (stun, sleep etc)
    SELF_MASTERY = EFFECTIVENESS, -- Ability to perform maneuvers
    MAGIC_ITEMS = EFFECTIVENESS, -- Ability to use magic items

-- Major 'elements'
    DARK = ALL,
    LIGHT = ALL,
    FIRE = ALL,
    WATER = ALL,

-- Minor elements
    EARTH = ALL,
    AIR = ALL,

-- NON-TRAINABLE APTITUDES
    CHAOS = RESISTANCE_AND_EFFECTIVENESS,
    POISON = RESISTANCE_AND_EFFECTIVENESS,
-- Alternate weapon categories
    BLADE = ALL,
    AXE = ALL,
    MACE = ALL,
    BOWS = ALL,
    POLEARM = ALL,
    STAFF = ALL,
-- Misc proficiencies
    WEAPON_IDENTIFICATION = EFFECTIVENESS,
    WEAPON_PROFICIENCY = EFFECTIVENESS,

    MAGIC_SPEED = EFFECTIVENESS,
    MELEE_SPEED = EFFECTIVENESS,
    RANGED_SPEED = EFFECTIVENESS,


    MOVEMENT_SPEED = EFFECTIVENESS
}

for type, allowable in pairs(M.allowed_aptitudes) do
    M[type] = type
end

return M