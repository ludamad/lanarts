require "utils_general" -- for 'memoized'

-- Draw parts of text colored differently
function draw_colored_parts(font, origin, xy, ...)
    local rx, ry = 0, 0

    local parts = {...}
    local x_coords = {}

    -- First calculate relative positions
    for idx, part in ipairs(parts) do
        local color, text = unpack(part)
        local w, h = unpack( font:draw_size(text) )
        x_coords[idx] = rx
        rx = rx + w
    end

    local adjusted_origin = {0, origin[2]}
    local adjusted_x = xy[1] - rx * origin[1]

    -- Next draw according to origin
    for idx, part in ipairs(parts) do
        local color, text = unpack(part)
        local position = {adjusted_x + x_coords[idx],  xy[2]} 
        font:draw( { color = color, origin = adjusted_origin }, position, text)
    end

    return rx -- return final width for further chaining
end

font_cached_load = memoized(font_load)
image_cached_load = memoized(image_load)

local DEBUG_FONT = font_cached_load(settings.menu_font, 10)

function DEBUG_BOX_DRAW(self, xy)
    if DEBUG_LAYOUTS then
        local mouse_is_over = mouse_over(xy, self.size)
        local color = mouse_is_over and COL_PALE_BLUE or COL_YELLOW
        local line_width = mouse_is_over and 5 or 2
        local alpha = mouse_is_over and 0.5 or 0.25

        if mouse_is_over then
            DEBUG_FONT:draw( { color = COL_WHITE, origin = LEFT_BOTTOM }, xy, tostring(self) )
        end

        draw_rectangle_outline(with_alpha(color, alpha), bbox_create(xy, self.size), line_width )
    end
end

function with_alpha(col, alpha) -- Don't mutate, we might be passed a colour constant!
    local copy = { unpack(col) } 
    -- Assume we have at least 3 components, but may have 4
    copy[4] = (copy[4] and copy[4] or 255 ) * alpha
    return copy    
end
