local InstanceBox = import "core.ui.InstanceBox"
local InstanceLine = import "core.ui.InstanceLine"
local TextLabel = import "core.ui.TextLabel"
local Sprite = import "core.ui.Sprite"
local utils = import "core.utils"

local tutorial_title_font = settings.menu_font

local function lanarts_explain_screen_create(click_back, click_forward) 
    local title_font = font_cached_load(tutorial_title_font, 20)
    local font = font_cached_load(tutorial_title_font, 12)

    local contents = InstanceBox.create( { size = {640, 480} } )

    contents:add_instance(
         TextLabel.create( title_font, {color=COL_WHITE}, "Lanarts"),
         LEFT_TOP,
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
         CENTER
    )

    local window = InstanceBox.create( { size = Display.display_size} )
    window:add_instance(
         contents,
         CENTER
    )
    
    return window
end

local tutorial_screens = {lanarts_explain_screen_create}

function tutorial_menu_show(...)
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

    while Game.input_capture() and not key_pressed(keys.ESCAPE) and screen  do
        Display.draw_start()
        screen:step( {0,0} )

        if key_pressed(keys.ESCAPE) then
            screen = nil -- exit
        elseif key_pressed(keys.ENTER) or key_pressed(keys.RIGHT) then 
            go_forward()
        elseif key_pressed(keys.LEFT) then 
            go_back()
        end

        screen:draw( {0,0} )
        Display.draw_finish()
        Game.wait(100)
    end

    return false
end