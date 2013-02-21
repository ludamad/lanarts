require "game_settings_menu"

require "InstanceBox"
require "Sprite"
require "TextLabel"
require "utils"
require "config"

local text_button_params = {
    font = font_cached_load(settings.menu_font, 12),
    color = COL_WHITE,
    hover_color = COL_RED,
    click_box_padding = 5
}

local function scores_menu_list_create()

    local list_size = {200, 400} -- For placement algorithms
    local entry_height = 40

    local scores = game.score_board_fetch()
    local font = font_cached_load(settings.font, 10)

    local scores_drawer = { step = do_nothing } 
    function scores_drawer:draw(xy)

        for idx, score_entry in ipairs(scores) do
            local ex = xy[1] - list_size[1] / 2
            local ey = xy[2] + (idx - 1) * entry_height

            local offset = {0,0}

            local function draw_part(color, text) 
                offset[1] = offset[1] + font:draw( 
                    { color = color}, 
                    { ex + offset[1], ey + offset[2] }, 
                    text
                )
            end
-- TODO: Draw box around score entry
            draw_part(COL_MUTED_GREEN, score_entry.name .. "  ")
            draw_part(COL_LIGHT_GRAY, "level " .. score_entry.character_level .. " ")
            draw_part(COL_LIGHT_GRAY, "Adventurer ")

            offset = {0, entry_height / 2}

            if score_entry.hardcore then 
                draw_part(COL_PALE_BLUE, "Hardcore")
            else 
                draw_part(COL_PALE_RED, "Deaths: " .. score_entry.deaths)
            end
        end
    end
    
    local group = InstanceGroup.create()
    group.size = {200, 400} 

    group:add_instance( 
        TextLabel.create(
            font_cached_load(settings.menu_font, 20), 
            {color=COL_WHITE, origin = CENTER_TOP}, 
            "Highscore List:"
        ), 
        {100, 30}
    )
    group:add_instance( scores_drawer, {100, 70} )

    return group
end

--            local text_parts = {
--                idx .. ") ", score_entry.name,
--                "floor=", score_entry.deepest_floor,
--                "kills=", score_entry.kills,
--                "level=", score_entry.character_level,
--                "hardcore=", tostring(score_entry.hardcore)
--            }

function scores_menu_create(on_back_click)
    local menu = InstanceBox.create( { size = display.window_size } )
    local logo_displacement = 0

    if menu.size[2] >= 600 then -- display the logo only if we're >= 800x600 res
        local logo = Sprite.image_create("res/interface/sprites/lanarts_logo.png")
    
        menu:add_instance(
            logo, 
            CENTER_TOP,
            --[[Down 10 pixels]]
            {0, 10}
        )

        logo_displacement = logo.size[2]
    end

    menu:add_instance(
        scores_menu_list_create(), 
        CENTER_TOP,
        {0, 10 + logo_displacement}
    )

    menu:add_instance(
        text_button_create("Back", on_back_click, text_button_params),
        CENTER_BOTTOM,
        --[[Up 40 pixels]]
        {0, -40}
    )

    return menu
end
