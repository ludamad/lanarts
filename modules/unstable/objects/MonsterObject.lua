local ObjectUtils = import "lanarts.objects.ObjectUtils"
local GameObject = import "core.GameObject"
local Animations = import "lanarts.objects.Animations"
local Attacks = import "@Attacks"
local CombatObject = import ".CombatObject"
local MonsterType = import "@MonsterType"
local GameMap = import "core.GameMap"
local PlayerObject = import ".PlayerObject"

local MonsterObject = ObjectUtils.type_create(CombatObject)
-- Ensure that the monster's type is looked up in resolution
function MonsterObject:__index(k)
    if k == "monster_type" then return nil end -- Don't recurse
    local monster_type = self["monster_type"]
    return monster_type and monster_type[k]
end

MonsterObject.MONSTER_TRAIT = "MONSTER_TRAIT"

-- Monster object methods:

function MonsterObject:death(self)
    if self.destroyed then return end

    GameObject.destroy(self)
    Animations.fadeout_create {
        map = self.map, xy = self.xy,
        sprite = self.sprite,
        duration = 20
    }

    if self.on_die then self:on_die() end
end

function MonsterObject:on_step()
    -- Death event:
    if self.base_stats.hp <= 0 then 
        return self:death() 
    end
    self.base.on_step(self)
    local player
    for obj in GameMap.objects() do 
        if PlayerObject.is_player(obj) then 
            player = obj 
            break
        end
    end
    if not player then 
        return 
    end

    local S,P = self:stat_context(), player:stat_context()
    local weapon_action, source = self:weapon_action()
    if self:can_use_action(weapon_action, P, source) then 
        self:use_action(weapon_action, P, source) 
    end

    local dx,dy = unpack(vector_subtract(player.xy, self.xy))
    local move_speed = self:stat_context().derived.movement_speed
    local xspeed, yspeed= math.min(move_speed, math.abs(dx)), math.min(move_speed, math.abs(dy))
    if vector_distance(player.xy, self.xy) > player.radius + self.radius then
        self.xy = vector_add(self.xy, {math.sign_of(dx) * xspeed, math.sign_of(dy) * yspeed})
    end
end

function MonsterObject.create(args)
    assert(args.monster_type and not args.type)
    if type(args.monster_type) == "string" then
        args.monster_type = MonsterType.lookup(args.monster_type)
    end
    args.base_stats = table.deep_clone(args.monster_type.base_stats)

    -- Set up type signature
    args.traits = args.traits or {}
    table.insert(args.traits, MonsterObject.MONSTER_TRAIT)

    return MonsterObject.base_create(args)
end

function MonsterObject.is_monster(obj)
    return table.contains(obj.traits, MonsterObject.MONSTER_TRAIT)
end

return MonsterObject