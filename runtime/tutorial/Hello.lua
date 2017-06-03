-- Includes
-- Each call to 'require' includes a component of either
-- the Lua standard library or the Lanarts engine.

-- The core.Display component handles everything necessary for
-- 2D graphics (uses internal Lanarts library called ldraw, builds on OpenGL and SDL):
local Display = require "core.Display"

-- The constants 
local WIDTH, HEIGHT = 640, 480

Display.initialize("Hello", {WIDTH, HEIGHT}, false)

-- Load a font from the Lanarts   
local font = Display.font_load("fonts/Gudea-Regular.ttf", --[[Size]] 60)

local function draw() 
    -- We want white, centered text
    local style = {colour = COL_WHITE, origin = Display.CENTER}
    -- In the center of the screen
    local position = {WIDTH/2, HEIGHT/2}
    -- Draw with our font object
    font:draw(style, position, "Hello World")
end

-- Set up the loop that queries user input and sets up drawing:
Display.draw_loop(draw)
