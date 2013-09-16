local Display = import "core.Display"
local GameState = import "core.GameState"

local InstanceBox = import "core.ui.InstanceBox"
local InstanceLine = import "core.ui.InstanceLine"
local TextLabel = import "core.ui.TextLabel"
local Sprite = import "core.ui.Sprite"
local utils = import "core.Utils"
local keys = import "core.Keyboard"

local tutorial_title_font = settings.menu_font

local function lanarts_explain_screen_create(click_back, click_forward) 
    local title_font = font_cached_load(tutorial_title_font, 20)
    local font = font_cached_load(tutorial_title_font, 12)

    local contents = InstanceBox.create( { size = {640, 480} } )

    contents:add_instance(
         TextLabel.create( title_font, {color=COL_WHITE}, "Lanarts"),
         Display.LEFT_TOP,
         {50, 50 }
    )

    contents:add_instance(
         TextLabel.create( 
            font, 
            { color=COL_WHITE, max_width = 400}, 
            string.pack [[
                Press YUIOP to use spell slots 1-5. test test test test test test test test test test test test test test test test test test test test 
            ]]
         ),
         Display.CENTER
    )

    local window = InstanceBox.create( { size = Display.display_size} )
    window:add_instance(
         contents,
         Display.CENTER
    )
    
    return window
end

local tutorial_screens = {lanarts_explain_screen_create}

local function tutorial_menu_show(...)
    local screen

    local screen_idx = 1
    local go_back, go_forward -- foward declare

    local function update_screen(idx)
        screen_idx, screen = idx, tutorial_screens[idx]
        if screen then screen = screen(go_back, go_forward) end
    end

    update_screen(screen_idx)

    function go_back()    update_screen(screen_idx - 1) end
    function go_forward() update_screen(screen_idx + 1) end

    while GameState.input_capture() and not Keys.key_pressed(keys.ESCAPE) and screen  do
        Display.draw_start()
        screen:step( {0,0} )

        if Keys.key_pressed(keys.ESCAPE) then
            screen = nil -- exit
        elseif Keys.key_pressed(keys.ENTER) or Keys.key_pressed(keys.RIGHT) then 
            go_forward()
        elseif Keys.key_pressed(keys.LEFT) then 
            go_back()
        end

        screen:draw( {0,0} )
        Display.draw_finish()
        GameState.wait(100)
    end

    return false
end

-- submodule
return {
    show = tutorial_menu_show
}

