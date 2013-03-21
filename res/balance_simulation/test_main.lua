require "utils"
require "battle_formulas"

local FONT = font_cached_load(settings.menu_font, 10)

DEBUG_LAYOUTS = false

local function project(point, old_area, new_area)
    local x,y = unpack(point)
    x = (x - old_area[1]) / (old_area[3] - old_area[1])
    y = (old_area[4] - y + old_area[2]) / (old_area[4] - old_area[2])

    x = x * (new_area[3] - new_area[1]) + new_area[1]
    y = y * (new_area[4] - new_area[2]) + new_area[2]

    return {x, y}
end

local function graph_line(func, range, color, draw_area)

    local logical_area = {range[1], 0, range[2], 100}
    local points = {}

    for i = range[1], range[2] do
        points[i] = {i, func(i)}
        points[i] = project( points[i], logical_area, draw_area )
    end

    for i = range[1], range[2] - 1 do
        draw_line(color, points[i], points[i+1], 1)
    end

    local mouse_point = project( mouse_xy, draw_area, logical_area)

    for point in values( { {5,5}, {5, 95}, {50,50}, {95, 5}, {95, 95}, mouse_point } ) do
        FONT:draw(
            COL_BLACK,
             project( point, logical_area, draw_area ), 
            "(" .. math.floor(point[1]) .. ", " .. math.floor(point[2]) .. ")" 
        )
    end
end

local function draw_function_create()
   local w,h = unpack(display.window_size)
   
   local function draw_function()
        draw_rectangle(COL_WHITE, {0,0, unpack(display.window_size)} )
        for F in values(functions_to_plot) do
            graph_line( 
                F[1],
                {0, 100}, -- range
                F[2],
                {0, 0, w, h} 
            )
        end
   end

   return draw_function
end

local function menu_loop()
    local draw_function = draw_function_create()

    while Game.input_capture() and not key_pressed(keys.ESCAPE) do
        if key_pressed(keys.F9) then
            -- note, globals are usually protected against being changed
            -- but a bypass is allowed for cases where it must be done
            setglobal("DEBUG_LAYOUTS", not DEBUG_LAYOUTS) -- flip on/off
        end

        display.draw_start()
        draw_function()
        display.draw_finish()

        io.flush()
        Game.wait(10)
    end

    return false -- User has quit the game
end


function main() 
    local WINDOW_SIZE = { 800, 800 }

    display.initialize("Lanarts Example", WINDOW_SIZE, false)
    menu_loop()
end