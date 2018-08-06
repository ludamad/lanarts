-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

define_bonus {
    name: "Spiky"
    sprite: tosprite "spr_spells.spectral_weapon"
    effects_granted: {{"Spiky", {recoil_percentage: 0.10}}}
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}
