arrow_frames = image_split( image_load("arrows.png"), {32,32} )
arrow = directional_create(arrow_frames, math.rad(90.0))

center = {200, 200}

local function position_angle(p1, p2) 
	return math.atan2(p2[2] - p1[2], p2[1] - p1[1])
end
	
function draw()	
	draw_rectangle(COL_BABY_BLUE, {0, 0, 400, 400});
	for y=0,9 do 
		for x=0,9 do 
			local pos = {x * 40 + 20, y * 40 + 20}
			local dir = position_angle(pos, center)
			arrow:draw({angle = dir, origin = CENTER}, pos)
		end
	end
end 