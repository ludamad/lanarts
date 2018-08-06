{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

of_class = (obj, classes) ->
    for klass in *classes
        if obj\has_effect(klass)
            return true
    return false

-- +1/+1 COLOR BONUSES
define_color_bonus = (name, classes) ->
    define_bonus {
        :name
        sprite: tosprite("spr_bonuses.#{name\lower()}")
        console_draw_func: (obj, get_next) =>
            if of_class obj, classes
                draw_console_effect get_next(), @sprite, {
                    {COL_PALE_GRAY, "#{@name}: "}
                    {COL_WHITE, "#{@name} classes get +1 Strength, +1 Magic."}
                }
            else
                draw_console_effect get_next(), @sprite, {
                    {COL_GRAY, "#{@name}: "}
                    {COL_GRAY, "#{@name} classes get +1 Strength, +1 Magic."}
                }
            get_next()
        effect: {
            stat_func: (obj, old, new) =>
                -- TODO dont hardcode class lists
                if of_class obj, classes
                    new.strength += @n_derived
                    new.magic += @n_derived
        }
        item_draw_func: (options, x, y) =>
            @sprite\draw options, {x, y}
    }

define_color_bonus "Black", {"Necromancer"}
define_color_bonus "White", {"Stormcaller"}
define_color_bonus "Red", {"Pyrocaster"}
define_color_bonus "Green", {"Fighter"}
define_color_bonus "Blue", {}
