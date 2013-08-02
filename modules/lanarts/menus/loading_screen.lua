local InstanceBox = import "core.ui.InstanceBox"
local InstanceLine = import "core.ui.InstanceLine"
local TextLabel = import "core.ui.TextLabel"
local Sprite = import "core.ui.Sprite"
local utils = import "core.utils"

local cached_screen
local setup_callback

local function cached_setup(percentage, task)
	local ret = cached_screen
	setup_callback(percentage, task)

	if percentage == 100 then -- cleanup
		cached_screen = nil
		setup_callback = nil
	end

	return ret
end

local function loading_string(percentage)
	return percentage .. "% Loaded"
end

local function center_screen_setup()
	local center_box = InstanceBox.create( { size = {640, 480} } )
	local task_label = TextLabel.create( Fonts.small, {color=COL_MUTED_GREEN, origin=CENTER_TOP}, "")
	local loading_label = TextLabel.create( Fonts.large, { color=COL_LIGHT_GRAY, origin=CENTER_TOP }, "" )
	local dragon_sprite = Sprite.image_create("modules/lanarts/enemies/bosses/sprites/reddragon.png")

	center_box:add_instance( 
		loading_label,
		CENTER,
		-- Up 100 pixels
		{0, -100}
	)
	center_box:add_instance( 
		dragon_sprite,
		CENTER
	)
	center_box:add_instance( task_label, CENTER, --[[Down 50 pixels]] {0, 50} )

	function setup_callback(percentage, task) 
		task_label.text = task
		loading_label.text = loading_string(percentage)
		dragon_sprite.options.color = with_alpha( COL_WHITE, percentage/100 )
	end

	return center_box
end

local function loading_screen_setup(...) 
	if cached_screen then
		return cached_setup(...)
	end

	local screen_box = InstanceBox.create( { size = Display.display_size } )

	screen_box:add_instance( center_screen_setup(), CENTER )

	-- set up cache
	cached_screen = screen_box

	return cached_setup(...)
end

-- Loading callback
local function loading_screen_draw(...)
	perf.timing_begin("system.loading_draw")
    Display.draw_start()

    local screen = loading_screen_setup(...)
    screen:draw( {0,0} )

    Display.draw_finish()
	perf.timing_end("system.loading_draw")
end

-- Submodule
return {
    draw = loading_screen_draw
}
