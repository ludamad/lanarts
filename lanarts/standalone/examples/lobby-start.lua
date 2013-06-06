require "utils"
require "InstanceBox"
require "InstanceLine"
require "Sprite"
require "TextLabel"

require "networking.lobby"
require "networking.tasks"

require "utils_text_component"

local game_entry_draw
local logo_path = "res/interface/sprites/lanarts_logo_no_sub.png"
local small_font = font_cached_load(settings.font, 10)
local large_font = font_cached_load(settings.font, 20)
local alt_font = font_cached_load("res/fonts/MateSC-Regular.ttf", 14)

local SETTINGS_BOX_MAX_CHARS = 18
local SETTINGS_BOX_SIZE = {180, 34}

local ENTRY_SIZE = {350, 40}
local ENTRY_SPACING = 45
local PLAYER_LIST_MAX_CHARS = 50

-- For reference, this is how the received table looks
local sample_lobby_entry = { 
    host = "ludamad",
    creationTime = 1010010,
    id = "51b002b8e1382367f2000003",
    players = {
        "ciribot",
        "ludamad",
    }
}

-- Filled with data from the server
local entries = { }

local configuration = {
    update_frequency = 5000 -- milliseconds
}

local function game_entry_create(entry_number, entry_data)
    local obj = {}
    obj.entry_data = entry_data
    obj.size = ENTRY_SIZE

    function obj:step(xy)
        local bbox = bbox_create(xy, self.size)
        if bbox_left_clicked(bbox) then
            print("Entry " .. entry_number .. " was clicked.")
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
    for i=1,#entries do
        obj:add_instance(game_entry_create(i, entries[i]))
    end
    return obj
end

local function draw_in_box(font, bbox, origin, offset, ...)
    return draw_colored_parts(font, origin, origin_aligned(bbox, origin, {0,0}, offset), ...)
end

local function player_list_string(player_list, max_chars) 
    local str = (", "):join(player_list)
    if #str > max_chars - 3 then 
        str = str:sub(1, max_chars - 3) .. "..." 
    end
    return str
end

function game_entry_draw(number, entry, bbox)
    local game_number_color = vector_interpolate(COL_YELLOW, COL_DARK_GRAY, (number-1) / 10)
    draw_in_box(alt_font, bbox, LEFT_CENTER, {-14,0}, {game_number_color, number})
    draw_in_box(small_font, bbox, LEFT_TOP, {0,18}, {COL_WHITE, "Host: "}, {COL_PALE_RED, entry.host})
    draw_in_box(small_font, bbox, LEFT_TOP, {0,3}, {COL_WHITE, "Players: "}, {COL_MUTED_GREEN, player_list_string(entry.players, PLAYER_LIST_MAX_CHARS)} )
    draw_in_box(small_font, bbox, RIGHT_TOP, {-5,20},  {COL_LIGHT_GRAY, os.date("%I:%M%p", entry.creationTime)} )

    draw_rectangle_outline( bbox_mouse_over(bbox) and COL_WHITE or COL_GRAY, bbox, 1 )
end

function lobby_menu_create(on_back_click) 
    local menu = InstanceBox.create{ size = vector_min(Display.display_size, {800, 600}) }
    menu.entry_list = game_entry_list_create()
    menu:add_instance(menu.entry_list, LEFT_TOP, {20, 210})
    menu:add_instance(Sprite.image_create(logo_path), LEFT_TOP, {10,10})
    menu:add_instance(TextLabel.create(alt_font, "Open Games"), LEFT_CENTER, {20,-52})

    local w, h = unpack(menu.size)
    local right_side = InstanceBox.create{ size = {w/2, h} }
    right_side:add_instance(
        name_field_create { 
            label_text = "Your name: ",
            size = SETTINGS_BOX_SIZE, font = small_font, max_chars = SETTINGS_BOX_MAX_CHARS
        }, CENTER, {0, -13}
    )
    right_side:add_instance(
        text_button_create( "Back", on_back_click, {font = large_font} ), 
        CENTER, {0, 100}
    )

    menu:add_instance(right_side, RIGHT_CENTER)

    return menu
end

function main()
    Display.initialize("Lanarts Example", {640, 480}, false)
--    Display.initialize("Lanarts Example", {1200, 900}, false)
    local menu_frame = InstanceBox.create { size = Display.display_size }
    local lobby_menu = lobby_menu_create()
    menu_frame:add_instance(lobby_menu, CENTER)

    local function query_game_list()
        local first_time = true
        while true do
            local timer = timer_create()
            while not first_time and timer:get_milliseconds() < configuration.update_frequency do
                coroutine.yield()
            end
            local response = Lobby.query_game_list()
            first_time = false
            pretty_print(response)
            lobby_menu.entry_list:clear()
            for i=1,#response.gameList do
                lobby_menu.entry_list:add_instance(game_entry_create(i, response.gameList[i]))
            end
        end
    end

    Task.create(query_game_list)

    local state = {menu = menu_frame}

    while Game.input_capture() do
        state.menu:step({0,0})

        Display.draw_start()
        state.menu:draw({0,0})
        Display.draw_finish()

        Task.run_all()
        io.flush()

        Game.wait(5)
    end
end

