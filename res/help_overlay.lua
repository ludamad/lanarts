require "InstanceBox"
require "InstanceLine"
require "TextLabel"
require "Sprite"
require "utils"

local function action_bar_hints_create()
    local hint_text = {'H', ' ', 'Y', 'U', 'I', 'O', 'P'}
    local options = {origin = CENTER_TOP}

    local hints = { size = {32 * #hint_text, 32}, step = do_nothing }

    function hints:draw(xy) 
        local spells = world.local_player.spells
        local x, y = unpack(xy)
        for i, text in ipairs(hint_text) do
            options.color = with_alpha(COL_PALE_YELLOW, i-2 <= #spells and 1.0 or 0.2)
            fonts.small:draw( options, {x + (i-0.5) * 32, y}, text )
        end
    end

    return hints
end

local function help_overlay_create()
    local overlay = InstanceBox.create( {size = display.window_size} )
    local action_bar = action_bar_hints_create()
    overlay:add_instance( action_bar, LEFT_BOTTOM, --[[Up 16 pixels]] {0, -16})

    return overlay
end
local help_overlay = help_overlay_create()

function help_overlay_toggle() 
    if help_overlay then
        help_overlay = help_overlay_create()
    else
        help_overlay = nil
    end

end

function help_overlay_draw() 
    if help_overlay then
        help_overlay:draw({0,0})
    end
end