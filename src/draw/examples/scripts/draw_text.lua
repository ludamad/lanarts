font = font_load("sample.ttf", 20)

function draw()
	font:draw(COL_WHITE, {0,0}, "Hello World!")
	font:draw_wrapped({origin = CENTER, color = COL_PALE_RED},
			{200, 200}, 250,
			"This text is wrapped because it's sort of long.")
end