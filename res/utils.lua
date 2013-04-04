require "utils_general"
require "utils_math"
require "utils_draw"

-- More ad hoc utilities without a home, yet:

function bbox_mouse_over(bbox, origin)
    return bbox_contains( shift_origin(bbox, origin or LEFT_TOP), mouse_xy  )
end

function mouse_over(xy, size, origin)
    return bbox_mouse_over(bbox_create(xy, size), origin)
end


-- Returns a dummy object if the file doesn't exist
function music_optional_load(file) 
	return optional_load(file, music_load)
end

-- Returns a dummy object if the file doesn't exist
function sound_optional_load(file) 
	return optional_load(file, sound_load)
end