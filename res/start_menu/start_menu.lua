require "game_settings_menu"
require "scores_menu"

require "InstanceBox"
require "Sprite"
require "TextLabel"
require "utils"
require "config"
require "menu_utils"

-- START SCREEN -- 

local text_button_params = {
    font = font_cached_load(settings.menu_font, 20),
    color = COL_WHITE,
    hover_color = COL_RED,
    click_box_padding = 5
}

local function start_menu_create(on_start_click, on_load_click, on_score_click)
    local menu = InstanceBox.create( { size = display.window_size } )

    menu:add_instance(
        Sprite.image_create("res/interface/sprites/lanarts_logo.png"),
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    local y_position = 100 -- Start down 100 pixels
    menu:add_instance(
        text_button_create("Start a New Game", on_start_click, text_button_params),
        CENTER,
        {0, y_position}
    )
    y_position = y_position + 50

    if file_exists("savefile.save") then
        menu:add_instance(
            text_button_create("Continue Game", on_load_click, text_button_params),
            CENTER,
            {0, y_position}
        )
        y_position = y_position + 50
    end

    menu:add_instance(
        text_button_create("Highscores", on_score_click, text_button_params),
        CENTER,
        {0, y_position}
    )
    y_position = y_position + 50


    function menu:step(xy) -- Makeshift inheritance
       InstanceBox.step(self, xy)
       if key_pressed('N') then
            on_start_click()
       elseif key_pressed('S') then
            on_score_click()
       end 
    end

    return menu
end


-- MAIN --

DEBUG_LAYOUTS = false

local menu_state = { }

local exit_menu -- forward declare
local setup_start_menu -- forward declare
local setup_settings_menu -- forward declare
local setup_lobby_menu -- forward declare
local setup_scores_menu -- forward declare

function exit_menu()
    menu_state.menu = nil -- Signals event loop that menu is finished
    menu_state.continue = nil
end

function setup_start_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    
    local function on_load_click()
        config.startup_function = function()
            if file_exists("savefile.save") then
                game.load("savefile.save")
            end
        end
        exit_menu()
    end

    if file_exists("savefile.save") then
        menu_state.continue = on_load_click
    else 
        menu_state.continue = setup_settings_menu
    end

    menu_state.menu:add_instance(
        start_menu_create( --[[New Game Button]] setup_settings_menu, --[[Load Game Button]] on_load_click, --[[Highscores Button]] setup_scores_menu),
        CENTER
    )
end

function setup_settings_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )

    menu_state.continue = function ()
        if settings.class_type ~= -1 then
            exit_menu()
        end
    end

    menu_state.menu:add_instance(
        game_settings_menu_create( --[[Back Button]] setup_start_menu, --[[Start Game Button]] menu_state.continue), 
        CENTER
    )
end

function setup_lobby_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    menu_state.continue = exit_menu
    menu_state.menu:add_instance(
        lobby_menu_create( --[[Start Game Button]] menu_state.continue ),
        CENTER
    )
end

function setup_scores_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    menu_state.continue = setup_start_menu
    menu_state.menu:add_instance(
        scores_menu_create( --[[Back Button]] setup_start_menu ),
        CENTER
    )
end

local function menu_loop(should_poll)
    while game.input_capture() and not key_pressed(keys.ESCAPE) do
        if key_pressed(keys.F9) then
            -- note, globals are usually protected against being changed
            -- but a bypass is allowed for cases where it must be done
            setglobal("DEBUG_LAYOUTS", not DEBUG_LAYOUTS) -- flip on/off
        end

        if should_poll then
            net.connections_poll()
        end

        menu_state.menu:step( {0, 0} )

        if key_pressed(keys.ENTER) and menu_state.continue then
            menu_state:continue()
        end

        if not menu_state.menu then -- because we have moved on 
            return true
        end

        display.draw_start()
        menu_state.menu:draw( {0, 0} )
        display.draw_finish()

        io.flush()
        game.wait(10)
    end

    return false -- User has quit the game
end

function start_menu_show()
    setup_start_menu()

	return menu_loop(--[[Do not poll connections]] false)
end


function lobby_menu_show()
    setup_lobby_menu()

    return menu_loop(--[[Poll connections]] true)
end
