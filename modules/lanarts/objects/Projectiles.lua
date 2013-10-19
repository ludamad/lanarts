local GameObject = import "core.GameObject"
local Map = import "core.Map"
local ObjectUtils = import ".ObjectUtils"

local M = {} -- Submodule

M.PROJECTILE_TRAIT = "PROJECTILE_TRAIT"

-- PROJECTILE BASE CLASS

local Base = ObjectUtils.type_create()
M.ProjectileBase = Base

local function projectile_wrapper_on_step(self)
    local tile_xy = Map.object_tile_check(self)
    if tile_xy then
        ObjectUtils.object_callback(self, "on_tile_collide", assert(tile_xy))
    end

    local collisions = Map.object_collision_check(self)
    for obj in values(collisions) do
        if self.destroyed then return end
        ObjectUtils.object_callback(self, "on_object_collide", assert(obj))
    end

    if self.destroyed then return end
    self:_on_step()
end

-- Manditory arguments:
-- xy, radius, on_step, on_draw, on_tile_collide, on_object_collide, do_init
-- Everything else is extra.
function Base.create(args)
    args.traits = args.traits or {}
    table.insert(args.traits, M.PROJECTILE_TRAIT)
    -- Wrap supplied on_step
    args._on_step = args.on_step or args.type.on_step
    -- Make sure our on_step is called
    args.on_step = projectile_wrapper_on_step

    return Base.base_create(args)
end

-- LINEAR PROJECTILE

local LinearBase = ObjectUtils.type_create(Base)
M.LinearProjectileBase = LinearBase

LinearBase.on_tile_collide = GameObject.destroy

function LinearBase:on_step()
    if self.range_left <= 0 then
        GameObject.destroy(self)
    end
    local vx, vy = unpack(self.velocity)
    self.x = self.x + vx
    self.y = self.y + vy
    self.range_left = self.range_left - math.sqrt(vx*vx+vy*vy)
end

function LinearBase.create(args)
    assert(args.velocity and args.radius)
    args.direction = vector_to_direction(args.velocity)
    args.range_left = args.range_left or 250
    return LinearBase.base_create(args)
end

-- HELPER FUNCTIONS

function M.is_projectile(object)
    return table.contains(object.traits, M.PROJECTILE_TRAIT)
end

return M