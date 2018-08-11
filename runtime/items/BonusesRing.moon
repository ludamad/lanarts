{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:draw_simple_effect} = require "items.BonusesUtils"

-- ITEM COST BONUSES
-- TODO NEVER PUT LUXURY IN STORES

define_bonus {
    -- TODO is this just like finding money?
    name: "Luxury"
    appears_in_stores: false -- TODO use
    sprite: tosprite("spr_bonuses.luxury")
    console_draw_func: (obj, get_next) =>
        draw_simple_effect @, get_next(), "Luxury", "Sells for 3x."
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}
