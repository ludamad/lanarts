local Display = import "core.Display"

-- Necessary to load images:
Display.initialize("Lanarts Example", {640,480}, --[[Not fullscreen]] false)

local Keys = import "core.Keyboard"
local Mouse = import "core.Mouse"
local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"
local MapGen = import "core.MapGeneration"
local ColAvoid = import "core.CollisionAvoidance"

-- Include parts of lanarts, for convenience
local MapUtils = import "lanarts.maps.MapUtils"
local TileSets = import "lanarts.tiles.Tilesets"

local function signof(v)
    if v > 0 then return 1 end
    if v == 0 then return 0 end
    return -1
end

local cgroup = ColAvoid.collision_group_create()

local Obj = {}
function Obj:step()
    local close_to_wall = (GameMap.radius_tile_check(self.xy, self.radius + 5))
    local prev_xy = self.xy
    if not close_to_wall then
        cgroup:object_copy_xy(self.sim_id, self)
    end
    local x, y = unpack(self.xy)
    if Mouse.mouse_left_held then
        local dx,dy = signof(Mouse.mouse_xy[1] - x), signof(Mouse.mouse_xy[2] - y)
        self.preferred_velocity = {dx*self.speed, dy*self.speed}
    else
        self.preferred_velocity = {0,0}
    end
    if close_to_wall then
        local dx,dy = unpack(self.preferred_velocity)
        local new_xy = {self.x+dx,self.y+dy}
        if not GameMap.radius_tile_check(new_xy, self.radius) then
            self.xy = new_xy
        end
    end
    cgroup:update_object(self.sim_id, self)
end

function Obj:draw()
    Display.draw_circle(COL_WHITE, self.xy, self.radius)
end

local map = MapUtils.area_template_to_map("LanartsExampleLevel", path_resolve "simple-swarm-map.txt", 0, {
    ['.'] =  { add = MapGen.FLAG_SEETHROUGH, content = TileSets.pebble.floor }, 
    ['x'] =  { add = {MapGen.FLAG_SEETHROUGH,MapGen.FLAG_SOLID}, content = TileSets.pebble.wall }
})
local gmap = GameMap.create { map = map }

for i=1,10 do
    local tile_xy = MapUtils.random_square(map)
    local obj = GameObject.object_create {
        map=gmap, xy = {tile_xy[1]*32+16, tile_xy[2]*32+16},
        radius = 12, on_step = Obj.step, on_draw = Obj.draw
    }
    obj.speed = 4
    obj.radius = 15
    obj.sim_id = cgroup:add_object(obj)
end

while Game.input_capture() and not Keys.key_pressed(Keys.ESCAPE) do
    GameMap.map_step {map=gmap}
    cgroup:step()
    Display.draw_start()
    GameMap.map_draw {map=gmap}
    Display.draw_finish()
    Game.wait(10)
end
