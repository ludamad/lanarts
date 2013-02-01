require "utils"
require "InstanceOriginGroup"
require "TextInputBox"
require "TextLabel"
require "game_settings_menu"

DEBUG_LAYOUTS = false

-- MAIN --

function main()
    display.initialize("Lanarts Example", {640, 480}, false)

    local frame = InstanceOriginGroup.create( { size = {640, 480} } )
    frame:add_instance(game_settings_menu_create(), CENTER)

    local timer = timer_create()
    while game.input_capture() do

        if key_pressed(keys.SPACE) then
            setglobal("DEBUG_LAYOUTS", not DEBUG_LAYOUTS)
        end

        frame:step( {0, 0} )

        display.draw_start()
        frame:draw( {0, 0} )
        display.draw_finish()

        io.flush()
        game.wait(10)
    end
end
