require "utils"
require "InstanceLine"
require "Sprite"

local game_entry_draw
local logo = image_cached_load("res/interface/sprites/lanarts_logo_no_sub.png")
local small_font = font_cached_load(settings.font, 10)
local medium_font = font_cached_load("res/fonts/MateSC-Regular.ttf", 14)
local large_font = font_cached_load("res/fonts/MateSC-Regular.ttf", 14)

local ENTRY_SIZE = {200, 40}
local ENTRY_SPACING = 45

local sample_lobby_entry = { 
    host = "ludamad",
    max_players = 4, 
    players = {
        "ciribot"
    }
}

local sample_player_entry = {
    name = "ciribot",
    registered = true
    -- In the future stats will go here. However it is too early to try to work with them.
}


local function game_entry_create(entry_number, entry_data)
    local obj = {}
    obj.entry_data = entry_data
    obj.size = ENTRY_SIZE

    function obj:step(xy)
        local bbox = bbox_create(xy, self.size)
        if bbox_left_clicked(bbox) then
            print("Event entry clicked, event_data = ")
            pretty_print(self.entry_data)
        end
    end

    function obj:draw(xy)
        local bbox = bbox_create(xy, self.size)
        game_entry_draw(entry_number, self.entry_data, bbox)
    end
    return obj
end

-- Recreated every time the game set changes
local function game_entry_list_create()
    local obj = InstanceLine.create( {dx = 0, dy = ENTRY_SPACING, per_row = 1} )
    local entries = { sample_player_entry, sample_player_entry } -- TODO real data
    for i=1,#entries do
        obj:add_instance(game_entry_create(i, entries[i]))
    end
    return obj
end

local function draw_in_box(font, bbox, origin, offset, ...)
    return draw_colored_parts(font, origin, origin_aligned(bbox, origin, {0,0}, offset), ...)
end

function game_entry_draw(number, entry, bbox)
    local game_number_color = vector_interpolate(COL_YELLOW, COL_DARK_GRAY, (number-1) / 10)
    draw_in_box(medium_font, bbox, LEFT_CENTER, {-14,0}, {game_number_color, number})
    draw_in_box(small_font, bbox, LEFT_TOP, {0,3}, {COL_WHITE, "Players: "}, {COL_MUTED_GREEN, entry.name})
    draw_in_box(small_font, bbox, LEFT_TOP, {0,18}, {COL_WHITE, "Host: "}, {COL_PALE_RED, entry.name})

    draw_rectangle_outline( bbox_mouse_over(bbox) and COL_WHITE or COL_GRAY, bbox, 1 )
end

function main()
    Display.initialize("Lanarts Example", {800, 600}, false)
    local state = {}
    state.menu = game_entry_list_create()

    while Game.input_capture() do
        state.menu:step({20,200})

        Display.draw_start()
        logo:draw( {10,10} )
        state.menu:draw( {20,200} )
        Display.draw_finish()

        Game.wait(5)
    end
end

