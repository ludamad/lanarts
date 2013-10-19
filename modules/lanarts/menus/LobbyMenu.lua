local Display = import "core.Display"

local InstanceBox = import "core.ui.InstanceBox"
local InstanceLine = import "core.ui.InstanceLine"
local Sprite = import "core.ui.Sprite"
local TextLabel = import "core.ui.TextLabel"

local lobby = import "core.networking.Lobby"
local tasks = import "core.networking.Tasks"

local game_entry_draw
local logo_path = "modules/lanarts/LANARTS.png"
local small_font = font_cached_load(settings.font, 10)
local large_font = font_cached_load(settings.font, 20)
local alt_font = font_cached_load("modules/core/fonts/MateSC-Regular.ttf", 14)

local SETTINGS_BOX_MAX_CHARS = 18
local SETTINGS_BOX_SIZE = {180, 34}

local ENTRY_SIZE = {350, 40}
local ENTRY_SPACING = 45
local PLAYER_LIST_MAX_CHARS = 50

local session_info = {
    username = nil,
    sessionId = nil
}

local function login_if_needed()
     if session_info.username ~= settings.username then
         local credentials = lobby.guest_login(settings.username)
         session_info.username = settings.username
         session_info.sessionId = credentials.sessionId
    end
end

local function join_game_task_create(entry_data)
    tasks.create(function()
        login_if_needed()
        pretty_print( lobby.join_game(session_info.username, session_info.sessionId, entry_data.id) )
    end)
end

-- A component that starts by displaying a loading animation until 'replace' is called
local function loading_box_create(size)
    local obj = InstanceBox.create( {size=size} )
    local loading_animation = Display.animation_create( Display.image_split(image_cached_load "modules/lanarts/menus/loading_64x64.png", {64, 64}), 0.1 )
    local contents = Sprite.create(loading_animation, {color=with_alpha(COL_WHITE, 0.25)} )
    obj:add_instance(contents, Display.CENTER_TOP, {0,50})
    -- Called when component has loaded
    function obj:replace(newcontents, origin)
        if contents then self:remove(contents) end
        contents = newcontents
        self:add_instance(newcontents, origin)
    end
    return obj
end

-- For reference, this is how the received table looks
--local sample_lobby_entry = { 
--    host = "ludamad",
--    creationTime = 1010010,
--    id = "51b002b8e1382367f2000003",
--    players = {
--        "ciribot",
--       "ludamad",
--    }
--}

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
            join_game_task_create(obj.entry_data)
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
local function game_entry_list_create(entries)
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
    draw_in_box(alt_font, bbox, Display.LEFT_CENTER, {-14,0}, {game_number_color, number})
    draw_in_box(small_font, bbox, Display.LEFT_TOP, {0,18}, {COL_WHITE, "Host: "}, {COL_PALE_RED, entry.host})
    draw_in_box(small_font, bbox, Display.LEFT_TOP, {0,3}, {COL_WHITE, "Players: "}, {COL_MUTED_GREEN, player_list_string(entry.players, PLAYER_LIST_MAX_CHARS)} )    
    -- XXX: Find out why this returns nil on windows
    local date = os.date("%I:%M%p", entry.creationTime)
    if date ~= nil then
        draw_in_box(small_font, bbox, RIGHT_TOP, {-5,20},  {COL_LIGHT_GRAY, date } )
    end

    Display.draw_rectangle_outline( bbox_mouse_over(bbox) and COL_WHITE or COL_GRAY, bbox, 1 )
end

local function game_query_task_create(menu)
   local function query_game_list()
        local first_time = true
        while true do
            local timer = timer_create()
            while not first_time and timer:get_milliseconds() < configuration.update_frequency do
                coroutine.yield()
                -- Have we been signalled to finish ?
                if not menu.alive then 
                    return 
                end
            end
            local response = lobby.query_game_list()
            first_time = false
            menu.entry_list:replace( game_entry_list_create(response.gameList), Display.LEFT_TOP, {0,0} )
        end
    end
    tasks.create(query_game_list)
end

local function lobby_menu_create(on_back_click) 
    local menu = InstanceBox.create{ size = vector_min(Display.display_size, {800, 600}) }
    -- The game query task will finish once menu.alive is false
    menu.alive = true

    menu.entry_list = loading_box_create({350, 400})
    menu:add_instance(menu.entry_list, Display.LEFT_TOP, {20, 200})
    menu:add_instance(Sprite.image_create(logo_path), Display.LEFT_TOP, {10,10})
    menu:add_instance(TextLabel.create(alt_font, "Open Games"), Display.LEFT_TOP, {20,175})

    local w, h = unpack(menu.size)
    local right_side = InstanceBox.create{ size = {w/2, h} }

    right_side:add_instance(
        name_field_create { 
            label_text = "Your name: ",
            size = SETTINGS_BOX_SIZE, font = small_font, max_chars = SETTINGS_BOX_MAX_CHARS
        }, Display.CENTER, {0, -13}
    )

    game_query_task_create(menu)
    right_side:add_instance(
        text_button_create( "Back", 
            function (...) 
                menu.alive = false -- Signal the task to finish
                return on_back_click(...) 
            end, 
            {font = large_font} ), 
        Display.CENTER, {0, 100}
    )

    menu:add_instance(right_side, RIGHT_CENTER)

    return menu
end

-- submodule
return {
    create = lobby_menu_create
}
