local M = nilprotect {} -- Submodule

-- TRAINABLE APTITUDES

-- Main combat proficiencies
M.melee = "melee"
M.ranged = "ranged"
M.magic = "magic"

-- Weapon types
M.piercing = "piercing"
M.slashing = "slashing"
M.blunt = "blunt"

-- Spell types
M.enchantments = "enchantments"
M.curses = "curses"
M.force = "force"

-- Misc proficiencies
M.armour = "armour" -- Ability to use armour
M.willpower = "willpower" -- Ability to resist spell effects
M.fortitude = "fortitude" -- Ability to resist physical abilities (stun, sleep etc)
M.self_mastery = "self_mastery" -- Ability to perform maneuvers
M.magic_devices = "magic_devices" -- Ability to use magic devices

-- Major 'elements'
M.dark = "dark"
M.light = "light"
M.fire = "fire"
M.water = "water"

-- Minor elements
M.earth = "earth"
M.air = "air"

-- NON-TRAINABLE APTITUDES
M.poison = "poison"

return M