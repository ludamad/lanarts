local ObjectUtils = import "lanarts.objects.ObjectUtils"
local Projectiles = import "lanarts.objects.Projectiles"
local Animations = import "lanarts.objects.Animations"
local Relations = import "lanarts.objects.Relations"
local LogUtils = import "lanarts.LogUtils"
local GameObject = import "core.GameObject"
local Actions = import "@Actions"

local ActionProjectileObject = GameObject.type_create(Projectiles.LinearProjectileBase)

function ActionProjectileObject:on_object_collide(other)
    local user = assert(self.stats.obj)
    if other ~= user then
        if Relations.is_hostile(user, other) then
            -- No prereq for attack projectile!
            self:apply_action(other)
            GameObject.destroy(self)
        elseif other.solid then
            GameObject.destroy(self)
        end
    end
end

ActionProjectileObject.on_draw = ObjectUtils.draw_sprite_member_if_seen

function ActionProjectileObject:apply_action(target_obj)
    Actions.use_action(self.stats, self.action, target_obj:stat_context(), self)
end

function ActionProjectileObject:on_deinit()
    local ANIMATION_FADEOUT_DURATION = 25
    Animations.fadeout_create { sprite = self.sprite, duration = ANIMATION_FADEOUT_DURATION, direction = self.direction, xy = self.xy }
end

function ActionProjectileObject:init(args)
    args.radius = args.radius or args.sprite.width / 2
    ActionProjectileObject.parent_init(self, args)
    self.sprite = args.sprite
    self.stats = args.stats
    self.action = args.action
end

return ActionProjectileObject