-- Routines encapsulate task decisions for a monster.
-- The monster has a task-list, which can be interrupted by one of the routines.
--
-- Routines come in mutable, immutable, and shared state flavours.
-- A notable shared-state routine is the RVO collision avoidance routine. 

local ColAvoid = import "core.CollisionAvoidance"
local PathFinding = import "core.PathFinding"
local GameMap = import "@GameMap"
local Mouse = import "core.Mouse"

local M = nilprotect {} -- Submodule

local cgroup = ColAvoid.collision_group_create()
local path_buffer = PathFinding.astar_buffer_create()

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
    for reach in values{{radius,0}, {0, radius}} do
        if GameMap.line_tile_check(gmap, vsub(from_xy,reach), vsub(to_xy,reach)) then
            return true
        elseif GameMap.line_tile_check(gmap, vadd(from_xy,reach), vadd(to_xy,reach)) then
            return true
        end
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
        if not node then 
            self.path = nil
            break
        end
        local dx,dy=node[1]-x,node[2]-y
        local xspeed, yspeed= math.min(self.speed, math.abs(dx)), math.min(self.speed, math.abs(dy))
        local dist, dist_needed = math.sqrt(dx*dx+dy*dy), (is_final_node and 0 or 16)

        local straight_line_to_next_node = false
        if not is_final_node and not fat_line_check(self.radius, self.xy, self.path[idx+1]) then
            straight_line_to_next_node = true
        elseif dist >= 32 and fat_line_check(self.radius, self.xy, self.path[idx]) then
            self.path_idx = math.max(0,self.path_idx - 1)
        end

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

M.ActionTask = newtype()
function M.ActionTask:init(object)
end

M.GoToTask = newtype()
function M.GoToTask:init(xy)
end

M.FollowRoutine = newtype()
function M.FollowRoutine:run()
    
end

M.ActionRoutine = newtype()
function M.ActionRoutine:run()
    
end

return M