local GameMap = import "core.Map"
local PathFinding = import "core.PathFinding"

local M = nilprotect {} -- Submodule

-- Search two lines at two lines of a circle, combined with two circle checks.
-- Emulates a check of eg a missile's trajectory.
function M.radius_and_line_tile_check(map, radius, from_xy, to_xy)
    local vadd,vsub = vector_add, vector_subtract
    local reach = {radius, 0}

    -- Circle checks at the ends of the line
    if GameMap.radius_tile_check(map, from_xy, radius) then return true end
    if GameMap.radius_tile_check(map, to_xy, radius) then return true end

    -- Two lines from either sides of the 'end circles'
    for reach in values{{radius,0}, {0, radius}} do
        if GameMap.line_tile_check(map, vsub(from_xy,reach), vsub(to_xy,reach)) then
            return true
        elseif GameMap.line_tile_check(map, vadd(from_xy,reach), vadd(to_xy,reach)) then
            return true
        end
    end
    -- Refuted
    return false
end

local Path = newtype()

function Path:init(path) 
	self.path = path
	self.node_index = 1
end

M.path_create = Path.create
function M.path_calculate(path_buffer, map, from_xy, to_xy)
    path_buffer:calculate_path(map, to_tilexy(from_xy), to_tilexy(to_xy))
end

-- Returns next dx,dy on the path
function Path:follow_path(obj)
    local x, y = unpack(obj.xy)
    while obj.path do
        local idx = self.node_index
        local node, is_final_node = self.path[idx], (idx == #self.path)
        if not node then 
            obj.path = nil
            break
        end
        local dx,dy=node[1]-x,node[2]-y
        local xspeed, yspeed= math.min(obj.speed, math.abs(dx)), math.min(obj.speed, math.abs(dy))
        local dist, dist_needed = math.sqrt(dx*dx+dy*dy), (is_final_node and 0 or 16)

        local straight_line_to_next_node = false
        if not is_final_node and not M.radius_and_line_tile_check(obj.map, obj.radius, obj.xy, self.path[idx+1]) then
            straight_line_to_next_node = true
        elseif dist >= 32 and M.radius_and_line_tile_check(obj.map, obj.radius, obj.xy, self.path[idx]) then
            obj.path_idx = math.max(0,obj.path_idx - 1)
        end

        if straight_line_to_next_node or dist < dist_needed then
            self.node_index = self.node_index + 1
            if self.node_index > #self.path then self.path = nil end
            -- Continue onto next node
        else
            return {signof(dx)*xspeed, signof(dy)*yspeed}
        end
    end

    return {0,0}
end

return M
