-- Use this include when you want util_* functions.
-- As well as including those, it defines some additional ad-hoc utility functions.

local Display = import "core.Display"
local Mouse = import "core.Mouse"

import "@utils.UtilsGeneral"
import "@utils.UtilsMath"
import "@utils.UtilsDraw"
import "@utils.UtilsDescription"
import "@utils.UtilsTextComponent"
local Sound = import "core.Sound"

-- More ad hoc utilities without a home, yet

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

--- Return a dummy sound or music object if the given file does not exist.
-- Otherwise return the result of 'loader' on the file.
local function snd_optional_load(file, loader)  
    if not file_exists(file) then 
        return {  
            play = function() end,  
            loop = function() end  
        } 
    end 
    return loader(file) 
end

--- Returns a dummy object if the file doesn't exist
function music_optional_load(file) 
	return snd_optional_load(file, Sound.music_load)
end


--- Returns a dummy object if the file doesn't exist
function sound_optional_load(file) 
	return snd_optional_load(file, Sound.sound_load)
end
