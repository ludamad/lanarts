local InstanceGroup = import "core.ui.InstanceGroup"
local InstanceLine = import "core.ui.InstanceLine"
local InstanceBox = import "core.ui.InstanceBox"
local TextInputBox = import "core.ui.TextInputBox"
local TextLabel = import "core.ui.TextLabel"
local Sprite = import "core.ui.Sprite"

local Network = import "core.Network"
local Keys = import "core.Keyboard"
local Mouse = import "core.Mouse"
local Display = import "core.Display"

local SETTINGS_BOX_MAX_CHARS = 18
local SETTINGS_BOX_SIZE = {180, 34}

local TEXT_COLOR = {255, 250, 240}
local CONFIG_MENU_SIZE = {640, 480}

local SETTINGS_FONT = font_cached_load(settings.font, 10)
local BIG_SETTINGS_FONT = font_cached_load(settings.menu_font, 20)

-- Adds common settings for text field functions that take size, font & max_chars
local function settings_text_field_params(params)
    params = params or {}
    params.size = params.size or SETTINGS_BOX_SIZE
    params.font = params.font or SETTINGS_FONT
    params.max_chars = params.max_chars or SETTINGS_BOX_MAX_CHARS
    return params
end

local function is_valid_ip_string(text)
    if text == "localhost" then return true end

    local parts = text:split(".")

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
    local params = settings_text_field_params {
        label_text = "Host IP:",
        default_text = settings.ip,
        input_callbacks = { -- Field validating & updating 
            update = function(field) -- Update host IP based on contents
                settings.ip = field.text
            end,
            valid_string = is_valid_ip_string
        }
    }
    return text_field_create(params)
end

local function connection_port_field_create()
    local params = settings_text_field_params {
        label_text = "Connection Port:",
        default_text = settings.port,
        input_callbacks = { -- Field validating & updating 
            update = function(field) -- Update connection port based on contents
                settings.port = tonumber(field.text)
            end,
            valid_string = tonumber
        }
    }
    return text_field_create(params)
end

local function connection_toggle_create()

    local client_option_image = image_cached_load("game/lanarts/interface/sprites/config/client_icon.png")
    local server_option_image = image_cached_load("game/lanarts/interface/sprites/config/server_icon.png")
    local single_player_option_image = image_cached_load("game/lanarts/interface/sprites/config/single_player_icon.png")

    local toggle = { 
        size = SETTINGS_BOX_SIZE,
        font = SETTINGS_FONT
    }

    function toggle:step(xy)
        -- Toggle the connection type
        if Mouse.mouse_left_pressed and mouse_over(xy, self.size) then
            settings.connection_type = (settings.connection_type + 1) % 3
        end
    end
    
    function toggle:draw(xy)
        -- Draw the connection type

        local text, color, box_color, sprite

        if settings.connection_type == Network.CLIENT then
            text = "Connect to a game"
            color = COL_MUTED_GREEN
            sprite = client_option_image
        elseif settings.connection_type == Network.SERVER then
            text = "Host a game"
            color = COL_PALE_RED
            sprite = server_option_image
        else -- settings.connection_type == Network.NONE
            text = "Single-player"
            color = COL_BABY_BLUE
            sprite = single_player_option_image
        end

        local x,y = unpack(xy)
        local w, h = unpack(self.size)

        sprite:draw( { color = color, origin = Display.LEFT_CENTER }, { x, y + h / 2 } )
        self.font:draw( { color = color, origin = Display.LEFT_CENTER }, { x + 8 + sprite.size[2], y + h / 2 }, text )

        local box_color = mouse_over(xy, self.size) and COL_GOLD or color
        Display.draw_rectangle_outline(box_color, bbox_create(xy, self.size), 1)
    end

    return toggle
end

local function respawn_toggle_create()
    local toggle = { 
        size = SETTINGS_BOX_SIZE,
        font = SETTINGS_FONT,
    }

    local respawn = image_cached_load("game/lanarts/interface/sprites/config/respawn_setting.png")
    local hardcore = image_cached_load("game/lanarts/interface/sprites/config/hardcore_setting.png")

    function toggle:step(xy)
        -- Toggle the connection type
        if Mouse.mouse_left_pressed and mouse_over(xy, self.size) then
            settings.regen_on_death = not settings.regen_on_death
        end
    end

    function toggle:draw(xy)
        local sprite = settings.regen_on_death and respawn or hardcore

        local x,y = unpack(xy)
        local w, h = unpack(self.size)

        local text = settings.regen_on_death and "Respawn on Death" or "Hardcore (No respawn!)"
        local text_color = settings.regen_on_death and TEXT_COLOR or COL_LIGHT_RED
        local sprite_color = settings.regen_on_death and TEXT_COLOR or COL_LIGHT_RED
        local box_color = sprite_color

        if mouse_over(xy, self.size) then 
            box_color = COL_GOLD 
        end

        sprite:draw( {origin = Display.LEFT_CENTER, color = sprite_color}, { x, y + h / 2 } )
        self.font:draw( {color = TEXT_COLOR, origin = Display.LEFT_CENTER, color = text_color}, { x + 8 + sprite.size[2], y + h / 2 }, text )
    
        Display.draw_rectangle_outline(box_color, bbox_create(xy, self.size), 1)
    end

    return toggle
