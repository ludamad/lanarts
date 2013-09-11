local ObjectUtils = import "lanarts.objects.ObjectUtils"
local AttackableObject = import ".AttackableObject"
local MonsterType = import "@MonsterType"

local M = nilprotect {} -- Submodule

M.MONSTER_TRAIT = "MONSTER_TRAIT"

-- We need to create a new type for each monster, based off their MonsterType
-- Since it is immutable and we only need oen per MonsterType we can keep a cache
local MONSTER_OBJECT_TYPE_CACHE = {}

--local function monster_object_type()

function M.create(args)
    assert(args.monster_type)
--    local object_type = ObjectUtils.type_create(AttackableObject) 

    if type(args.monster_type) == "string" then
        args.monster_type = MonsterType.lookup(args.monster_type)
    end

    args.sprite = args.monster_type.sprite
    args.base_stats = table.deep_clone(args.monster_type.stats)
    args.unarmed_attack = args.unarmed_attack or args.monster_type.unarmed_attack

    -- Set up type signature
    args.type = args.type or MonsterObject
    args.traits = args.traits or {}
    table.insert(args.traits, M.MONSTER_TRAIT)

    -- AttackableObject configuration
    args.can_attack = true
end

return M

local MonsterObject = ObjectUtils.type_create(AttackableObject) 

function MonsterObject.create(args)
    assert(args.monster_type)

    return MonsterObject._base_create(MonsterObject, args)
end

function MonsterObject:on_draw()
    print "HELLO!!!"
end

function MonsterObject.is_attackable(obj)
    return table.contains(obj.traits, MonsterObject.MONSTER_TRAIT)
end

return MonsterObject