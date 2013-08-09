local InstanceBox = import "core.ui.InstanceBox"
local InstanceLine = import "core.ui.InstanceLine"
local TextLabel = import "core.ui.TextLabel"
local Sprite = import "core.ui.Sprite"
local utils = import "core.utils"
local keys = import "core.keyboard"

local winning_screen_font = "modules/core/fonts/MateSC-Regular.ttf"

local function winning_screen_create()
    local box = InstanceBox.create( { size = Display.display_size} )
    local sprite = Sprite.image_create(path_resolve "winning_screen.png")

    box:add_instance(
        sprite,
        CENTER
    )

    if settings.regen_on_death then
        box:add_instance(
             TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_PALE_GREEN}, "You Have Won!"),
             CENTER,
             {0, (-20 - sprite.size[2]/2) }
        )
    else
        box:add_instance(
             TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_PALE_RED}, "You Have Won On Hardcore!!"),
             CENTER,
             {0, (-20 - sprite.size[2]/2) }
        )
        sprite.options.color = COL_RED
    end

    box:add_instance(
         TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_LIGHT_GRAY}, "Thanks for playing."),
         CENTER,
         {0, (20 + sprite.size[2]/2) }
    )

    box:add_instance(
         TextLabel.create( font_cached_load(settings.menu_font, 12), {color=COL_PALE_YELLOW}, "-ludamad"),
         CENTER,
         {100, (45 + sprite.size[2]/2) }
    )

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

local function winning_screen_show(...)
    local screen = winning_screen_create()

    while Game.input_capture() and not key_pressed(keys.ESCAPE)  do
        Display.draw_start()
        screen:step( {0,0} )
        screen:draw( {0,0} )
        Display.draw_finish()
        Game.wait(100)
    end
end

-- Submodule
return {
    show = winning_screen_show
}
