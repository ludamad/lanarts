local Map = import "core.Map"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local ActionContext = import "@ActionContext"

local M = nilprotect {} -- Submodule

local Base = newtype()
M.ActionResolverBase = Base

function Base:init(cgroup)
	self.cgroup = cgroup -- Collision avoidance group
	self:_reset()
end

function Base:_reset()
	self.preferred_velocity = {0, 0}
	-- Use false as a 'safe' nil-value
	self.action, self.target = false, false
	self.close_to_wall = false
end

function Base:_set_action(action, target)
    self.action, self.target = (action or false), (target or false)
end

-- CollisionAvoidance expects 'radius, speed, xy, preferred_velocity' members.
-- For convenience, we make it so we can pass 'self' directly.
function Base:_copy_from_object(obj)
    self.radius, self.xy = obj.radius, obj.xy
    self.speed = obj:stat_context().derived.movement_speed
end

function Base:_copy_from_colavoid(obj)
    self.cgroup:object_copy_xy(self.sim_id, obj)
end

function Base:_copy_to_colavoid(obj)
    self:_copy_from_object(obj)
    self.cgroup:update_object(self.sim_id, self)
end

function Base:on_object_init(obj)
    self:_copy_from_object(obj)
	self.sim_id = self.cgroup:add_object(self)
end

function Base:on_object_deinit(obj)
	self.cgroup:remove_object(self.sim_id)
end

-- Assumes a cgroup:step() between on_prestep and use_resolved_action 
function Base:use_resolved_action(obj)
    -- Do action before moving, invalidates less actions
    if self.action and ActionContext.can_use_action(self.action, self.target or nil) then
        ActionContext.use_action(self.action, self.target or nil)
    end

    local close_to_wall = Map.radius_tile_check(obj.map, obj.xy, obj.radius + 10)
    if close_to_wall then
        local new_xy = ObjectUtils.find_free_position(obj, self.preferred_velocity)
        obj.xy = new_xy or obj.xy
    else
        self:_copy_from_colavoid(obj)
    end

end

function Base:on_prestep(obj)
    self:_reset()
    if getmetatable(self).resolve_action then self:resolve_action(obj) end
    self:_copy_to_colavoid(obj)
end

local AI = newtype {parent = Base}
M.AIActionResolver = AI

function AI:init(cgroup)
    Base.init(self, cgroup)
    self.path = false
end

function AI:resolve_action(obj)
--    print("STARTIN")
--    profile(function()
--        for i=1,10000 do
--            obj.hostile_something = ObjectUtils.find_closest_hostile(obj)
--        end
--    end)
--    print("ENMDO")
--    io.read "*l"
    local hostile = ObjectUtils.find_closest_hostile(obj)
    if not hostile then return nil end -- No target
    
    local S,H = obj:stat_context(), hostile:stat_context()
    local action_context = obj:weapon_action_context()

    if ActionContext.can_use_action(action_context, H) then
        self.action = action_context
        self.target = H
    end

    local dx,dy = unpack(vector_subtract(hostile.xy, obj.xy))
    local move_speed = obj:stat_context().derived.movement_speed
    local xspeed, yspeed= math.min(move_speed, math.abs(dx)), math.min(move_speed, math.abs(dy))
    if vector_distance(hostile.xy, obj.xy) > hostile.radius + obj.radius then
        self.preferred_velocity[1] = math.sign_of(dx) * xspeed
        self.preferred_velocity[2] = math.sign_of(dy) * yspeed
    end
    -- For drawing purposes:
    obj.direction = vector_to_direction(self.preferred_velocity)
end

return M
