local M = nilprotect {} -- Submodule

-- TRAINABLE APTITUDES

local aptitudes = {
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
    "MAGIC_DEVICES", -- Ability to use magic devices

-- Major 'elements'
    "DARK",
    "LIGHT",
    "FIRE",
    "WATER",

-- Minor elements
    "EARTH",
    "AIR",

-- NON-TRAINABLE APTITUDES
    "POISON"
}

for type in values(aptitudes) do
    M[type] = type
end

return M