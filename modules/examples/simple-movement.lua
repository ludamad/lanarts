local Display = import "core.Display"
local Keys = import "core.Keyboard"

local GameObject = newtype()

function GameObject:init(xy, speed, radius)
    self.xy = xy
    self.speed = speed
    self.radius = radius
end

function GameObject:step()
    local x, y = unpack(self.xy)

    if Keys.key_held(Keys.LEFT)     then x = x - self.speed end
    if Keys.key_held(Keys.RIGHT)    then x = x + self.speed end

    if Keys.key_held(Keys.UP)       then y = y - self.speed end
    if Keys.key_held(Keys.DOWN)     then y = y + self.speed end

    self.xy = {x, y}
end

function GameObject:draw()
    Display.draw_circle(COL_WHITE, self.xy, self.radius)
end

local WINDOW_SIZE = { 640, 480 }
Display.initialize("Lanarts Example", WINDOW_SIZE, --[[Not fullscreen]] false)
local obj = GameObject.create({320, 240}, 8, 32)

while Game.input_capture() and not Keys.key_pressed(Keys.ESCAPE) do obj:step()
    Display.draw_start()
    obj:draw()
    Display.draw_finish()
    Game.wait(10)
end
