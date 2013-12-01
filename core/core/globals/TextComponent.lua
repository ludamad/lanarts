local Mouse = import "core.Mouse"
local InstanceGroup, TextLabel, InstanceBox, TextInputBox

-- Solve circular dependence by late-loading
local function ensure_loaded_dependencies()
    InstanceGroup = InstanceGroup or import "@ui.InstanceGroup"
    TextLabel = TextLabel or import "@ui.TextLabel"
    InstanceBox = InstanceBox or import "@ui.InstanceBox"
    TextInputBox = TextInputBox or import "@ui.TextInputBox"
end

--- Create a clickable piece of text
-- Displays the text 'text'
-- Performs function 'on_click' when clicked
-- Parameter table 'params': { 
--      'font' determins the font used to draw the text
--      'color' is the color of the text, default white
--      'hover_color' is the color when the mouse is over the text, default same as 'color'
--      'padding' controls how much bigger the click area is than the text, default 5
-- }
function text_button_create(text, on_click, params)
    ensure_loaded_dependencies()

    local no_hover_color = params.color or COL_WHITE
    local hover_color = params.hover_color or no_hover_color
    local padding = params.click_box_padding or 5
    local font = params.font

    -- Solves circular dependencies:
    local label = TextLabel.create(font, { color=no_hover_color }, text)

    function label:step(xy) -- Makeshift inheritance
        TextLabel.step(self, xy)

        local bbox = bbox_padded( xy, self.size, padding )
        self.options.color = bbox_mouse_over( bbox ) and hover_color or no_hover_color

        if Mouse.mouse_left_pressed and bbox_mouse_over( bbox ) then
            on_click()
        end
    end

    return label
end

--- Takes a parameter table with the following parameters: {
--      'font': The font to use when drawing the input box
--      'size': The size of the input box
--      'max_chars': The maximum characters allowed in the input box
--      'label_text': Optional, the text to display above the input box
--      'default_text': Optional, the default contents of the input box
--      'input_callbacks': Controls the TextInputBox, see TextInputBox.lua
-- }
function text_field_create(params)
    ensure_loaded_dependencies()

    local font, size = params.font, params.size
    local max_chars, label_text = params.max_chars, params.label_text
    local default_text = params.default_text or ""
    local callbacks = params.input_callbacks or {}

    local field = InstanceGroup.create()

    if label_text then
        -- Add text label
        field:add_instance(
            TextLabel.create(
                font, -- TextLabel font
                {color = COL_YELLOW }, 
                label_text
            ),
            {0, -20} -- position
        )
    end

    -- Add text input box
    field:add_instance(
        TextInputBox.create( 
            font, -- TextInputBox font
            size, -- Text input box size
            {max_chars, default_text}, -- input box parameters
            callbacks
        ),
       {0, 0} -- position
    )

    field.size = size -- Necessary for placement in InstanceBox's and InstanceLine's

    return field
end

-- Takes size, font, max_chars as parameters
function name_field_create(params)
    return text_field_create {
        size = params.size,
        font = params.font,
        max_chars = params.max_chars,
        label_text = params.label_text or "Enter your name:",
        default_text = settings.username,
        input_callbacks = {
            update = function(field) -- Update username based on contents
                settings.username = field.text
            end
        }
    }
end