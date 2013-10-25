local ObjectUtils = import "lanarts.objects.ObjectUtils"
local GameObject = import "core.GameObject"
local Display = import "core.Display"
local Animations = import "lanarts.objects.Animations"
local Attacks = import "@Attacks"
local CombatObject = import ".CombatObject"
local MonsterType = import "@MonsterType"
local Map = import "core.Map"
local PlayerObject = import ".PlayerObject"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local LogUtils = import "lanarts.LogUtils"
local ActionResolvers = import ".ActionResolvers"

local MonsterObject = GameObject.type_create(CombatObject)
local rawget = rawget
-- Ensure that the monster's type is looked up in resolution
function MonsterObject:__index(k)
    return rawget(rawget(self, "monster_type"), k)
end

MonsterObject.MONSTER_TRAIT = "MONSTER_TRAIT"

-- Monster object methods:

function MonsterObject:on_deinit()
    table.clear(self)
end

function MonsterObject:on_death(attacker_obj)
    local xp = ExperienceCalculation.challenge_rating_to_xp_gain(attacker_obj.base_stats.level, self.challenge_rating)
    xp = random_round(xp)
    attacker_obj:gain_xp(xp)
    if self.destroyed then return end

    Animations.fadeout_create {
        map = self.map, xy = self.xy,
        sprite = self.sprite,
        direction = self.direction,
        duration = 20
    }

    if self.on_die then self:on_die() end
    GameObject.destroy(self)
end

local shadow = Display.image_load(path_resolve "sprites/minor-shadow.png")

local DIST_THRESHOLD = 900
function MonsterObject:on_step()
    assert(self.team)
    self.deactivated = true
    for _, p in ipairs(self.map.players) do
        if math.abs(p.x - self.x) < DIST_THRESHOLD and math.abs(p.y - self.y) < DIST_THRESHOLD then
            self.deactivated = false
            break
        end
    end
    CombatObject.on_step(self)
end

function MonsterObject:on_predraw()
    if Map.object_visible(self) then
        ObjectUtils.screen_draw(shadow, self.xy)
    end
end

-- Used internally by the lua bindings to know how to decode our custom object type
function MonsterObject:init(args)
    local mtype = assert(args.monster_type) -- Resolve monster_type
    if type(mtype) == "string" then mtype = MonsterType.lookup(mtype) end

    args.radius = args.radius or mtype.radius
    args.base_stats = table.deep_clone(mtype.base_stats)
    args.action_resolver = ActionResolvers.AIActionResolver.create(args.collision_group)

    MonsterObject.parent_init(self, args)

    self.monster_type = mtype
    self.unarmed_action = mtype.unarmed_action

    -- Set up type signature
    table.insert(self.traits, MonsterObject.MONSTER_TRAIT)
end

function MonsterObject.is_monster(obj)
    return table.contains(obj.traits, MonsterObject.MONSTER_TRAIT)
end

return MonsterObject