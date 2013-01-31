require "utils"
require "InstanceGroup"
require "InstanceGridGroup"
require "TextInputBox"

local SETTINGS_BOX_MAXCHARS = 18
local SETTINGS_BOX_SIZE = {180, 34}

local CONFIG_MENU_SIZE = {640, 480}

local SETTINGS_FONT = font_cached_load("res/sample.ttf", 10)

local function text_field_create(label_text, default_text, callbacks)

    local field = InstanceGroup.create()

    -- Add text label
    field:add_instance(
        TextLabel.create(
            SETTINGS_FONT, -- TextLabel font
            {color = COL_YELLOW }, 
            label_text
        ),
        {0, -20} -- position
    )

    -- Add text input box
    field:add_instance(
        TextInputBox.create( 
            SETTINGS_FONT, -- TextInputBox font
            SETTINGS_BOX_SIZE, -- Text input box size
            {SETTINGS_BOX_MAXCHARS, default_text}, -- input box parameters
            callbacks
        ),
       {0, 0} -- position
    )

    return field
end

local function name_field_create()
    return text_field_create(
            "Enter your name:",
            "User",
            { -- Field validating & updating 
                update = function(field) -- Update username based on contents
                    settings.username = field.text
                end
            }
    )
end

local function is_valid_ip_string(text)
    if text == "localhost" then return true end

    local parts = string_split(text, ".")

    -- Valid IP string has 4 components, eg 1.2.3.4
    if #parts ~= 4 then return false end

    -- Assert all components are numbers <= 255
    for part in values(parts) do 
        local number = tonumber(part)
        if number == nil then return false end
        if number < 0 or number > 255 then return false end
    end

    return true
end

local function host_IP_field_create()
    return text_field_create(
            "Host IP:",
            settings.ip,
            { -- Field validating & updating 
                update = function(field) -- Update host IP based on contents
                    settings.ip = field.text
                end,
                valid_string = is_valid_ip_string
            }
    )
end

local function connection_port_field_create()
    return text_field_create(
            "Connection Port:",
            settings.port,
            { -- Field validating & updating 
                update = function(field) -- Update connection port based on contents
                    settings.port = tonumber(field.text)
                end,
                valid_string = tonumber
            }
    )
end

local function connection_toggle_create()

    local client_option_image = image_cached_load("res/sprites/config/client_icon.png")
    local server_option_image = image_cached_load("res/sprites/config/server_icon.png")
    local single_player_option_image = image_cached_load("res/sprites/config/single_player_icon.png")

    local toggle = { 
        size = SETTINGS_BOX_SIZE,
        font = SETTINGS_FONT
    }

    function toggle:step(xy)
        -- Toggle the connection type
        if mouse_left_pressed and mouse_over(xy, self.size) then
            settings.connection_type = (settings.connection_type + 1) % 3
        end
    end
    
    function toggle:draw(xy)
        -- Draw the connection type

        local text, color, box_color, sprite

        if settings.connection_type == net.CLIENT then
            text = "Connect to a game"
            color = COL_MUTED_GREEN
            sprite = client_option_image
        elseif settings.connection_type == net.SERVER then
            text = "Host a game"
            color = COL_PALE_RED
            sprite = server_option_image
        else -- settings.connection_type == net.NONE
            text = "Single-player"
            color = COL_BABY_BLUE
            sprite = single_player_option_image
        end

        local x,y = unpack(xy)
        local w, h = unpack(self.size)

        sprite:draw( { color = color, origin = LEFT_CENTER }, { x, y + h / 2 } )
        self.font:draw( { color = color, origin = LEFT_CENTER }, { x + 8 + sprite.size[2], y + h / 2 }, text )

        local box_color = mouse_over(xy, self.size) and COL_GOLD or color
        draw_rectangle_outline(box_color, bbox_create(xy, self.size), 1)
    end

    return toggle
end

local function speed_toggle_create()
    local toggle = { 
        size = SETTINGS_BOX_SIZE,
        font = SETTINGS_FONT,
        sprite = image_cached_load("res/sprites/config/speed_setting.png")
    }

    function toggle:step(xy)
        -- Toggle the connection type
        if mouse_left_pressed and mouse_over(xy, self.size) then
            settings.time_per_step = settings.time_per_step - 2
            if settings.time_per_step < 10 then
                settings.time_per_step = 16
            end
        end
    end

    function toggle:draw(xy)
        local text = "Speed: Slow"
    
        if settings.time_per_step <= 10 then 
            text = "Speed: Very Fast"
        elseif settings.time_per_step <= 12 then
            text = "Speed: Fast"
        elseif settings.time_per_step <= 14 then
            text = "Speed: Normal"
        end
    
        local alpha = 255 - (settings.time_per_step - 10) * 30

        local x,y = unpack(xy)
        local w, h = unpack(self.size)

        self.sprite:draw( { color = {255, 255, 255, alpha}, origin = LEFT_CENTER }, { x, y + h / 2 } )
        self.font:draw( {origin = LEFT_CENTER}, { x + 8 + self.sprite.size[2], y + h / 2 }, text )
    
        local box_color = mouse_over(xy, self.size) and COL_GOLD or COL_WHITE
        draw_rectangle_outline(box_color, bbox_create(xy, self.size), 1)
    end

    return toggle
end

-- Simple layout built on top of InstanceGroup
local function column_instance_group_create(parameters) 
    local column_amount = parameters.columns
    local dx, dy = parameters.size[1] / column_amount, parameters.dy

    local instance_group = InstanceGroup.create()
    local column, row = 0, 0

    local new_group = {
        size = parameters.size
    }

    function new_group:draw(xy) 
        instance_group:draw(xy)
        DEBUG_BOX_DRAW(self, xy)
    end
    function new_group:step(xy) 
        instance_group:step(xy) 
    end

    function new_group:add_instance(obj) 
        instance_group:add_instance(obj, {column * dx, row * dy} )

        column = column + 1
        if column >= column_amount then
            column = 0
            row = row + 1
        end
    end

    function new_group:add_instances(...) 
        for instance in values{...} do 
            self:add_instance(instance) 
        end
    end

    return new_group
end

function game_settings_menu_create()
    local menu = InstanceGridGroup.create( {size = { 400, 192 }, dimensions = { 2, 3 }} )

    local components = {
        name_field_create(), 
        connection_toggle_create(), 
        host_IP_field_create(),
        connection_port_field_create(),
        speed_toggle_create()
    }

    for obj in values(components) do
        menu:add_instance(obj)
    end

    return menu
end