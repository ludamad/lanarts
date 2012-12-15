local directional = drawable_parse("{ type: directional, frames: res/arrow/arrow(0-7).png }")
local animation = drawable_parse("{ type: animation, frames: res/arrow/arrow(0-7).png }")
local split_animation = drawable_parse("{ type: animation, frames: res/arrows.png%32x32 }")

local frame = 0
local angle = 0

local function drawargs()
	return {origin=CENTER, angle=angle, frame=frame}
end

function test_draw_function()
	split_animation:draw({frame=frame, alpha=0.5}, {200,50})
end

function draw()
	angle = angle + math.pi / 180
	frame = frame +  8/180
	
	directional:draw({angle=angle}, {200,200})
	animation:draw({frame=frame}, {200,150})
	split_animation:draw({frame=frame}, {200,100})
	local luadrawable = drawable_parse("{ type: lua, function: test_draw_function }")
	
end