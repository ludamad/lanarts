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

SETTINGS_BOX_MAX_CHARS = 18
SETTINGS_BOX_SIZE = {180, 34}

TEXT_COLOR = {255, 250, 240}
CONFIG_MENU_SIZE = {640, 480}

SETTINGS_FONT = font_cached_load(settings.font, 10)
MID_SETTINGS_FONT = font_cached_load(settings.menu_font, 14)
BIG_SETTINGS_FONT = font_cached_load(settings.menu_font, 20)

settings_text_field_params = (params={}) ->
    params.size or= SETTINGS_BOX_SIZE
    params.font or= SETTINGS_FONT
    params.max_chars or= SETTINGS_BOX_MAX_CHARS
    return params

adventure_menu_start = () ->
   fields = InstanceBox.create {size:  {640, 480}}
   fields\add_instance(
       --Sprite.create(image_cached_load ),
       TextLabel.create(MID_SETTINGS_FONT, {}, "")
       Display.CENTER,
       {0,0}
   )


adventure_menu_config_screen = (on_back_click, on_start_click) ->


