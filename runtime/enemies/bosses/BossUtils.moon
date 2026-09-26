-- Shared by the bosses in enemies/bosses/
Map = require "core.Map"
GameState = require "core.GameState"
Display = require "core.Display"

-- on_post_draw: the boss's name, bobbing above it
draw_boss_name = () =>
    if Map.object_visible(@)
        color = with_alpha(COL_WHITE, 0.75)
        progress = math.abs(1 - (GameState.frame % 100) / 50)
        screen_xy = Display.to_screen_xy({@x, @y - @radius - 16 - 8 * progress})
        bitmap_draw({:color, origin: Display.CENTER}, screen_xy, @name)

return {:draw_boss_name}
