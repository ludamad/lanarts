local ObjectUtils = import "lanarts.objects.ObjectUtils"
local GameObject = import "core.GameObject"
local Animations = import "lanarts.objects.Animations"
local Attacks = import "@Attacks"
local AttackableObject = import ".AttackableObject"
local MonsterType = import "@MonsterType"
local GameMap = import "core.GameMap"
local PlayerObject = import ".PlayerObject"

local M = nilprotect {} -- Submodule

M.MONSTER_TRAIT = "MONSTER_TRAIT"

-- Monster object methods:

local function monster_die(self)
    if self.destroyed then return end

    GameObject.destroy(self)
    Animations.fadeout_create {
        map = self.map, xy = self.xy,
        sprite = self.sprite,
        duration = 20
    }

    if self.on_die then self:on_die() end
end

local function monster_step(self)
    -- Death event:
    if self.base_stats.hp <= 0 then 
        return monster_die(self) 
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
    local in_range = Attacks.obj_in_range(self.unarmed_attack, S, P)
    if self.unarmed_attack:on_prerequisite(S,P) and in_range then
        self:apply_attack(self.unarmed_attack, player)
    end

    local dx,dy = unpack(vector_subtract(player.xy, self.xy))
    local move_speed = self:stat_context().derived.movement_speed
    local xspeed, yspeed= math.min(move_speed, math.abs(dx)), math.min(move_speed, math.abs(dy))
    if vector_distance(player.xy, self.xy) > player.radius + self.radius then
        self.xy = vector_add(self.xy, {math.sign_of(dx) * xspeed, math.sign_of(dy) * yspeed})
    end
end

-- We need to create a new type for each monster, based off their MonsterType and AttackableObject
-- Since it is immutable and we only need oen per MonsterType we can keep a cache
local MONSTER_OBJECT_TYPE_CACHE = {}
local function monster_object_type(type)
    if MONSTER_OBJECT_TYPE_CACHE[type] then
        return MONSTER_OBJECT_TYPE_CACHE[type]
    end
    local object_type = ObjectUtils.type_create(AttackableObject)
    -- Merge data from monster type
    for k,v in pairs(type) do object_type[k] = v end

    object_type.on_step = monster_step

    MONSTER_OBJECT_TYPE_CACHE[type] = object_type
    return object_type
end

function M.create(args)
    assert(args.monster_type and not args.type)
    if type(args.monster_type) == "string" then
        args.monster_type = MonsterType.lookup(args.monster_type)
    end
    local obj_type = monster_object_type(args.monster_type)

    args.base_stats = table.deep_clone(args.monster_type.base_stats)

    -- Set up type signature
    args.traits = args.traits or {}
    table.insert(args.traits, M.MONSTER_TRAIT)

    -- AttackableObject configuration
    args.has_attack = true

    return obj_type.base_create(args)
end

function M.is_attackable(obj)
    return table.contains(obj.traits, M.MONSTER_TRAIT)
end

return M