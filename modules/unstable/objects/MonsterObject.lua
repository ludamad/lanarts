local ObjectUtils = import "lanarts.objects.ObjectUtils"
local GameObject = import "core.GameObject"
local Animations = import "lanarts.objects.Animations"
local Attacks = import "@Attacks"
local CombatObject = import ".CombatObject"
local MonsterType = import "@MonsterType"
local GameMap = import "core.GameMap"
local PlayerObject = import ".PlayerObject"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local LogUtils = import "lanarts.LogUtils"

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

function MonsterObject:on_draw()
    local hostile = ObjectUtils.find_closest_hostile(self)
    local check = GameMap.line_tile_check(self.map, self.xy, hostile.xy)
    ObjectUtils.screen_draw(self.sprite, self.xy)
    Fonts.small:draw({color=COL_WHITE}, self.xy, check and "NOT SEEN" or "SEEN")
end

function MonsterObject:on_step()
    self.base.on_step(self)
    local hostile = ObjectUtils.find_closest_hostile(self)
    if not hostile then return end -- No target

    local S,P = self:stat_context(), hostile:stat_context()
    local weapon_action, source = self:weapon_action()
    if self:can_use_action(weapon_action, P, source) then
        self:use_action(weapon_action, P, source) 
    end

    local dx,dy = unpack(vector_subtract(hostile.xy, self.xy))
    local move_speed = self:stat_context().derived.movement_speed
    local xspeed, yspeed= math.min(move_speed, math.abs(dx)), math.min(move_speed, math.abs(dy))
    if vector_distance(hostile.xy, self.xy) > hostile.radius + self.radius then
        self.xy = vector_add(self.xy, {math.sign_of(dx) * xspeed, math.sign_of(dy) * yspeed})
    end
end

function MonsterObject.create(args)
    assert(args.monster_type and not args.type)
    if type(args.monster_type) == "string" then
        args.monster_type = MonsterType.lookup(args.monster_type)
    end
    -- The AI state module holds the 
    args.ai_state = {}
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