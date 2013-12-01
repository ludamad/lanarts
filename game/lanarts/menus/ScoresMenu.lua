local Display = import "core.Display"
local GameState = import "core.GameState"

local InstanceBox = import "core.ui.InstanceBox"
local InstanceGroup = import "core.ui.InstanceGroup"
local Sprite = import "core.ui.Sprite"
local TextLabel = import "core.ui.TextLabel"
local Keys = import "core.Keyboard"

local score_menu_font = "game/fonts/alagard_by_pix3m-d6awiwp.ttf"

-- This is a hack.
-- Currently we cannot rely on the sprites being loaded, so we duplicate information from class_sprites.yaml here
-- This will be fixed in the coming move from YAML to Lua (allowing us to simply require 'class_sprite_data.lua')
local function class_image_for_name_hack(name)
    local path_table = {
        fighter = "game/lanarts/classes/sprites/fighter.png",
        fighter2 = "game/lanarts/classes/sprites/fighter2.png", 
        wizard = "game/lanarts/classes/sprites/wizard.png", 
        wizard2 = "game/lanarts/classes/sprites/wizard2.png",
        archer = "game/lanarts/classes/sprites/archer.png"
    }

    local path = path_table[name]
    if path and file_exists(path) then
        return image_cached_load(path)
    else
        return nil
    end
end

local TEXT_COLOR = {255, 250, 240}

local function sorted_scores_fetch()
    local scores = GameState.score_board_fetch()

    local function entry_compare(a, b)
        if a.won_the_game ~= b.won_the_game     then return a.won_the_game end
        if a.hardcore ~= b.hardcore             then return a.hardcore end
        if a.deepest_floor ~= b.deepest_floor   then return a.deepest_floor > b.deepest_floor end
        if a.deaths ~= b.deaths                 then return a.deaths < b.deaths end
        -- else
        return a.kills > b.kills
    end

    table.sort(scores, entry_compare)
    return scores
end

local function score_entry_draw(entry, ranking, ex, ey, ew, eh)
    -- Draw ranking
    local ranking_font = font_cached_load("game/fonts/MateSC-Regular.ttf", 14)
    local ranking_color = vector_interpolate(COL_YELLOW, COL_DARK_GRAY, (ranking-1) / 10)
    ranking_font:draw( 
        {color = ranking_color, origin = Display.RIGHT_CENTER}, 
        {ex - 40, ey + eh/2}, 
        ranking
    )

    local font = font_cached_load(settings.font, 10)
    -- Draw class sprite
    local class_sprite = class_image_for_name_hack(entry.sprite_name)
    if class_sprite then
        class_sprite:draw( {origin=Display.RIGHT_CENTER}, {ex - 2, ey + eh/2} )
    end

    -- Draw character name & level
    draw_colored_parts(font, Display.LEFT_TOP, {ex + 2, ey + 2},
        {COL_MUTED_GREEN, entry.name .. "  "},
        {TEXT_COLOR, "Level " .. entry.character_level .. " "},
        {TEXT_COLOR, entry.class_name}
    )

    -- Draw timestamp
    draw_colored_parts(font, Display.RIGHT_TOP, {ex + ew - 4, ey + 2},
        {COL_LIGHT_GRAY,
         os.date("%Y %b %d %I:%M%p", entry.timestamp)}
    )

    -- Draw either 'Has Won!' or deepest floor
    local progress_entry
    if entry.won_the_game then 
        progress_entry = {COL_PALE_BLUE, "Victorious!"}
    else
        progress_entry = {COL_BABY_BLUE, "Reached Floor " .. entry.deepest_floor .. " "}
    end
    draw_colored_parts(font, Display.LEFT_BOTTOM, {ex + 2, ey+eh - 4}, progress_entry)

    -- Draw how many monsters killed
    draw_colored_parts(font, Display.LEFT_BOTTOM, {ex + ew/2 - 50, ey+eh - 4},
        {COL_MUTED_YELLOW, "Killed " .. 
         entry.kills .. (( entry.kills == 1) and " Enemy" or " Enemies")}
    )

    -- Draw either 'Hardcore' text or death count
    local death_entry
    if entry.hardcore then 
        death_entry = {COL_PALE_BLUE, "Hardcore"}
    else
        death_entry = { COL_PALE_RED, entry.deaths .. ((entry.deaths == 1) and " Death" or " Deaths") }
    end

    draw_colored_parts(font, Display.RIGHT_BOTTOM, {ex + ew - 2, ey+eh - 4},
        death_entry
    )

    -- Draw a box around the entry
    Display.draw_rectangle_outline( 
        entry.won_the_game and COL_LIGHT_GRAY or COL_DARK_GRAY, 
        bbox_create( {ex - 32, ey}, {ew + 32, eh} ), 
        1  -- outline width
    )
