local GameObject = import "core.GameObject"
local Map = import "core.Map"
local ObjectUtils = import ".ObjectUtils"

local M = {} -- Submodule

-- PROJECTILE BASE CLASS

M.ProjectileBase = GameObject.type_create()
local Base = M.ProjectileBase
Base.on_tile_collide, Base.on_object_collide = do_nothing, do_nothing
Base.on_draw = do_nothing
function Base:on_step()
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

-- LINEAR PROJECTILE BASE CLASS

M.LinearProjectileBase = GameObject.type_create(Base)
local LinearBase = M.LinearProjectileBase

function LinearBase:init(args)
    LinearBase.parent_init(self, args.xy, args.radius, true, args.depth)
    self.velocity = assert(args.velocity)
    self.direction = vector_to_direction(args.velocity)
    self.range_left = self.range_left or 250
end

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

return M