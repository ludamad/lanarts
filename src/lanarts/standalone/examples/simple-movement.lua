
GameObject = newtype()

function GameObject:init(xy, speed, radius)
    self.xy = xy
    self.speed = speed
    self.radius = radius
end

function GameObject:step()
    local x, y = unpack(self.xy)

    if key_held(keys.LEFT)     then x = x - self.speed end
    if key_held(keys.RIGHT)    then x = x + self.speed end

    if key_held(keys.UP)       then y = y - self.speed end
    if key_held(keys.DOWN)     then y = y + self.speed end

    self.xy = {x, y}
end

function GameObject:draw()
    draw_circle(COL_WHITE, self.xy, self.radius)
end

function main()
    local WINDOW_SIZE = { 640, 480 }
    Display.initialize("Lanarts Example", WINDOW_SIZE, --[[Not fullscreen]] false)
    local obj = GameObject.create({320, 240}, 8, 32)

    while Game.input_capture() and not key_pressed(keys.ESCAPE) do
        obj:step()
        Display.draw_start()
        obj:draw()
        Display.draw_finish()
        Game.wait(10)
    end
end
