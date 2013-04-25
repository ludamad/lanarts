-- Cache some locals for performance:
local assert = assert 
local unpack = unpack

function with_alpha(col, alpha) -- Don't mutate, we might be passed a colour constant!
    local copy = { unpack(col) } 
    -- Assume we have at least 3 components, but may have 4
    copy[4] = (copy[4] and copy[4] or 255 ) * alpha
    return copy    
end

function memoized(func, separator) 
    local cache = {}
    setmetatable( cache, {__mode = "kv"} ) -- Make table weak

    separator = separator or ";"

    return function(...)
        local key = table.concat({...}, separator)

        if not cache[key] then 
            cache[key] = func(...)
        end

        return cache[key]
    end
end

function vector_multiply(v1, v2)
    local length = # v1

    assert(length == #v2, "vector_multiply(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] * v2[i] end

    return ret
end

function vector_divide(v1, v2)
    local length = # v1

    assert(length == #v2, "vector_divide(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] / v2[i] end

    return ret
end

function do_nothing() 
    -- does nothing
end

function origin_valid(origin)
    local rx, ry = unpack(origin)
    return rx >= 0 and rx <= 1 and ry >= 0 and ry <= 1
end

function bbox_mouse_over(bbox, origin)
    return bbox_contains( shift_origin(bbox, origin or LEFT_TOP), mouse_xy  )
end

function bbox_padded(xy, size, pad) 
    return { xy[1] - pad, 
             xy[2] - pad, 
             xy[1] + size[1] + pad, 
             xy[2] + size[2] + pad }
end

function mouse_over(xy, size, origin)
    return bbox_mouse_over(bbox_create(xy, size), origin)
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