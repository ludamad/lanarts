local M = nilprotect {} -- Submodule

local aptitudes = {
-- TRAINABLE APTITUDES
-- Main combat proficiencies
    "MELEE",
    "RANGED",
    "MAGIC",

-- Weapon types
    "PIERCING",
    "SLASHING",
    "BLUNT",

-- Spell types
    "ENCHANTMENTS",
    "CURSES",
    "FORCE",

-- Misc proficiencies
    "ARMOUR", -- Ability to use armour
    "WILLPOWER", -- Ability to resist spell effects
    "FORTITUDE", -- Ability to resist physical abilities (stun, sleep etc)
    "SELF_MASTERY", -- Ability to perform maneuvers
    "MAGIC_ITEMS", -- Ability to use magic items

-- Major 'elements'
    "DARK",
    "LIGHT",
    "FIRE",
    "WATER",

-- Minor elements
    "EARTH",
    "AIR",

-- NON-TRAINABLE APTITUDES
    "POISON",
-- ALTERNATE WEAPON CLASSES
    "BLADE",
    "AXE",
    "MACE",
    "POLEARM",
    "STAFF",
-- Misc proficiencies
    "WEAPON_IDENTIFICATION"
}

for type in values(aptitudes) do
    M[type] = type
end

return M