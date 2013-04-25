require "utils"
require "InstanceBox"
require "TextInputBox"
require "TextLabel"

require "start_menu"
require "game_settings_menu"

DEBUG_LAYOUTS = false
WINDOW_SIZE = { 1200, 700 }

-- MAIN --

local menu_state = { }
local setup_start_menu -- forward declare
local setup_settings_menu -- forward declare

function setup_start_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    menu_state.menu:add_instance(
        start_menu_create( --[[Start Button]] setup_settings_menu ),
        CENTER
    )
end

function setup_settings_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    menu_state.menu:add_instance(
        game_settings_menu_create( --[[Back Button]] setup_start_menu,
            function() 
                print("START CLICKED") 
            end
        ), 
        CENTER
    )
end

function main()
    display.initialize("Lanarts Example", WINDOW_SIZE, false)

    setup_start_menu()

    local timer = timer_create()
    while game.input_capture() do

        if key_pressed(keys.SPACE) then
            -- note, globals are usually protected against being changed
            -- but a bypass is allowed for cases where it must be done
            setglobal("DEBUG_LAYOUTS", not DEBUG_LAYOUTS) -- flip on/off
        end

        menu_state.menu:step( {0, 0} )

        display.draw_start()
        menu_state.menu:draw( {0, 0} )
        display.draw_finish()

        io.flush()
        game.wait(10)
    end
end
