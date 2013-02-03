require "InstanceBox"
require "Sprite"

local SETTINGS_FONT = font_cached_load(settings.font, 10)
local BIG_SETTINGS_FONT = font_cached_load(settings.menu_font, 20)

local PLAYER_COLOURS = { 
    COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
    COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY 
}

local function lobby_menu_create(text, on_click)
    local menu = InstanceBox.create( { size = display.window_size } )

    menu:add_instance(
        Sprite.image_create("res/interface/sprites/lanarts_logo.png"),
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        text_button_create("Start a New Game", on_start_click),
        CENTER
    )

    return menu
end
