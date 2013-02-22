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

local function draw_colored_parts(font, origin, xy, ...)
    local rx, ry = 0, 0

    local parts = {...}
    local x_coords = {}

    -- First calculate relative positions
    for idx, part in ipairs(parts) do
        local color, text = unpack(part)
        local w, h = unpack( font:draw_size(text) )
        x_coords[idx] = rx
        rx = rx + w
    end

    local adjusted_origin = {0, origin[2]}
    local adjusted_x = xy[1] - rx * origin[1]

    -- Next draw according to origin
    for idx, part in ipairs(parts) do
        local color, text = unpack(part)
        local position = {adjusted_x + x_coords[idx],  xy[2]} 
        font:draw( { color = color, origin = adjusted_origin }, position, text)
    end
end

-- This is a hack.
-- Currently we cannot rely on the sprites being loaded, so we duplicate information from class_sprites.yaml here
-- This will be fixed in the coming move from YAML to Lua (allowing us to simply require 'class_sprite_data.lua')
local function class_image_for_name(name)
    local path_table = {
        fighter = "res/classes/sprites/fighter.png",
        fighter2 = "res/classes/sprites/fighter2.png", 
        wizard = "res/classes/sprites/wizard.png", 
        wizard2 = "res/classes/sprites/wizard2.png",
        archer = "res/classes/sprites/archer.png"
    }

    local path = path_table[name]
    if path and file_exists(path) then
        return image_cached_load(path)
    else
        return nil
    end
end

local function sorted_scores_fetch()
    local scores = game.score_board_fetch()
    local function entry_compare(a, b)
        if a.hardcore and not b.hardcore then
            return true
        elseif b.hardcore and not a.hardcore then
            return false
        end
        if a.deepest_floor == b.deepest_floor then

            if a.deaths == b.deaths then
                return a.kills > b.kills
            end
            return a.deaths < b.deaths
        end
        return a.deepest_floor > b.deepest_floor
    end
    table.sort(scores, entry_compare)
    return scores
end

local function scores_menu_list_create()

    local list_size = {300, 400} -- For placement algorithms
    local entry_height = 32
    local max_entries_to_draw = 10

    local scores = sorted_scores_fetch()
    local font = font_cached_load(settings.font, 10)

    local scores_drawer = { step = do_nothing }

    function scores_drawer:draw(xy)

        for idx, score_entry in ipairs(scores) do
            if idx > max_entries_to_draw then
                break
            end

            local ex = xy[1] - list_size[1] / 2
            local ey = xy[2] + (idx - 1) * entry_height
            local ew = list_size[1]
            local eh = entry_height

            local class_sprite = class_image_for_name(score_entry.sprite_name)
            if class_sprite then
                class_sprite:draw( {origin=RIGHT_CENTER}, {ex - 2, ey + eh/2} )
            end

            draw_colored_parts(font, LEFT_TOP, {ex + 2, ey + 2},
                {COL_MUTED_GREEN, score_entry.name .. "  "},
                {COL_WHITE, "Level " .. score_entry.character_level .. " "},
                {COL_WHITE, score_entry.class_name}
            )

            draw_colored_parts(font, RIGHT_TOP, {ex + ew - 4, ey + 2},
                {COL_LIGHT_GRAY, 
                 os.date("%Y %b %I:%M%p", score_entry.timestamp)}
            )

            draw_colored_parts(font, LEFT_BOTTOM, {ex + 2, ey+eh - 4},
                {COL_BABY_BLUE, "Reached Floor " .. score_entry.deepest_floor .. " "}
            )


            draw_colored_parts(font, CENTER_BOTTOM, {ex + ew/2, ey+eh - 4},
                {COL_MUTED_YELLOW, "Killed " .. score_entry.kills .. " Enemies"}
            )

            local death_entry
            if score_entry.hardcore then 
                death_entry = {COL_PALE_BLUE, "Hardcore"}
            else
                death_entry = {COL_PALE_RED, score_entry.deaths .. ((score_entry.deaths == 1) and " Death" or " Deaths")}
            end

            draw_colored_parts(font, RIGHT_BOTTOM, {ex + ew - 2, ey+eh - 4},
                death_entry
            )

            -- Draw a box around the entry
            draw_rectangle_outline( COL_DARK_GRAY, bbox_create( {ex - 32, ey}, {ew + 32, eh} ), 1)
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
