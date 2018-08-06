-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

-- Knockback: Stun bonus (description from effect)
define_bonus {
    name: "Knockback"
    sprite: tosprite "spr_effects.fleeing"
    effects_granted: {"KnockbackWeapon"}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 20, y}
}

-- Fortified: Grants fortifaction (description from effect)

define_bonus {
    name: "Fortified"
    sprite: tosprite "spr_spells.statue_form"
    effects_granted: {"Fortification"}
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Spiky"
    sprite: tosprite "spr_spells.spectral_weapon"
    effects_granted: {{"Fortification", {recoil_percentage: 0.10}}}
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

-- -- Append the effect to add, as well as the perceived 'tier' of this effect
-- -- Higher tier effects effect the final scoring of the randart quadratically.
-- Fortification: (level) => append @effects, {"Fortification", 1} -- Fixed value
-- Spiky: (level) =>
--     recoil_percentage = 0
--     for i =1,level
--         recoil_percentage += @rng\randomf(-0.05, 0.10)  -- Value proportional to recoil_percentage
--     recoil_percentage = math.max(0.05, recoil_percentage)
--     append @effects, {{"Spiky", {:recoil_percentage}}, recoil_percentage / 0.05}
-- PossiblySummonCentaurOnKill: (level) => append @effects, {"PossiblySummonCentaurOnKill", 2.5} -- Fixed value
-- PossiblySummonGolemOnKill: (level) => append @effects, {"PossiblySummonGolemOnKill", 2.5} -- Fixed value
-- PossiblySummonStormElementalOnKill: (level) => append @effects, {"PossiblySummonStormElementalOnKill", 2} -- Fixed value
-- VampiricWeapon: (level) => append @effects, {"VampiricWeapon", 1} -- Fixed value
-- ConfusingWeapon: (level) => append @effects, {"ConfusingWeapon", 1} -- Fixed value
-- KnockbackWeapon: (level) => append @effects, {"KnockbackWeapon", 1} -- Fixed value
-- PoisonedWeapon: (level) =>
--     poison_percentage = 0
--     for i =1,level
--         poison_percentage += @rng\randomf(-0.05, 0.10)  -- Value proportional to recoil_percentage
--     poison_percentage = math.max(0.05, poison_percentage)
--     append @effects, {{"PoisonedWeapon", {:poison_percentage}}, poison_percentage / 0.05}
