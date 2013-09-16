local Display = import "core.Display"

-- Necessary to load images:
Display.initialize("Lanarts Example", {640,640}, --[[Not fullscreen]] false)

local Keys = import "core.Keyboard"
local Mouse = import "core.Mouse"
local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"
local MapGen = import "core.MapGeneration"
local ColAvoid = import "core.CollisionAvoidance"
local PathFinding = import "core.PathFinding"

-- Include parts of lanarts, for convenience
local MapUtils = import "lanarts.maps.MapUtils"
local TileSets = import "lanarts.tiles.Tilesets"

local function signof(v)
    if v > 0 then return 1 end
    if v == 0 then return 0 end
    return -1
end

local cgroup = ColAvoid.collision_group_create()
local path_buffer = PathFinding.astar_buffer_create()

local function to_tilexy(xy)
    return {math.floor(xy[1]/32), math.floor(xy[2]/32)}
end
local function to_worldxy(xy)
    return {xy[1]*32+16, xy[2]*32+16}
end

local map = MapUtils.area_template_to_map("LanartsExampleLevel", path_resolve "simple-swarm-map.txt", 0, {
    ['.'] =  { add = MapGen.FLAG_SEETHROUGH, content = TileSets.pebble.floor },
    ['x'] =  { add = {MapGen.FLAG_SEETHROUGH,MapGen.FLAG_SOLID}, content = TileSets.pebble.wall }
})

local gmap = GameMap.create { map = map }

local function path_create(xy)
    local mxy = to_tilexy(Mouse.mouse_xy)
    return path_buffer:calculate_path(gmap, to_tilexy(xy), mxy)
end

-- Search two lines at two lines of a circle, combined with two circle checks.
-- Emulates a check of eg a missile's trajectory.
local function fat_line_check(radius, from_xy, to_xy)
    local vadd,vsub = vector_add, vector_subtract
    local reach = {radius, 0}

    -- Circle checks at the ends of the line
    if GameMap.radius_tile_check(from_xy, radius) then return true end
    if GameMap.radius_tile_check(to_xy, radius) then return true end

    -- Two lines from either sides of the 'end circles'
    if GameMap.line_tile_check(gmap, vsub(from_xy,reach), vsub(to_xy,reach)) then
        return true
    end
    if GameMap.line_tile_check(gmap, vadd(from_xy,reach), vadd(to_xy,reach)) then
        return true
    end
    -- Refuted
    return false
end

local function follow_path(self)
    self.preferred_velocity = {0,0}
    local x, y = unpack(self.xy)
    while self.path do
        local idx = self.path_idx
        local node, is_final_node = self.path[idx], (idx == #self.path)
        local dx,dy=node[1]-x,node[2]-y
        local xspeed, yspeed= math.min(self.speed, math.abs(dx)), math.min(self.speed, math.abs(dy))
        local dist, dist_needed = math.sqrt(dx*dx+dy*dy), (is_final_node and 0 or 16)

        local straight_line_to_next_node = false
--        if not is_final_node and not fat_line_check(self.radius, self.xy, self.path[idx+1]) then
--            straight_line_to_next_node = true
--        end

        if straight_line_to_next_node or dist < dist_needed then
            self.path_idx = self.path_idx + 1
            if self.path_idx > #self.path then self.path = nil end
            -- Continue onto next node
        else
            self.preferred_velocity = {signof(dx)*xspeed, signof(dy)*yspeed}
            break -- Done
        end
    end
end

local Obj = {}
function Obj:step()
    local close_to_wall = (GameMap.radius_tile_check(self.xy, self.radius + 10))
    local prev_xy = self.xy
    if not close_to_wall then
        cgroup:object_copy_xy(self.sim_id, self)
    end

    if Mouse.mouse_left_pressed then
        self.path = path_create(self.xy)
        self.path_idx = 1
    end

    follow_path(self)

    if close_to_wall then
        local dx,dy = unpack(self.preferred_velocity)
        local new_xy = {self.x+dx,self.y+dy}
        if not GameMap.radius_tile_check(new_xy, self.radius) then
            self.xy = new_xy
        else
            new_xy[1], new_xy[2] = self.x+dx, self.y
            if not GameMap.radius_tile_check(new_xy, self.radius) then self.xy = new_xy
            else
                new_xy[1], new_xy[2] = self.x, self.y+dy
                if not GameMap.radius_tile_check(new_xy, self.radius) then self.xy = new_xy end
            end
        end
    end
    cgroup:update_object(self.sim_id, self)
end

function Obj:draw()
    Display.draw_circle(COL_WHITE, self.xy, self.radius)
end

for i=1,10 do
    local tile_xy = MapUtils.random_square(map)
    local obj = GameObject.object_create {
        map=gmap, xy = {tile_xy[1]*32+16, tile_xy[2]*32+16},
        radius = 11, on_step = Obj.step, on_draw = Obj.draw
    }
    obj.speed = 4
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
