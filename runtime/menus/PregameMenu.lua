local Display = require "core.Display"

local InstanceGroup = require "ui.InstanceGroup"
local InstanceBox = require "ui.InstanceBox"
local Sprite = require "ui.Sprite"
local TextLabel = require "ui.TextLabel"
local World = require "core.World"

local text_button_params = {
    font = font_cached_load(settings.menu_font, 20),
    color = COL_WHITE,
    hover_color = COL_RED,
    click_box_padding = 5
}

local PLAYER_COLORS = { 
    COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
    COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY 
}

local function pregame_joined_players_list_create()
    local font = font_cached_load(settings.menu_font, 20)
    local group = InstanceGroup.create()
    group.size = {200, 400} -- For placement algorithms

    group:add_instance( 
        TextLabel.create(font, {color=COL_WHITE, origin = Display.CENTER_TOP}, "Players In Game:"), 
        {100, 30}
    )

    local pregame_list = { step = do_nothing }

    function pregame_list:draw(xy)
        local font = font_cached_load(settings.font, 10)
        local x, y = unpack(xy)
    
        for idx, player in ipairs(World.players) do
            local color_idx = ( (idx -1) % #PLAYER_COLORS ) + 1
            local color = PLAYER_COLORS[ color_idx ]

            local text = "Player " .. idx .. ": "
            if idx == 1 then -- Slot 1 always local player
                text = text .. "You, " 
            end
            text = text .. player.name .. " the " .. player.class_name

            font:draw( 
                { color = color, origin = Display.CENTER_TOP }, 
                { x, y + (idx - 1) * 20 }, 
                text
            )
        end    
    end

    group:add_instance( pregame_list, {100, 70} )

    return group
end

local function pregame_menu_create(on_start_click)
    local menu = InstanceBox.create( { size = Display.display_size } )
    local logo = Sprite.image_create("interface/sprites/lanarts_logo.png")

    menu:add_instance(
        logo, 
        Display.CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        pregame_joined_players_list_create(), 
        Display.CENTER_TOP,
        {0, 10 + logo.size[2]}
    )

    menu:add_instance(
        text_button_create("Start the Game!", on_start_click, text_button_params),
        Display.CENTER_BOTTOM,
        --[[Up 40 pixels]]
        {0, -40}
    )

    return menu
end

-- Submodule
return {
    create = pregame_menu_create
}
