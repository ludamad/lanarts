require "TextLabel"
require "utils"


function text_button_create(text, on_click, params)
    local no_hover_color = params.color or COL_WHITE
    local hover_color = params.hover_color or no_hover_color
    local padding = params.click_box_padding or 5
    local font = params.font

    local label = TextLabel.create(font, { color=no_hover_color }, text)

    function label:step(xy) -- Makeshift inheritance
        TextLabel.step(self, xy)

        local bbox = bbox_padded( xy, self.size, padding )
        self.options.color = bbox_mouse_over( bbox ) and hover_color or no_hover_color

        if mouse_left_pressed and bbox_mouse_over( bbox ) then
            on_click()
        end
    end

    return label
end