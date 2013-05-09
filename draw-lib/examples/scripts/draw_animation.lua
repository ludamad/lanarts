puncher_frames = image_split( image_load("animation.png"), {480 / 6, 120} )
puncher = animation_create(puncher_frames, 0.1)

center = {200, 200}
frame = 0
	
function draw()	
	puncher:draw( { origin = CENTER, frame = frame }, center)
	frame = frame + 1
end