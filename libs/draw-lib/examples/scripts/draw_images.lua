image = image_load("sample.png")
function draw()
	for y=0,9 do 
		for x=0,9 do 
			image:draw({x*40, y *40})
		end
	end
end