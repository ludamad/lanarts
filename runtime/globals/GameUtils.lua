local Display = require "core.Display"
local Mouse = require "core.Mouse"
local GameState = require "core.GameState"

-- More ad hoc utilities without a home, yet

function to_tilexy(xy)
    return {math.floor(xy[1]/32), math.floor(xy[2]/32)}
end
function to_worldxy(xy)
    return {xy[1]*32+16, xy[2]*32+16}
end

--- Return whether the mouse has been right clicked within a bounding box.
function bbox_right_clicked(bbox, origin)
    return Mouse.mouse_right_pressed and bbox_mouse_over(bbox, origin)
end

--- Return whether the mouse has been left clicked within a bounding box.
function bbox_left_clicked(bbox, origin)
    return Mouse.mouse_left_pressed and bbox_mouse_over(bbox, origin)
end

--- Return whether the mouse is within a bounding box.
function bbox_mouse_over(bbox, origin)
    return bbox_contains(Display.shift_origin(bbox, origin or Display.LEFT_TOP), Mouse.mouse_xy )
end

--- Return whether the mouse is within a bounding box defined by xy and size.
function mouse_over(xy, size, origin)
    return bbox_mouse_over(bbox_create(xy, size), origin)
end

local event_log_enabled = GameState.event_log_is_active
function event_log(format_str, ...)
    if event_log_enabled() then -- TODO potentially cache this result?
        --local stack = "Stack: {"
        --for i=2,3 do
        --    local info = debug.getinfo(i, 'lSn')
        --    if info then
        --        stack = stack .. (info.name or '<anon>').. " " .. info.source .. ":" .. info.currentline .. ', '
        --    end
        --end
        --stack = stack .. '}\n'
        --GameState.raw_event_log(stack .. format_str:format(...))
        GameState.raw_event_log(format_str:format(...))
    end
end
