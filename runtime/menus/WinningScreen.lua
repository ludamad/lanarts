local Display = require "core.Display"
local GameState = require "core.GameState"
local Keys = require "core.Keyboard"
local InstanceBox = require "ui.InstanceBox"
local InstanceLine = require "ui.InstanceLine"
local TextLabel = require "ui.TextLabel"
local Sprite = require "ui.Sprite"
local keys = require "core.Keyboard"

local winning_screen_font = "fonts/MateSC-Regular.ttf"

local function winning_screen_create()
    local box = InstanceBox.create( { size = Display.display_size} )
    local sprite = Sprite.image_create("menus/winning_screen.png")

    box:add_instance(
        sprite,
        Display.CENTER
    )

    if settings.regen_on_death then
        box:add_instance(
             TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_PALE_GREEN}, "You Have Won!"),
             Display.CENTER,
             {0, (-20 - sprite.size[2]/2) }
        )
    else
        box:add_instance(
             TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_PALE_RED}, "You Have Won On Hardcore!!"),
             Display.CENTER,
             {0, (-20 - sprite.size[2]/2) }
        )
        sprite.options.color = COL_RED
    end

    box:add_instance(
         TextLabel.create( font_cached_load(winning_screen_font, 20), {color=COL_LIGHT_GRAY}, "Thanks for playing."),
         Display.CENTER,
         {0, (20 + sprite.size[2]/2) }
    )

    box:add_instance(
         TextLabel.create( font_cached_load(settings.menu_font, 12), {color=COL_PALE_YELLOW}, "-ludamad"),
         Display.CENTER,
         {100, (45 + sprite.size[2]/2) }
    )

    local black_box_alpha = 1.0
    function box:draw(xy) --Makeshift inheritance
        InstanceBox.draw(self, xy)
        black_box_alpha = math.max(0, black_box_alpha - 0.05)
        Display.draw_rectangle(
            with_alpha(COL_BLACK, black_box_alpha), 
            bbox_create( {0,0}, Display.display_size )
        ) 
    end

    return box
end

local function winning_screen_show(...)
    local screen = winning_screen_create()

    while GameState.input_capture() and not Keys.key_pressed(keys.ESCAPE)  do
        Display.draw_start()
        screen:step( {0,0} )
        screen:draw( {0,0} )
        Display.draw_finish()
        GameState.wait(100)
    end
end

-- Submodule
return {
    show = winning_screen_show
}
