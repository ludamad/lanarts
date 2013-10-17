local ObjectUtils = import "lanarts.objects.ObjectUtils"
local GameObject = import "core.GameObject"
local Animations = import "lanarts.objects.Animations"
local Attacks = import "@Attacks"
local CombatObject = import ".CombatObject"
local MonsterType = import "@MonsterType"
local GameMap = import "core.Map"
local PlayerObject = import ".PlayerObject"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local LogUtils = import "lanarts.LogUtils"
local ActionResolvers = import ".ActionResolvers"

local MonsterObject = ObjectUtils.type_create(CombatObject)
-- Ensure that the monster's type is looked up in resolution
function MonsterObject:__index(k)
    if k == "monster_type" then return nil end -- Don't recurse
    local monster_type = self["monster_type"]
    return monster_type and monster_type[k]
end

MonsterObject.MONSTER_TRAIT = "MONSTER_TRAIT"

-- Monster object methods:

function MonsterObject:on_death(attacker_obj)
    local xp = ExperienceCalculation.challenge_rating_to_xp_gain(attacker_obj.base_stats.level, self.challenge_rating)
    xp = random_round(xp)
    attacker_obj:gain_xp(xp)
    if self.destroyed then return end

    Animations.fadeout_create {
        map = self.map, xy = self.xy,
        sprite = self.sprite,
        duration = 20
    }

    if self.on_die then self:on_die() end
    GameObject.destroy(self)
end

function MonsterObject.create(args)
    assert(args.monster_type and not args.type)
    args.action_resolver = ActionResolvers.AIActionResolver.create(args.collision_group)
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