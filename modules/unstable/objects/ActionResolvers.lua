local GameMap = import "core.GameMap"
local ObjectUtils = import "lanarts.objects.ObjectUtils"

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
	self.action, self.target, self.source = false, false, false
	self.close_to_wall = false
end

-- CollisionAvoidance expects 'radius, speed, xy, preferred_velocity' members.
-- For convenience, we make it so we can pass 'self' directly.
function Base:_copy_from_object(obj)
    self.radius, self.xy = obj.radius, obj.xy
    self.speed = obj:stat_context().derived.movement_speed
end

function Base:_copy_from_colavoid(obj)
    self.cgroup:object_copy_xy(self.sim_id, self)
end

function Base:_copy_to_colavoid(obj)
    self.cgroup:object_update(self.sim_id, self)
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
    local close_to_wall = GameMap.radius_tile_check(obj.xy, obj.radius + 10)
    if close_to_wall then
        local new_xy = ObjectUtils.find_free_position(obj, self.preferred_velocity)
        self.xy = new_xy or self.xy
    else 
        self:_copy_from_colavoid(obj)
    end
    if self.action then
        obj:use_action(self.action, self.target or nil, self.source or nil)
    end
end

function Base:on_prestep(obj)
    self:_reset()
end

local AI = newtype {parent = Base}
M.AIActionResolver = AI

function AI:init(cgroup, action_chooser)
    Base.init(self, cgroup)
    assert(self.resolve_movement)
    self.action_chooser = action_chooser
end

function AI:on_prestep(obj)
    Base.on_prestep(obj)

    local action, target, source = self:action_chooser(obj)
    self.action, self.target = action or false, target or false
    self.source = source or false

    self:_copy_to_colavoid(obj)
end

return M