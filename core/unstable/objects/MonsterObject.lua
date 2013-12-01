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
local Relations = import "lanarts.objects.Relations"

local rawget, type = rawget, type

local ar_create = ActionResolvers.AIActionResolver.create

local MonsterObject = GameObject.type_create(CombatObject)
function MonsterObject:init(xy, collision_group, monster_type, --[[Optional]] team)
    self.monster_type = MonsterType.resolve(monster_type)

    MonsterObject.parent_init(self, xy, self.monster_type.radius, 
        table.deep_clone(self.monster_type.base_stats), -- Base stats 
        team or Relations.TEAM_MONSTER_ROOT, -- Team
        self.monster_type.unarmed_action, -- Unarmed action
        ar_create(collision_group) -- Action resolver
    )
end

local GOIndex = GameObject.Base.__index
-- Ensure that the monster's type is looked up in resolution
function MonsterObject:__index(k)
    local v = rawget(MonsterObject, k)
    if v ~= nil then return v end
    v = rawget(rawget(self, "monster_type"), k)
    if v ~= nil then return v end
    return GOIndex(self, k)
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
    self.deactivated = true
    for _, p in ipairs(self.map.players) do
        if math.abs(p.x - self.x) < DIST_THRESHOLD and math.abs(p.y - self.y) < DIST_THRESHOLD then
            self.deactivated = false
            break
        end
    end
    MonsterObject.parent_on_step(self)
end

function MonsterObject:on_predraw()
    if Map.object_visible(self) then
        ObjectUtils.screen_draw(shadow, self.xy)
    end
end

return MonsterObject