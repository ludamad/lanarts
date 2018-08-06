{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

-- ITEM COST BONUSES
-- TODO NEVER PUT LUXURY IN STORES

define_bonus {
    name: "Luxury"
    appears_in_stores: false -- TODO use
    sprite: tosprite("spr_bonuses.luxury")
    console_draw_func: (inst, get_next) =>
        draw_console_effect get_next(), @sprite, {
            {COL_GOLD, "Luxury: "}
            -- TODO is this just like finding money?
            {COL_WHITE, "Sells for 3x."}
        }
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

-- STUN BONUS
-- TODO only put on weapons that go boof

define_bonus {
    name: "Knockback"
    sprite: tosprite "spr_effects.fleeing"
    effects_granted: {{"KnockbackWeapon", 1}}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 20, y}
}

-- STUN BONUS
-- TODO only put on weapons that go boof
