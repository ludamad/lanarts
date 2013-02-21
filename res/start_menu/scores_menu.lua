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

    local group = InstanceGroup.create()
    group.size = {200, 400} -- For placement algorithms

    group:add_instance( 
        TextLabel.create(
            font_cached_load(settings.menu_font, 20), 
            {color=COL_WHITE, origin = CENTER_TOP}, 
            "Highscore List:"
        ), 
        {100, 30}
    )

    local scores_drawer = { step = do_nothing }

    local scores = game.score_board_fetch()
    local font = font_cached_load(settings.font, 10)
    function scores_drawer:draw(xy)
        local x, y = unpack(xy)

        for idx, score_entry in ipairs(scores) do
            local text_parts = {
                idx .. ") ", score_entry.name,
                "floor=", score_entry.deepest_floor,
                "kills=", score_entry.kills,
                "level=", score_entry.character_level,
                "hardcore=", tostring(score_entry.hardcore)
            }

            font:draw( 
                { color = COL_WHITE, origin = CENTER_TOP }, 
                { x, y + (idx - 1) * 20 }, 
                table.concat(text_parts)
            )
        end    
    end

    group:add_instance( scores_drawer, {100, 70} )

    return group
end

function scores_menu_create(on_back_click)
    local menu = InstanceBox.create( { size = display.window_size } )
    local logo = Sprite.image_create("res/interface/sprites/lanarts_logo.png")

    menu:add_instance(
        logo, 
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        scores_menu_list_create(), 
        CENTER_TOP,
        {0, 10 + logo.size[2]}
    )

    menu:add_instance(
        text_button_create("Back", on_back_click, text_button_params),
        CENTER_BOTTOM,
        --[[Up 40 pixels]]
        {0, -40}
    )

    return menu
end
