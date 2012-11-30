arrows = drawable_parse("{ files: res/sample.png }")

function draw()
	arrows:draw({origin=CENTER}, {200,200})
end