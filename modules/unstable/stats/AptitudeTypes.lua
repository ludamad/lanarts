local M = nilprotect {} -- Submodule

-- TRAINABLE APTITUDES

-- Main combat proficiencies
M.MELEE = "MELEE"
M.RANGED = "RANGED"
M.MAGIC = "MAGIC"

-- Weapon types
M.PIERCING = "PIERCING"
M.SLASHING = "SLASHING"
M.BLUNT = "BLUNT"

-- Spell types
M.ENCHANTMENTS = "ENCHANTMENTS"
M.CURSES = "CURSES"
M.FORCE = "FORCE"

-- Misc proficiencies
M.ARMOUR = "ARMOUR" -- Ability to use armour
M.WILLPOWER = "WILLPOWER" -- Ability to resist spell effects
M.FORTITUDE = "FORTITUDE" -- Ability to resist physical abilities (stun, sleep etc)
M.SELF_MASTERY = "SELF_MASTERY" -- Ability to perform maneuvers
M.MAGIC_DEVICES = "MAGIC_DEVICES" -- Ability to use magic devices

-- Major 'elements'
M.DARK = "DARK"
M.LIGHT = "LIGHT"
M.FIRE = "FIRE"
M.WATER = "WATER"

-- Minor elements
M.EARTH = "EARTH"
M.AIR = "AIR"

-- NON-TRAINABLE APTITUDES
M.POISON = "POISON"

return M