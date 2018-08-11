{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:of_color, :draw_color_bound_effect} = require "items.BonusesUtils"

-- +1/+1 COLOR BONUSES
define_color_bonus = (name) ->
    define_bonus {
        :name
        sprite: tosprite("spr_bonuses.#{name\lower()}")
        console_draw_func: (obj, get_next) =>
            draw_color_bound_effect @, obj, get_next, {name},
                "+1 Strength, +1 Magic."
        effect: {
            stat_func: (obj, old, new) =>
                if of_color obj, name
                    new.strength += @n_derived
                    new.magic += @n_derived
        }
        item_draw_func: (options, x, y) =>
            @sprite\draw options, {x, y}
    }

define_color_bonus "Black"
define_color_bonus "White"
define_color_bonus "Red"
define_color_bonus "Green"
define_color_bonus "Blue"