end

local function score_entry_drawer_create()
    local entry_width, entry_height = 400, 32
    local entry_space = 2

    local scores_drawer = {
        scores = sorted_scores_fetch(),
    }
    scores_drawer.index_start = 1
    scores_drawer.index_total = math.min(#scores_drawer.scores, 10)

    function scores_drawer:scroll_list(num_entries)
        local new_start = self.index_start + num_entries
        new_start = math.max(new_start, 1)
        new_start = math.min(new_start, #self.scores - self.index_total + 1)
        self.index_start = new_start
    end

    function scores_drawer:step(xy)
        if Keys.key_pressed(Keys.UP) then self:scroll_list(-1) end
        if Keys.key_pressed(Keys.PAGE_UP) then self:scroll_list(-10) end
        if Keys.key_pressed(Keys.DOWN) then self:scroll_list(1) end
        if Keys.key_pressed(Keys.PAGE_DOWN) then self:scroll_list(10) end
    end

    function scores_drawer:draw(xy)
        local index_end = self.index_start + self.index_total - 1
        local ex, ey = xy[1] - entry_width / 2, xy[2]
        for idx = self.index_start, index_end do
            score_entry_draw(self.scores[idx], idx, ex, ey, entry_width, entry_height)
            ey = ey + (entry_height + entry_space)
        end
    end

    return scores_drawer
end

local function scores_menu_body_create()
    local group = InstanceGroup.create()
    group.size = {200, 400} -- for placement algorithm

    -- Title text
    group:add_instance( 
        TextLabel.create(
            font_cached_load(score_menu_font, 20), 
            {color=TEXT_COLOR, origin = Display.CENTER_TOP}, 
            "In Memoriam"
        ), 
        {100, 30}
    )

    -- Object that draws the score entries   
    group:add_instance( 
        score_entry_drawer_create(), 
        {100, 70} 
    )

    return group
end

local function scores_menu_create(on_back_click)
    local menu = InstanceBox.create( { size = Display.display_size } )
    local logo_displacement = 0

    -- Display the logo if we are >= 800x600 res
    if menu.size[2] >= 600 then
        local logo = Sprite.image_create("game/lanarts/LANARTS.png")
    
        menu:add_instance(
            logo, 
            Display.CENTER_TOP,
            --[[Down 10 pixels]]
            {0, 10}
        )

        logo_displacement = logo.size[2]
    end

    -- Score title + drawer object
    menu:add_instance(
        scores_menu_body_create(), 
        Display.CENTER_TOP,
        {0, 10 + logo_displacement}
    )

    -- Back button
    menu:add_instance(
        text_button_create("Back", on_back_click, {
            font = font_cached_load(score_menu_font, 20),
            color = TEXT_COLOR,
            hover_color = COL_RED,
            click_box_padding = 5
        }),
        Display.CENTER_BOTTOM,
        --[[Up 25 pixels]]
        {0, -25}
    )

    return menu
end

-- submodule
return {
    create = scores_menu_create
}
