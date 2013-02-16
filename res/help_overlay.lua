require "InstanceBox"
require "InstanceLine"
require "TextLabel"
require "Sprite"
require "utils"


local function action_bar_hints_create()
    local hint_text = {'H', ' ', 'Y', 'U', 'I', 'O', 'P'}
    local options = { color = COL_PALE_YELLOW, origin = CENTER}

    local hints = { size = {32, 32 * #hint_text}, step = do_nothing }
    function hints.draw(xy) 
        local x, y = unpack(xy)
        for i, text in ipairs(hint_text) do
            fonts.large:draw( options, {x + (i-0.5) * 32, y}, text )
        end
    end

    return hint_text
end

local function help_overlay_create()
    local overlay = InstanceBox.create( {size = display.window_size} )
    local action_bar = action_bar_hints_create()
    overlay:add_instance( action_bar, LEFT_BOTTOM )

    return overlay
end

--local help_overlay = help_overlay_create()

function help_overlay_toggle() 
    if help_overlay then
        help_overlay = help_overlay_create()
    else
        help_overlay = nil
    end

end

function help_overlay_draw() 
    if help_overlay then
        help_overlay:draw()
    end
end