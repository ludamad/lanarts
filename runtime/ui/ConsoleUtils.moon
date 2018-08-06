Map = require "core.Map"
Display = require "core.Display"

draw_console_text = (xy, texts) ->
    {x, y} = xy
    for {color, text} in *texts
        x += font_cached_load(settings.font, 10)\draw {
            :color
            origin: Display.LEFT_CENTER
        }, {x, y}, text 
    return nil

draw_console_effect = (xy, sprite, texts) ->
    {x, y} = xy
    sprite\draw {
        origin: Display.LEFT_CENTER
    }, {x, y + 4}
    draw_console_text {x + Map.TILE_SIZE + 4, y}, texts

return {:draw_console_text, :draw_console_effect}
