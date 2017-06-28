EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
DataW = require "DataWrapped"
ObjectUtils = require "objects.ObjectUtils"
SpellUtils = require "spells.SpellUtils"
EffectUtils = require "spells.EffectUtils"

M = nilprotect {
}

-- ENERGY SPEAR
-- A basic elemental spell that goes through multiple enemies.
DataW.spell_create {
    name: "Energy Spear",
    description: "Allows you to create a durable spear of lightning.",
    spr_spell: "magic arrow"
    types: {"White"}
    projectile: SpellUtils.passing_projectile {
        speed: 5
        spr_attack: "crystal spear"
        damage_multiplier: 1
        n_steps: 500 / 8 
        can_wall_bounce: true
        redamage_cooldown: 40 -- Cooldown for when enemies are damaged again by effect
    }
    mp_cost: 10,
    cooldown: 35
}

-- LIGHTNING BLAST
--    spr_spell: "spr_spells.spell_icon_lightning_bolt",

return M