end

local function frame_action_repeat_toggle_create()
    local toggle = { 
        size = SETTINGS_BOX_SIZE,
        large_font = BIG_SETTINGS_FONT,
        font = SETTINGS_FONT,
    }

    function toggle:step(xy)
        -- Toggle the connection type
        local mouseover = mouse_over(xy, self.size)
        if Mouse.mouse_left_pressed and mouseover then
            settings.frame_action_repeat = (settings.frame_action_repeat + 1) % 5
        elseif Mouse.mouse_right_pressed and mouseover then
            settings.frame_action_repeat = (settings.frame_action_repeat - 1) % 5
        end
    end

    function toggle:draw(xy)

        local x,y = unpack(xy)
        local w, h = unpack(self.size)

        self.font:draw( {color=COL_GREEN, origin = Display.LEFT_CENTER}, { x + 8, y + h / 2 }, 
            (settings.frame_action_repeat+1) .. 'x'
        )
        self.font:draw( {color = COL_PALE_GREEN, origin = Display.LEFT_CENTER}, { x + 40, y + h / 2 }, 
            "Network Skip Rate"
        )
    
        local box_color = mouse_over(xy, self.size) and COL_GOLD or COL_PALE_GREEN
        Display.draw_rectangle_outline(box_color, bbox_create(xy, self.size), 1)
    end

    return toggle
    
end

local function speed_toggle_create()
    local toggle = { 
        size = SETTINGS_BOX_SIZE,
        font = SETTINGS_FONT,
        sprite = image_cached_load("game/lanarts/interface/sprites/config/speed_setting.png")
    }

    function toggle:step(xy)
        -- Toggle the connection type
        if Mouse.mouse_left_pressed and mouse_over(xy, self.size) then
            settings.time_per_step = settings.time_per_step - 3
            if settings.time_per_step < 10 then
                settings.time_per_step = 19
            end
        end
    end

    function toggle:draw(xy)
        local text = "Speed: " .. speed_description(settings.time_per_step)
   
        local alpha = 255 - (settings.time_per_step - 10) * 20

        local x,y = unpack(xy)
        local w, h = unpack(self.size)

        self.sprite:draw( { color = {255, 255, 255, alpha}, origin = Display.LEFT_CENTER }, { x, y + h / 2 } )
        self.font:draw( {color = TEXT_COLOR, origin = Display.LEFT_CENTER}, { x + 8 + self.sprite.size[2], y + h / 2 }, text )
    
        local box_color = mouse_over(xy, self.size) and COL_GOLD or COL_WHITE
        Display.draw_rectangle_outline(box_color, bbox_create(xy, self.size), 1)
    end

    return toggle
end

local function label_button_create(params, color_formula, on_click)
    local sprite = Sprite.create(params.sprite, { color = COL_WHITE })
    local label = TextLabel.create(params.font, {}, params.text)

    local size = params.size

    local label_button = InstanceBox.create( params )

    label_button:add_instance( sprite, Display.CENTER_TOP )
    label_button:add_instance( label, Display.CENTER_BOTTOM )
    
    function label_button:step(xy) -- Makeshift inheritance
        InstanceBox.step(self, xy)

        if self:mouse_over(xy) and Mouse.mouse_left_pressed then 
            on_click(self, xy)
        end

        local color = color_formula(self, xy)
        sprite.options.color = color
        label.options.color = color == COL_WHITE and TEXT_COLOR or color
    end

    return label_button
end

