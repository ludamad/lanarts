local M = nilprotect {} -- Submodule

--------------------------------------------------------------------------------
-- StatML schemas
--------------------------------------------------------------------------------

local StatML = import "statml.StatML"

local Pos = StatML.object_parsedef("Pos", [[
    x,y :float
]])

local Actor = StatML.class_parsedef("Actor", [[
    Pos as xy
]])

local Ship = StatML.class_parsedef("Ship", [[
    extend Actor
    speed :float
]])

local World = StatML.object_parsedef("World", [[
    actors :list
]])

function M.main()
    StatML.load_and_parse_file(path_resolve "demoshoot.yaml")
    local actor_set = StatML.get_instance_set "Actor"
    local world = World.create(table.clone(actor_set.list))
    world:simulate()
end

--------------------------------------------------------------------------------
-- Class implementations
--------------------------------------------------------------------------------

local Keys = import "core.Keyboard"
local GameState = import "core.GameState"
local Display = import "core.Display"

local function from_pos(p) return {p.x,p.y} end
function Actor:on_draw()
    Display.draw_circle(COL_WHITE, from_pos(self.xy), self.radius)
end
Actor.on_step = do_nothing

function Ship:on_step()
    local x, y = unpack(self.xy)

    if Keys.key_held(Keys.LEFT)     then x = x - self.speed end
    if Keys.key_held(Keys.RIGHT)    then x = x + self.speed end

    if Keys.key_held(Keys.UP)       then y = y - self.speed end
    if Keys.key_held(Keys.DOWN)     then y = y + self.speed end

    self.xy = {x, y}
end

function World:on_draw()
    for _,actor in ipairs(self.list) do actor:on_draw() end 
end

function World:on_step()
    for _,actor in ipairs(self.list) do actor:on_step() end 
end

function World:simulate()
    local WINDOW_SIZE = { 640, 480 }
    Display.initialize("Lanarts Example", WINDOW_SIZE, --[[Not fullscreen]] false)
    
    while GameState.input_capture() and not Keys.key_pressed(Keys.ESCAPE) do 
        Display.draw_start()

        Display.draw_finish()
        GameState.wait(10)
    end
end

return M