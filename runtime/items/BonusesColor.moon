{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

-- +1/+1 COLOR BONUSES
define_color_bonus = (name, classes) ->
    define_bonus {
        :name
        sprite: tosprite("spr_bonuses.#{name\lower()}")
        console_draw_func: (inst, get_next) =>
            draw_console_effect get_next(), @sprite, {
                {COL_LIGHT_GRAY, "#{@name}: "}
                {COL_WHITE, "#{@name} classes get +1 Strength, +1 Magic."}
            }
            get_next()
        stat_func: (amount, obj, old, new) =>
            -- TODO dont hardcode class lists
            for klass in *classes
                if obj\has_effect(klass)
                    new.strength += amount
                    new.magic += amount
                    return
        item_draw_func: (options, x, y) =>
            @sprite\draw options, {x, y}
    }

define_color_bonus "Black", {"Necromancer"}
define_color_bonus "White", {"Stormcaller"}
define_color_bonus "Red", {"Pyrocaster"}
define_color_bonus "Green", {"Fighter"}
define_color_bonus "Blue", {}
