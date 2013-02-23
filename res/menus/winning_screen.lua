require "InstanceBox"
require "InstanceLine"
require "TextLabel"
require "Sprite"
require "utils"

local winning_screen_font = "res/fonts/MateSC-Regular.ttf"

local function winning_screen_create()
    local box = InstanceBox.create( { size = display.window_size} )
    local sprite = Sprite.image_create("res/menus/winning_screen.png")

    box:add_instance(
        sprite,
        CENTER
    )

    box:add_instance(
         TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_PALE_GREEN}, "You Have Won!"),
         CENTER,
         {0, (-20 - sprite.size[2]/2) }
    )

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
            bbox_create( {0,0}, display.window_size )
        ) 
    end

    return box
end

function winning_screen_show(...)
    local screen = winning_screen_create()

    while game.input_capture() and not key_pressed(keys.ESCAPE)  do
        display.draw_start()
        screen:step( {0,0} )
        screen:draw( {0,0} )
        display.draw_finish()
        game.wait(100)
    end

    return false
end