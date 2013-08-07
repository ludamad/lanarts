local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"

local Projectile = {}

-- Takes as arguments:
-- xy, user, target, radius, velocity, on_step, on_draw, on_collide
function Projectile.create(args)
    local object = GameObject.create {
        xy = args.xy, 
        radius = args.radius, 
        solid = false,
        on_step = Projectile.on_step,
        on_draw = args.on_draw,
        do_init = args.do_init
    }

    object._on_step = args.on_step
    -- Avoid setting anything we already used
    args = table.clone(args)
    args.xy, args.on_draw, args.on_step, args.radius = nil
    for k,v in pairs(args) do
        object[k] = v
    end

    return object
end

function Projectile:on_step()
    local collisions = GameMap.object_collisions(self)
    for c in values(collisions) do
        self:on_collide(c)
    end

    self:_on_step()
end

function Projectile.example_projectile(user, target, xy)
    local projectile = Projectile.create()
end

return Projectile