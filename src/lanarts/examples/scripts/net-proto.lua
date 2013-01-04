
GameObject = {}
GameObject.__index = GameObject

function GameObject.init(obj)
	obj.pos = {0, 0}
	obj.radius = 16
end

function GameObject.step(obj)
	local x, y

	x, y = unpack(obj.pos)

	if key_held(keys.LEFT) 	then x = x - 1 end
	if key_held(keys.RIGHT) then x = x + 1 end

	if key_held(keys.UP) 	then y = y - 1 end
	if key_held(keys.DOWN) 	then y = y + 1 end

	obj.pos = {x, y}
end

function GameObject.draw(obj)
	draw_circle(COL_WHITE, obj.pos, obj.radius)
end

function create(class) 
	local obj = {}
	class.init(obj)
	setmetatable(obj, class)
	return obj
end

obj = create(GameObject)

function draw() 
	obj:step()
	obj:draw()
end