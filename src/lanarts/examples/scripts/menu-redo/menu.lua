require "utils"
require "InstanceOriginGroup"
require "TextInputBox"
require "TextLabel"
require "game_settings_menu"

DEBUG_LAYOUTS = false

-- MAIN --

function main()
    display.initialize("Lanarts Example", {640, 480}, false)

    local frame = InstanceOriginGroup.create( {640, 480} )
    frame:add_instance(game_settings_menu_create(), CENTER)

    while game.input_capture() do
        frame:step()

        display.draw_start()
        frame:draw()
        display.draw_finish()

        game.wait(10)
    end
end
