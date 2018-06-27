InstanceGroup = require "ui.InstanceGroup"
InstanceLine = require "ui.InstanceLine"
InstanceBox = require "ui.InstanceBox"
TextInputBox = require "ui.TextInputBox"
TextLabel = require "ui.TextLabel"
Sprite = require "ui.Sprite"

Network = require "core.Network"
Keys = require "core.Keyboard"
Mouse = require "core.Mouse"
Display = require "core.Display"

{:label_button_create} = require "menus.GameSettingsMenu"

SETTINGS_BOX_MAX_CHARS = 18
SETTINGS_BOX_SIZE = {180, 34}

TEXT_COLOR = {255, 250, 240}
CONFIG_MENU_SIZE = {640, 480}

SETTINGS_FONT = font_cached_load(settings.font, 10)
MID_SETTINGS_FONT = font_cached_load(settings.menu_font, 14)
BIG_SETTINGS_FONT = font_cached_load(settings.menu_font, 20)

class_choice_buttons_create = () ->
    sprite_base = "spr_menu/"
    x_padding, y_padding = 48, 16
    font = MID_SETTINGS_FONT

    buttons = {
        { "Adventure Mode", sprite_base .. "dragon-head.png", COL_WHITE},
        { "Arena Mode", sprite_base .. "airtight-hatch.png", COL_PALE_RED},
    }
    prev, next = {}, {}
    for i, button in ipairs(buttons)
        name = button[1]
        prev_i = i <= 1 and #buttons or i - 1
        next_i = i >= #buttons and 1 or i + 1

        prev[name] = buttons[prev_i][1]
        next[name] = buttons[next_i][1]

    button_size = { 96, 96 + y_padding + font.height }
    button_row = InstanceLine.create( { dx: button_size[1] + x_padding } )

    button_row.step = (xy) =>
        InstanceLine.step(self, xy)

        -- Allow choosing a class by using left/right arrows or tab
        if Keys.key_pressed(Keys.LEFT)
            settings.class_type = prev[settings.class_type] or buttons[#buttons][1]
        elseif Keys.key_pressed(Keys.RIGHT) or Keys.key_pressed(Keys.TAB)
            settings.class_type = next[settings.class_type] or buttons[1][1]

    for button in *buttons
        sprite = {size: button_size, font: font, text: button[1], sprite: image_cached_load(button[2])}
        color_formula = (xy) =>
            if settings.class_type == button[1]
                return COL_GOLD
            else
                return @mouse_over(xy) and COL_PALE_YELLOW or COL_WHITE--button[3]
        on_click = () =>
            settings.class_type = button[1]
        label = label_button_create sprite, color_formula, on_click
        button_row\add_instance label

    return button_row

game_settings_menu_create = (on_back_click, on_start_click) ->
    fields = InstanceBox.create {size: { 640, 480 }}
    fields\add_instance class_choice_buttons_create(), Display.CENTER, {0, 0}
    return fields

-- submodule
return {
    create: game_settings_menu_create
}

