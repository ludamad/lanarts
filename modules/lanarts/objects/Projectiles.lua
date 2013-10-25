local GameObject = import "core.GameObject"
local Map = import "core.Map"
local ObjectUtils = import ".ObjectUtils"

local M = {} -- Submodule

M.PROJECTILE_TRAIT = "PROJECTILE_TRAIT"

-- PROJECTILE BASE CLASS

M.ProjectileBase = GameObject.type_create()
local Base = M.ProjectileBase

Base.on_tile_collide, Base.on_object_collide = do_nothing, do_nothing
Base.on_draw = do_nothing

function Base:on_step(self)
    -- Tile checks:
    local tile_xy = Map.object_tile_check(self)
    if tile_xy then self:on_tile_collide(tile_xy) end
    -- Object collision checks:
    local collisions = Map.object_collision_check(self)
    for _, obj in ipairs(collisions) do
        if self.destroyed then return end
        self:on_object_collide(obj)
    end
    return (not self.destroyed)
end

function Base:init(args)
    Base.parent_init(self, args)
    self.traits = args.traits or {}
    table.insert(self.traits, M.PROJECTILE_TRAIT)
end

-- LINEAR PROJECTILE

M.LinearProjectileBase = GameObject.type_create(Base)
local LinearBase = M.LinearProjectileBase

LinearBase.on_tile_collide = GameObject.destroy

function LinearBase:on_step()
    if not LinearBase.parent_on_step(self) then return end
    if self.range_left <= 0 then
        GameObject.destroy(self)
        return
    end
    local vx, vy = self.velocity[1],self.velocity[2]
    self.x = self.x + vx
    self.y = self.y + vy
    self.range_left = self.range_left - math.sqrt(vx*vx+vy*vy)
end

function LinearBase:init(args)
    self.parent_init(self, args)
    self.velocity, self.radius = assert(args.velocity), assert(args.radius)
    self.direction = vector_to_direction(args.velocity)
    self.range_left = self.range_left or 250
end

-- HELPER FUNCTIONS

function M.is_projectile(object)
    return table.contains(object.traits, M.PROJECTILE_TRAIT)
end

return M