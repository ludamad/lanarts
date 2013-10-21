local Map = import "core.Map"
local PathFinding = import "core.PathFinding"

local M = nilprotect {} -- Submodule

-- Search two lines at two lines of a circle, combined with two circle checks.
-- Emulates a check of eg a missile's trajectory.
function M.radius_and_line_tile_check(map, radius, from_xy, to_xy)
    local vadd,vsub = vector_add, vector_subtract
    local reach = {radius, 0}

    -- Circle checks at the ends of the line
    if Map.radius_tile_check(map, from_xy, radius) then return true end
    if Map.radius_tile_check(map, to_xy, radius) then return true end

    -- Two lines from either sides of the 'end circles'
    for reach in values{{radius,0}, {0, radius}} do
        if Map.line_tile_check(map, vsub(from_xy,reach), vsub(to_xy,reach)) then
            return true
        elseif Map.line_tile_check(map, vadd(from_xy,reach), vadd(to_xy,reach)) then
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
function M.path_calculate(path_buffer, map, from_xy, to_xy, padding, --[[Optional]] imperfect_knowledge)
    local s_xy = {math.min(from_xy[1], to_xy[1]), math.min(from_xy[2], to_xy[2])}
    local e_xy = {math.max(from_xy[1], to_xy[1]), math.max(from_xy[2], to_xy[2])}
    for i,pad in ipairs(padding) do
        local pad_h1, pad_h2 = math.floor(pad/2), math.ceil(pad/2)
        s_xy[i] = s_xy[i] - pad_h1
        e_xy[i] = e_xy[i] + pad_h2
    end 
    local world_region = {s_xy[1], s_xy[2], e_xy[1], e_xy[2]}
    local path = path_buffer:calculate_path(map, to_tilexy(from_xy), to_tilexy(to_xy), world_region, imperfect_knowledge)
    return path and Path.create(path)
end

-- Returns next dx,dy on the path
function Path:follow_path(obj, speed)
    local x, y = unpack(obj.xy)
    while self.path do
        local idx = self.node_index
        local node, is_final_node = self.path[idx], (idx == #self.path)
        if not node then 
            self.path = false
            break
        end
        local dx,dy=node[1]-x,node[2]-y
        local dist, dist_needed = math.sqrt(dx*dx+dy*dy), (is_final_node and 0 or 16)

        local straight_line_to_next_node = false
        if not is_final_node and not M.radius_and_line_tile_check(obj.map, obj.radius+4, obj.xy, self.path[idx+1]) then
            straight_line_to_next_node = true
        elseif dist >= 32 and M.radius_and_line_tile_check(obj.map, obj.radius+4, obj.xy, self.path[idx]) then
            self.node_index = math.max(0,self.node_index - 1)
        end

        if straight_line_to_next_node or dist < dist_needed then
            self.node_index = self.node_index + 1
            if self.node_index > #self.path then self.path = nil end
            -- Continue onto next node
        else
            local mag = math.sqrt(dx*dx + dy*dy)
            speed = math.min(speed, mag)
            if mag == 0 then return 0, 0 end
            return dx * speed / mag, dy * speed / mag
        end
    end

    return 0,0
end

local Display = import "core.Display"
function Path:draw_path(obj)
    if self.path and self.path[self.node_index] then
        Display.draw_line(COL_BLACK, Display.to_screen_xy(obj.xy), Display.to_screen_xy(self.path[self.node_index]), 1)
    end
end

return M
