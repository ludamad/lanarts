-- Use this include when you want util_* functions.
-- As well as including those, it defines some additional ad-hoc utility functions.

require "utils_general"
require "utils_math"
require "utils_draw"

-- More ad hoc utilities without a home, yet

--- Return whether the mouse is within a bounding box.
function bbox_mouse_over(bbox, origin)
    return bbox_contains( shift_origin(bbox, origin or LEFT_TOP), mouse_xy )
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
	return snd_optional_load(file, music_load)
end


--- Returns a dummy object if the file doesn't exist
function sound_optional_load(file) 
	return snd_optional_load(file, sound_load)
end
