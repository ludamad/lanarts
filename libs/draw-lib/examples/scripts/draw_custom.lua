image = image_load("sample.png", true)

angle = 0.0
pi = math.pi

colors = {
	COL_PALE_RED, 	
	COL_GOLD,
	COL_LIGHT_RED,
	COL_BABY_BLUE,
	COL_LIGHT_BLUE,
}

function pick_color(idx) 
	return colors[ (idx % 5) + 1 ]
end

function coords(angle, cent, rad) 
	local sinval = math.sin(angle)
	local cosval = math.cos(angle)
	return cent + sinval * rad, cent + cosval * rad
end

function draw()
	angle = angle + pi / 100

	local ang = angle 
	for i = 1,50 do
		local x, y
		x,y = coords(ang, 200, 200)
		ang = ang + pi / 25
			
		local scalex, scaley
		scalex = 1.5 + math.sin(ang+angle)*.5
		
		local clr = pick_color(i)
		clr[4] = 100 * scalex
		
		image:draw(
			{ color=pick_color(i), origin=CENTER,
			  scale={scalex,scalex}, angle=angle }, 
			{x, y}
		)
		
	end
end
drawable = drawable_create(draw)
