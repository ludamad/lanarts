local ObjectUtils = import "lanarts.objects.ObjectUtils"
local AttackableObject = import ".AttackableObject"
local MonsterType = import "@MonsterType"

local M = nilprotect {} -- Submodule

M.MONSTER_TRAIT = "MONSTER_TRAIT"

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

    function object_type:on_draw()
        self.sprite:draw(self.xy)
    end

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
    args.can_attack = true

    return obj_type._base_create(obj_type, args)
end

function M.is_attackable(obj)
    return table.contains(obj.traits, M.MONSTER_TRAIT)
end

return M