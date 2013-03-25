-- Cache some locals for performance:
local assert = assert 
local unpack = unpack

function file_exists(name)
	local f = io.open(name,"r")
	if f ~= nil then io.close(f) end
	return f ~= nil
end

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

function vector_interpolate(v1, v2, percentage --[[0 results in v1, 1 results in v2, anything else is inbetween]])
    local length = # v1
    percentage = math.max( math.min(percentage, 1.0), 0.0)

    assert(length == #v2, "vector_interpolate(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do 
        local diff = v2[i] - v1[i]
        ret[i] = v1[i] + diff * percentage
    end

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

function pretty_tostring(val, --[[Optional]] tabs, --[[Optional]] packed, --[[Optional]] quote_strings)
    tabs = tabs or 0
    quote_strings = (quote_strings == nil) or quote_strings

    local tabstr = ""

    if not packed then
        for i = 1, tabs do
            tabstr = tabstr .. "  "
        end
    end

    if type(val) == "string" and quote_strings then
        return tabstr .. "\"" .. val .. "\""
    end

    if type(val) ~= "table" then
        return tabstr .. tostring(val)
    end

    local parts = {"{", --[[sentinel for remove below]] ""}

    for k,v in pairs(val) do
        table.insert(parts, packed and " " or "\n") 

        if type(k) == "number" then
            table.insert(parts, pretty_tostring(v, tabs+1, packed))
        else 
            table.insert(parts, pretty_tostring(k, tabs+1, packed, false))
            table.insert(parts, " = ")
            table.insert(parts, pretty_tostring(v, type(v) == "table" and tabs+1 or 0))
        end

        table.insert(parts, ",")
    end

    parts[#parts] = nil -- remove comma or sentinel

    table.insert(parts, (packed and " " or "\n") .. tabstr .. "}");

    return table.concat(parts)
end

function pretty_print(val, --[[Optional]] tabs, --[[Optional]] packed)
    print(pretty_tostring(val, tabs, packed))
end

function optional_load(file, loader) 
	if not file_exists(file) then
		return { 
			play = function() end, 
			loop = function() end 
		}
	end
	return loader(file)
end

-- Returns a dummy object if the file doesn't exist
function music_optional_load(file) 
	return optional_load(file, music_load)
end

-- Returns a dummy object if the file doesn't exist
function sound_optional_load(file) 
	return optional_load(file, sound_load)
end
