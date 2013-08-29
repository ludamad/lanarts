local Display = import "core.Display"

local InstanceBox = import "core.ui.InstanceBox"
local InstanceLine = import "core.ui.InstanceLine"
local TextLabel = import "core.ui.TextLabel"
local Sprite = import "core.ui.Sprite"
local utils = import "core.utils"

local keys = import "core.keyboard"

local death_screen_font = "modules/core/fonts/MateSC-Regular.ttf"

local function death_screen_create()
    local box = InstanceBox.create( { size = Display.display_size} )
    local sprite = Sprite.image_create(path_resolve "death_screen.png")

    box:add_instance(
        sprite,
        CENTER
    )

    box:add_instance(
         TextLabel.create( font_cached_load(death_screen_font, 20), {color=COL_PALE_RED}, "You Have Died!"),
         CENTER,
         {0, (-40 - sprite.size[2]/2) }
    )

    if settings.regen_on_death then
        box:add_instance(
             TextLabel.create( font_cached_load(death_screen_font, 20), {color=COL_LIGHT_GRAY}, "Press enter to respawn."),
             CENTER,
             {0, (20 + sprite.size[2]/2) }
        )
    else
        box:add_instance(
             TextLabel.create( font_cached_load(death_screen_font, 12), {color=COL_RED}, "Hardcore death is permanent!"),
             CENTER,
             {0, (-20 - sprite.size[2]/2) }
        )
        box:add_instance(
             TextLabel.create( font_cached_load(death_screen_font, 20), {color=COL_LIGHT_GRAY}, "Thanks for playing."),
             CENTER,
             {0, (20 + sprite.size[2]/2) }
        )
        box:add_instance(
             TextLabel.create( font_cached_load(settings.menu_font, 12), {color=COL_PALE_YELLOW}, "-ludamad"),
             CENTER,
             {100, (45 + sprite.size[2]/2) }
        )
    end

    local black_box_alpha = 1.0
    function box:draw(xy) --Makeshift inheritance
        InstanceBox.draw(self, xy)
        black_box_alpha = math.max(0, black_box_alpha - 0.05)
        draw_rectangle(
            with_alpha(COL_BLACK, black_box_alpha), 
            bbox_create( {0,0}, Display.display_size )
        ) 
    end

    return box
end

local function death_screen_show(...)
    local screen = death_screen_create(...)

    while Game.input_capture(true, false) and not key_pressed(keys.ESCAPE) and not key_pressed(keys.ENTER) and not key_pressed(keys.SPACE) do
        Display.draw_start()
        screen:step( {0,0} )
        screen:draw( {0,0} )
        Display.draw_finish()
        Game.wait(100)
    end
end

-- Submodule
return {
    show = death_screen_show
}
