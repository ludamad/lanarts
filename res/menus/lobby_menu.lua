require "InstanceGroup"
require "InstanceBox"
require "Sprite"
require "TextLabel"
require "utils"
require "menu_utils"

local text_button_params = {
    font = font_cached_load(settings.menu_font, 20),
    color = COL_WHITE,
    hover_color = COL_RED,
    click_box_padding = 5
}

local PLAYER_COLOURS = { 
    COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
    COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY 
}

local function lobby_joined_players_list_create()
    local font = font_cached_load(settings.menu_font, 20)
    local group = InstanceGroup.create()
    group.size = {200, 400} -- For placement algorithms

    group:add_instance( 
        TextLabel.create(font, {color=COL_WHITE, origin = CENTER_TOP}, "Players In Game:"), 
        {100, 30}
    )

    local lobby_list = { step = do_nothing }

    function lobby_list:draw(xy)
        local font = font_cached_load(settings.font, 10)
        local x, y = unpack(xy)
    
        for idx, player in ipairs(world.players) do
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

function lobby_menu_create(on_start_click)
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
        text_button_create("Start the Game!", on_start_click, text_button_params),
        CENTER_BOTTOM,
        --[[Up 40 pixels]]
        {0, -40}
    )

    return menu
end
