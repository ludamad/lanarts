local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"

local Animations = import ".Animations"
local ObjectUtils = import ".ObjectUtils"

local M = {} -- Submodule

M.PROJECTILE_TRAIT = "Projectile"

-- PROJECTILE BASE CLASS

local Base = ObjectUtils.type_create()
M.ProjectileBase = Base

local function projectile_wrapper_on_step(self)
    local tile_xy = GameMap.object_tile_check(self)
    if tile_xy then
        ObjectUtils.object_callback(self, "on_tile_collide", tile_xy)
    end

    local collisions = GameMap.object_collision_check(self)
    for obj in values(collisions) do
        if self.destroyed then return end
        ObjectUtils.object_callback(self, "on_object_collide", obj)
    end

    if self.destroyed then return end
    self:_on_step()
end

-- Manditory arguments:
-- xy, radius, on_step, on_draw, on_tile_collide, on_object_collide, do_init
-- Everything else is extra.
function Base._create(real_type, base_create, args)
    args.traits = {M.PROJECTILE_TRAIT}

    local object = base_create(real_type, args)
    -- Wrap supplied on_step
    object._on_step = object.on_step or object.type.on_step
    -- Make sure our on_step is called
    object.on_step = projectile_wrapper_on_step

    return object
end

-- LINEAR PROJECTILE

local LinearBase = ObjectUtils.type_create(Base)
M.LinearProjectileBase = LinearBase

LinearBase.on_draw = ObjectUtils.draw_sprite_member_if_seen
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

function LinearBase:on_deinit()
    local ANIMATION_FADEOUT_DURATION = 25
    Animations.fadeout_create { sprite = self.sprite, duration = ANIMATION_FADEOUT_DURATION, xy = self.xy }
end

function LinearBase.create(args)
    assert(args.velocity and args.sprite)

    args.type = args.type or LinearBase
    args.radius = args.radius or args.sprite.width / 2
    args.range_left = args.range_left or 250

    return LinearBase.base.create(args)
end

-- HELPER FUNCTIONS

function M.is_projectile(object)
    return table.contains(object.traits, M.PROJECTILE_TRAIT)
end

return M