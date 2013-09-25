-- DEPRECATED delete once ActionProjectileObject replaces it
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local Projectiles = import "lanarts.objects.Projectiles"
local Animations = import "lanarts.objects.Animations"
local Relations = import "lanarts.objects.Relations"
local LogUtils = import "lanarts.LogUtils"
local GameObject = import "core.GameObject"

local AttackProjectileObject = ObjectUtils.type_create(Projectiles.LinearProjectileBase)

function AttackProjectileObject:on_object_collide(other)
    local user = assert(self.stats.obj)
    if other ~= user then
        if Relations.is_hostile(user, other) then
            -- No prereq for attack projectile!
            self:apply_attack(other)
            GameObject.destroy(self)
        elseif other.solid then
            GameObject.destroy(self)
        end
    end
end

AttackProjectileObject.on_draw = ObjectUtils.draw_sprite_member_if_seen

function AttackProjectileObject:apply_attack(target_obj)
    local target = target_obj:stat_context()

    local dmg = self.attack:on_use(self.stats, target)
    LogUtils.event_log_resolved(target.obj, "{The }$You take{s} " ..dmg .. " damage!", COL_GREEN)
    LogUtils.event_log_resolved(target.obj, "{The }$You [have]{has} " .. math.ceil(target.base.hp) .. "HP left.", COL_PALE_BLUE)
end

function AttackProjectileObject:on_deinit()
    local ANIMATION_FADEOUT_DURATION = 25
    Animations.fadeout_create { sprite = self.sprite, duration = ANIMATION_FADEOUT_DURATION, xy = self.xy }
end

function AttackProjectileObject.create(args)
    assert(args.velocity and args.sprite and args.stats and args.attack)
    args.radius = args.radius or args.sprite.width / 2
    return AttackProjectileObject.base_create(args)
end

return AttackProjectileObject