local function class_choice_buttons_create()
    local sprite_base = "game/lanarts/interface/sprites/class_icons/"
    local x_padding, y_padding = 32, 16
    local font = BIG_SETTINGS_FONT

    local buttons = { 
        { "Mage", sprite_base .. "wizard.png"},
        { "Fighter", sprite_base .. "fighter.png"},
        { "Archer", sprite_base .. "archer.png"}
    }

    local button_size = { 96, 96 + y_padding + font.height }
    local button_row = InstanceLine.create( { dx = button_size[1] + x_padding } )

    function button_row:step(xy)
        InstanceLine.step(self, xy)

        -- Allow choosing a class by using left/right arrows or tab
        if Keys.key_pressed(Keys.LEFT) then
            settings.class_type = ( settings.class_type - 1 ) % #buttons
        elseif Keys.key_pressed(Keys.RIGHT) or Keys.key_pressed(Keys.TAB) then
            settings.class_type = ( settings.class_type + 1 ) % #buttons
        end
    end

    for i = 1, #buttons do
        local button = buttons[i]

        button_row:add_instance( 
            label_button_create(
                { size = button_size,
                  font = font,
                  text = button[1],
                  sprite = image_cached_load(button[2]) 
                },
                function(self, xy) -- color_formula
                    if settings.class_type == i-1 then
                        return COL_GOLD
                    else 
                        return self:mouse_over(xy) and COL_PALE_YELLOW or COL_WHITE
                    end
                end,
                function(self, xy) -- on_click
                    settings.class_type = i-1
                end
             ) 
         )

    end

    return button_row
end

local function center_setting_fields_create()
    local fields = InstanceLine.create( {force_size = {500, 162}, dx = 320, dy = 64, per_row = 2} )
    local current_setting 

    -- Adds different options depending on the connection type
    local function add_fields()
        current_setting = settings.connection_type

        fields:clear()

        fields:add_instance( connection_toggle_create() )

        if current_setting ~= Network.CLIENT then
            fields:add_instance( respawn_toggle_create() )
        end

        if current_setting ~= Network.CLIENT then
            fields:add_instance( speed_toggle_create() )
        end

        if current_setting == Network.CLIENT then
            fields:add_instance( host_IP_field_create() )
        end

        if current_setting == Network.SERVER then
           fields:add_instance( frame_action_repeat_toggle_create() )
        end

        local name_field = name_field_create( settings_text_field_params() )
        fields:add_instance(name_field)

        if current_setting ~= Network.NONE then
            fields:add_instance( connection_port_field_create() )
        end

    end
    
    add_fields() -- Do initial creation

    function fields:step(xy) -- Makeshift inheritance
        InstanceLine.step(self, xy)
        if current_setting ~= settings.connection_type then
            add_fields()
        end
    end

    return fields
end

local function choose_class_message_create()
    local label = TextLabel.create(SETTINGS_FONT, {}, "Choose your Class!")

    function label:step(xy) -- Makeshift inheritance
        TextLabel.step(self, xy)
        self:set_color()
    end

    function label:set_color()
        self.options.color = settings.class_type == -1 and COL_PALE_RED or COL_INVISIBLE
    end

    label:set_color() -- Ensure correct starting color

    return label
end

local function back_and_continue_options_create(on_back_click, on_start_click)
    local font = BIG_SETTINGS_FONT
    local options = InstanceLine.create( { dx = 200 } )

    -- associate each label with a handler
    -- we make use of the ability to have objects as keys
    local components = {
        [ TextLabel.create(BIG_SETTINGS_FONT, "Back") ] = on_back_click or do_nothing ,  
        [ TextLabel.create(BIG_SETTINGS_FONT, "Start") ] = on_start_click or do_nothing
    }

    for obj, handler in pairs(components) do
        options:add_instance(obj)
    end

    function options:step(xy) -- Makeshift inheritance
        InstanceLine.step(self, xy)
        for obj, obj_xy in self:instances(xy) do
            local click_handler = components[obj]

            local mouse_is_over = obj:mouse_over(obj_xy)
            obj.options.color = mouse_is_over and COL_GOLD or TEXT_COLOR

            if mouse_is_over and Mouse.mouse_left_pressed then click_handler() end
        end
    end

    return options
end

local function game_settings_menu_create(on_back_click, on_start_click)
    local fields = InstanceBox.create( {size = { 640, 480 } } )

    fields:add_instance( 
        class_choice_buttons_create(), 
        Display.CENTER_TOP, --[[Down 50 pixels]] { 0, 50 } )

    fields:add_instance( 
        center_setting_fields_create(), 
        {0.50, 0.70} )

    fields:add_instance( 
        back_and_continue_options_create(on_back_click, on_start_click), 
        Display.CENTER_BOTTOM, --[[Up 20 pixels]] { 0, -20 } )

    fields:add_instance( 
        choose_class_message_create(), 
        Display.CENTER_BOTTOM, --[[Up 50 pixels]] { 0, -50 }  )

    return fields
end

-- submodule
return {
    create = game_settings_menu_create
}

