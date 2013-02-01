require "utils"
require "InstanceGroup"
require "InstanceGridGroup"
require "TextInputBox"
require "Sprite"

local SETTINGS_BOX_MAXCHARS = 18
local SETTINGS_BOX_SIZE = {180, 34}

local CONFIG_MENU_SIZE = {640, 480}

local SETTINGS_FONT = font_cached_load("res/sample.ttf", 10)
local BIG_SETTINGS_FONT = font_cached_load("res/sample.ttf", 20)

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

local function label_button_create(params, onclick)
    local sprite = Sprite.create(params.sprite, { color = COL_WHITE })
    local label = TextLabel.create(params.font, {}, params.text)

    local size = params.size

    local label_button = InstanceOriginGroup.create( params )

    label_button:add_instance( sprite, CENTER_TOP )
    label_button:add_instance( label, CENTER_BOTTOM )
    
    function label_button:step(xy) -- Makeshift inheritance
        InstanceOriginGroup.step(self, xy)
        local mouse_is_over = mouse_over(xy, self.size)
        local color = mouse_is_over and COL_GOLD or COL_WHITE

        if mouse_is_over and mouse_left_pressed then
            if onclick then onclick() end
        end

        sprite.options.color = color
        label.options.color = color
    end

    return label_button
end

local function class_choice_buttons_create()
    local sprite_base = "res/sprites/class_icons/"
    local x_padding, y_padding = 32, 16
    local font = BIG_SETTINGS_FONT

    local buttons = { 
        { "Fighter", sprite_base .. "fighter.png"},
        { "Mage", sprite_base .. "wizard.png"},
        { "Archer", sprite_base .. "archer.png"}
    }
    local button_size = { 96, 96 + y_padding + font.height }
    local row_size = { (button_size[1] + x_padding) * #buttons, button_size[2] }

    local button_row = InstanceGridGroup.create( { size = row_size, dimensions = {#buttons, 1} } )

    for button in values(buttons) do
        local params = { size = button_size,
                         font = font,
                         text = button[1],
                         sprite = image_cached_load(button[2]) }

        button_row:add_instance( label_button_create(params) )
    end

    return button_row
end

local function center_setting_fields_create()
    local fields = InstanceGridGroup.create( {size = { 400, 192 }, size = { 400, 192 }, dimensions = { 2, 3 }} )

    local components = {
        name_field_create(), 
        connection_toggle_create(), 
        host_IP_field_create(),
        connection_port_field_create(),
        speed_toggle_create()
    }

    for obj in values(components) do
        fields:add_instance(obj)
    end

    return fields
end

local function choose_class_message_create()
    local label = TextLabel.create(SETTINGS_FONT, {}, "Choose your Class!")

    function label:step(xy) -- Makeshift inheritance
        TextLabel.step(self, xy)
        label.options.color = settings.class_type == -1 and COL_PALE_RED or COL_INVISIBLE
    end
    
    return label
end

local function back_and_continue_options_create(on_back_click, on_start_click)
    local font = BIG_SETTINGS_FONT
    local options = InstanceGridGroup.create( { size = { 200, font.height }, spacing = {200, font.height}, dimensions = { 2, 1 } } )

    -- associate each label with a handler
    -- we make use of the ability to have objects as keys
    local components = {
        [ TextLabel.create(BIG_SETTINGS_FONT, { origin = CENTER_TOP }, "BACK") ] = on_back_click or do_nothing ,  
        [ TextLabel.create(BIG_SETTINGS_FONT, { origin = CENTER_TOP }, "START") ] = on_start_click or do_nothing
    }

    for obj, handler in pairs(components) do
        options:add_instance(obj)
    end

    function options:step(xy) -- Makeshift inheritance
        InstanceGridGroup.step(self, xy)
        for obj, obj_xy in self:instances(xy) do
            local click_handler = components[obj]

            local mouse_is_over = obj:mouse_over(obj_xy)
            obj.options.color = mouse_is_over and COL_GOLD or COL_WHITE

            if mouse_is_over and mouse_left_pressed then click_handler() end
        end
    end

    return options
end

function game_settings_menu_create()
    local fields = InstanceOriginGroup.create( {size = { 640, 480 } } )

    fields:add_instance( class_choice_buttons_create(), CENTER_TOP, --[[Down 10 pixels]] { 0, 10 } )
    fields:add_instance( center_setting_fields_create(), {0.50, 0.70} )
    fields:add_instance( back_and_continue_options_create(), CENTER_BOTTOM, --[[Up 10 pixels]] { 0, -10 } )
    fields:add_instance( choose_class_message_create(), CENTER_BOTTOM, --[[Up 40 pixels]] { 0, -40 }  )

    return fields
end