local Display = require "core.Display"

local InstanceBox = require "ui.InstanceBox"
local InstanceLine = require "ui.InstanceLine"
local TextLabel = require "ui.TextLabel"
local Sprite = require "ui.Sprite"
local World = require "core.World"

local function action_bar_hints_create()
    local hint_text = {'H', ' ', 'Y', 'U', 'I', 'O', 'P', 'J', 'K', 'L'}
    local options = {origin = Display.CENTER_TOP}

    local hints = { size = {32 * #hint_text, 32}, step = do_nothing }

    function hints:draw(xy) 
        local spells = World.local_player.spells
        local x, y = unpack(xy)
        for i, text in ipairs(hint_text) do
            options.color = with_alpha(COL_PALE_YELLOW, i-2 <= #spells and 1.0 or 0.2)
            Fonts.small:draw( options, {x + (i-0.5) * 32, y}, text )
        end
    end

    return hints
end

local function help_overlay_create()
    local overlay = InstanceBox.create( {size = Display.display_size} )
    local action_bar = action_bar_hints_create()
    overlay:add_instance( action_bar, Display.LEFT_BOTTOM, --[[Up 16 pixels]] {0, -16})

    return overlay
end
local help_overlay = help_overlay_create()

local function help_overlay_toggle() 
    if help_overlay then
        help_overlay = help_overlay_create()
    else
        help_overlay = nil
    end

end

local function help_overlay_draw() 
    if help_overlay then
        help_overlay:draw({0,0})
    end
end

return {toggle = help_overlay_toggle, draw = help_overlay_draw}
