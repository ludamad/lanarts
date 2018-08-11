GameObject = require "core.GameObject"
Display = require "core.Display"
{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'

ANIMATE_VELOCITIES = {
    {-4, -4}
    {0, -4}
    {4, -4}
    {4, 0}
    {4, 4}
    {0, 4}
    {-4, 4}
    {-4, 0}
}

COLOR_CLASSES = {
    Black: {"Necromancer"}
    White: {"Stormcaller"}
    Red: {"Pyrocaster"}
    Green: {"Fighter"}
    Blue: {}
}


create_animation = (obj, sprite) =>
    GameObject.animation_create {
        xy: obj.xy
        sprite: sprite
        duration: 25
        velocity: ANIMATE_VELOCITIES[(@n_animations % #ANIMATE_VELOCITIES) + 1]
    }
    -- To make sure animations dont sync, add random jitter:
    @n_animations += if chance 0.1 then 2 else 1

of_color = (obj, color) ->
    for klass in *COLOR_CLASSES[color]
        if obj\has_effect(klass)
            return true
    return false

of_colors = (obj, colors) ->
    for color in *colors
        if of_color obj, color
            return true
    return false

_draw_color_bound_effect = (obj, xy, colors, description) =>
    {x, y} = xy
    if of_colors obj, colors
        draw_console_effect {x, y - 8}, @sprite, {
            {COL_GREEN, "#{table.concat colors, " or "} class bonus"}
        }
        draw_console_text {x + 36, y + 8}, {
            {COL_WHITE, description}
        }
    else
        draw_console_effect {x, y - 8}, @sprite, {
            {COL_RED, "#{table.concat colors, " or "} class bonus"}
        }
        draw_console_text {x + 36, y + 8}, {
            {COL_GRAY, description}
        }

draw_color_bound_effect = (obj, get_next, colors, description) =>
    xy = get_next()
    _draw_color_bound_effect @, obj, xy, colors, description
    -- get_next()

draw_color_bound_weapon_effect = (obj, get_next, colors, description) =>
    xy = get_next()
    obj.weapon_sprite\draw {
        origin: Display.LEFT_CENTER
    }, {xy[1], xy[2] + 4}
    _draw_color_bound_effect @, obj, xy, colors, description
    --get_next()

draw_simple_effect  = (xy, name, description) =>
    {x, y} = xy
    draw_console_effect {x, y - 8}, @sprite, {
        {COL_GOLD, name}
    }
    draw_console_text {x + 36, y + 8}, {
        {COL_WHITE, description}
    }

return {
    :create_animation, :of_color, :of_colors, :draw_color_bound_effect
    :draw_color_bound_weapon_effect, :draw_simple_effect
}
