require "game_settings_menu"

require "InstanceBox"
require "Sprite"
require "TextLabel"
require "utils"
require "config"

-- START SCREEN -- 

local function text_button_create(text, on_click)
    local font = font_cached_load(settings.menu_font, 20)
    local label = TextLabel.create(font, { color=COL_WHITE }, text)
    local padding = 5

    function label:step(xy) -- Makeshift inheritance
        TextLabel.step(self, xy)

   		local bbox = bbox_padded( xy, self.size, padding )
    	self.options.color = bbox_mouse_over( bbox ) and COL_RED or COL_WHITE

    	if mouse_left_pressed and bbox_mouse_over( bbox ) then
    		on_click()
    	end
    end

    return label
end

local function start_menu_create(on_start_click, on_load_click)
    local menu = InstanceBox.create( { size = display.window_size } )

    menu:add_instance(
        Sprite.image_create("res/interface/sprites/lanarts_logo.png"),
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        text_button_create("Start a New Game", on_start_click),
        CENTER,
        --[[Down 100 pixels]]
        {0, 100}
    )
    if file_exists("savefile.save") then
        menu:add_instance(
            text_button_create("Continue Game", on_load_click),
            CENTER,
            --[[Down 150 pixels]]
            {0, 150}
        )
    end

    return menu
end

-- LOBBY SCREEN --

local PLAYER_COLOURS = { 
    COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
    COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY 
}

local function lobby_joined_players_list_create()
    local font = font_cached_load(settings.menu_font, 20)
    local group = InstanceGroup.create()
    group.size = {200, 400}

    group:add_instance( 
        TextLabel.create(font, {color=COL_WHITE, origin = CENTER_TOP}, "Players In Game:"), 
        {100, 30}
    )

    local lobby_list = { step = do_nothing }

    function lobby_list:draw(xy)
        local font = font_cached_load(settings.font, 10)
        local x, y = unpack(xy)
    
        for idx, player in ipairs(game.players) do
            local color_idx = ( (idx -1) % #PLAYER_COLOURS ) + 1
            local color = PLAYER_COLOURS[ color_idx ]

            local text = "Player " .. idx .. ": "
            if idx == 1 then -- Slot 1 always local player
                text = text .. "You, " 
            end
            text = text .. player.name .. " the " .. player.class_name

            font:draw( 
                { color = color, origin = CENTER_TOP }, 
                { x, y + (idx - 1) * 20 }, 
                text
            )
        end    
    end

    group:add_instance( lobby_list, {100, 70} )

    return group
end

local function lobby_menu_create(on_start_click)
    local menu = InstanceBox.create( { size = display.window_size } )
    local logo = Sprite.image_create("res/interface/sprites/lanarts_logo.png")

    menu:add_instance(
        logo, 
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        lobby_joined_players_list_create(), 
        CENTER_TOP,
        {0, 10 + logo.size[2]}
    )

    menu:add_instance(
        text_button_create("Start the Game!", on_start_click),
        CENTER_BOTTOM,
        --[[Up 40 pixels]]
        {0, -40}
    )

    return menu
end


-- MAIN --

DEBUG_LAYOUTS = false

local menu_state = { }

local exit_menu -- forward declare
local setup_start_menu -- forward declare
local setup_settings_menu -- forward declare
local setup_lobby_menu -- forward declare

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
        start_menu_create( --[[New Game Button]] setup_settings_menu, --[[Load Game Button]] on_load_click),
        CENTER
    )
end

function setup_settings_menu()
    menu_state.menu = InstanceBox.create( { size = display.window_size } )

    function menu_state.continue() 
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
