require "game_settings_menu"

require "InstanceBox"
require "Sprite"
require "utils"

local TextButton = newtype()

function TextButton:init(text, action)
    self.font = font_cached_load(settings.menu_font, 20)

    self.size = self.font:draw_size(text)
    self.text = text

    self.action = action

    self.padding = 5
end

function TextButton:_click_area(xy) 
    return bbox_padded( xy, self.size, self.padding )
end

function TextButton:step(xy)
    if mouse_left_pressed and self.action ~= nil then
        if bbox_mouse_over( self:_click_area(xy)) then
            self.action()
        end
    end
end

function TextButton:draw(xy)
    local text_color = COL_WHITE

    if bbox_mouse_over( self:_click_area(xy) ) then 
        text_color = COL_RED
    end

    self.font:draw( {color = text_color }, xy, self.text )
end

-- MENUS -- 

local function start_menu_create(on_start_click)
    local menu = InstanceBox.create( { size = display.window_size } )

    menu:add_instance(
        Sprite.image_create("res/interface/sprites/lanarts_logo.png"),
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        TextButton.create("Start a New Game", on_start_click),
        CENTER
    )

    return menu
end


DEBUG_LAYOUTS = false
WINDOW_SIZE = { 1200, 700 }

-- MAIN --

local menu_state = { }
local setup_start_menu -- forward declare
local setup_settings_menu -- forward declare

function setup_start_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    menu_state.menu:add_instance(
        start_menu_create( --[[New Game Button]] setup_settings_menu ),
        CENTER
    )
end

function setup_settings_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )
    menu_state.menu:add_instance(
        game_settings_menu_create( --[[Back Button]] setup_start_menu,
        	 --[[Start Game Button]] function()
        		if settings.class_type ~= -1 then
                	menu_state.menu = nil -- Signals event loop that menu is finished
        		end
            end
        ), 
        CENTER
    )
end

function start_menu_show()
    display.initialize("Lanarts Example", WINDOW_SIZE, false)

    setup_start_menu()

    local timer = timer_create()
    while game.input_capture() and not key_pressed(keys.ESCAPE) do

        if key_pressed(keys.F9) then
            -- note, globals are usually protected against being changed
            -- but a bypass is allowed for cases where it must be done
            setglobal("DEBUG_LAYOUTS", not DEBUG_LAYOUTS) -- flip on/off
        end

        menu_state.menu:step( {0, 0} )

		if not menu_state.menu then -- because we have moved on 
			return true
		end

        display.draw_start()
        menu_state.menu:draw( {0, 0} )
        display.draw_finish()

        io.flush()
        game.wait(10)
    end

	return false
end