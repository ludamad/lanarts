-- TODO: recreate the menu screen in lua

local logo = image_load("res/lanarts_logo.png")

function draw()
	local w, h
	w, h = unpack(window_size())
	logo:draw( { origin=CENTER }, {w/2, h/4} )
end