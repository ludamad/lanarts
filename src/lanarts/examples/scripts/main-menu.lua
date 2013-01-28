-- HELPERS --

local function bbox_padded(xy, size, pad) 
	return { xy[1] - pad, 
			 xy[2] - pad, 
			 xy[1] + size[1] + pad, 
			 xy[2] + size[2] + pad }
end

-- BUTTONS --

local function mouse_over(bbox, origin)
	return bbox_contains( shift_origin(bbox, origin or LEFT_TOP), mouse_xy  )
end

local TextButton = newtype()

function TextButton.init(self, font, text, action, col1, col2)
	self.font = font

	self.size = font:draw_size(text)
	self.text = text

	self.action = action

	self.col1 = col1 or COL_WHITE -- default
	self.col2 = col2 or COL_RED -- default

	self.padding = 5
	self.origin = CENTER
end

function TextButton._click_area(self, xy) 
	return bbox_padded( xy, self.size, self.padding )
end

function TextButton.step(self, xy)
	if mouse_left_pressed and self.action ~= nil then
		if mouse_over( self:_click_area(xy), self.origin ) then
			self.action()
		end
	end
end

function TextButton.draw(self, xy)
	local text_color = self.col1

	if mouse_over( self:_click_area(xy), self.origin ) then 
		text_color = self.col2 
	end

	self.font:draw( { origin = self.origin, color = text_color }, xy, self.text )
end

-- MENUS -- 

local function do_nothing() 
	-- does nothing
end

local function blank_menu(state)
	return { step = do_nothing, draw = do_nothing}
end

local function main_menu(state)
	local font = font_load("res/sample.ttf", 20)

	local logo = image_load("res/lanarts_logo.png")

	local function switch_menu()
		state.menu = blank_menu(state) 
	end

	local start = TextButton.create(font, "START", switch_menu)

	local menu = {}

	function menu.step()
		local w, h = unpack(display.window_size)
	
		start:step( {w/2, h*3/4} )
	end

	function menu.draw() 
		local w, h = unpack(display.window_size)

		logo:draw( { origin=CENTER }, {w/2, h/4} )
		start:draw( {w/2, h*3/4} )
	end

	return menu
end

-- MAIN --

function main()
	display.initialize("Lanarts Example", {640, 480}, false)
	local state = {}
	state.menu = main_menu(state)

	while game.input_capture() do
		state.menu.step()

		display.draw_start()
		state.menu.draw()
		display.draw_finish()

		game.wait(5)
	end
end