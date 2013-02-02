require "utils"
require "InstanceBox"
require "TextInputBox"
require "TextLabel"
require "game_settings_menu"

DEBUG_LAYOUTS = false
WINDOW_SIZE = { 1200, 900 }

-- MAIN --

function main()
    display.initialize("Lanarts Example", WINDOW_SIZE, false)

    local frame = InstanceBox.create( { size = display.window_size } )
    frame:add_instance(
        game_settings_menu_create(
            function() print("BACK CLICKED") end, 
            function() print("START CLICKED") end), 
            CENTER)

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
