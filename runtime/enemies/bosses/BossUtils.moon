-- Shared by the bosses in enemies/bosses/
Map = require "core.Map"
GameState = require "core.GameState"
Display = require "core.Display"

-- An on_post_draw that shows the boss's name bobbing above it, height pixels above its centre
-- (default: just above its radius; tall sprites want more)
boss_name_drawer = (height = nil) -> () =>
    if Map.object_visible(@)
        color = with_alpha(COL_WHITE, 0.75)
        progress = math.abs(1 - (GameState.frame % 100) / 50)
        screen_xy = Display.to_screen_xy({@x, @y - (height or @radius + 16) - 8 * progress})
        bitmap_draw({:color, origin: Display.CENTER}, screen_xy, @name)

return {:boss_name_drawer, draw_boss_name: boss_name_drawer()}
