-- Includes
-- Each call to 'require' includes a component of either
-- the Lua standard library or the Lanarts engine.

-- The core.Display component handles everything necessary for
-- 2D graphics (uses internal Lanarts library called ldraw, builds on OpenGL and SDL):
local Display = require "core.Display"

-- The constants 

local TILE_WIDTH, TILE_HEIGHT = 64, 64
local WIDTH, HEIGHT = TILE_WIDTH * 10, TILE_HEIGHT * 10 -- 8 tiles, 1 padding around

Display.initialize("Checkerboard", {WIDTH, HEIGHT}, false)

-- Load a font from the Lanarts   
local font = Display.font_load("fonts/Gudea-Regular.ttf", --[[Size]] 60)

local function draw_checkerboard()
    -- Iterate over (x, y) values, from 1 to 8
    for x=1,8 do
        for y=1,8 do
            local colour = COL_WHITE
            -- Does x + y have an even sum?
            local even_sum = ((x + y) % 2 == 0)
            if even_sum then -- Causes a checkerboard pattern of white/blue
                colour = COL_BLUE
            end
            -- Coordinates of rectangle
            local x1, y1 = x * TILE_WIDTH, y * TILE_HEIGHT
            local x2, y2 = (x+1) * TILE_WIDTH, (y+1) * TILE_HEIGHT
            Display.draw_rectangle(colour, {x1, y1, x2, y2})
        end
    end
end

local function draw()
    draw_checkerboard()
end

-- Set up the loop that queries user input and sets up drawing:
Display.draw_loop(draw)
