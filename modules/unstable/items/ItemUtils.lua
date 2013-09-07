-- The item sprite is 'auto-calculated' from the sprite name.

local ItemType = import "@ItemType"

local M = nilprotect {} -- Submodule

M.weapon = "weapon"
M.body_armour = "body_armour"
M.ring = "ring"
M.gloves = "gloves"
M.boots = "boots"
M.bracers = "bracers"
M.headgear = "headgear"
M.ammunition = "ammunition"
M.POTION = "potion"
M.SCROLL = "scroll"


local function type_define(args, type)
    args.traits = args.traits or {}
    table.insert(args.traits, type)
    table.insert(args.traits, ItemType.EQUIPMENT_TRAIT)
    return ItemType.define(args)
end

function M.weapon_define(args)
    return type_define(args, M.weapon)
end

function M.body_armour_define(args)
    return type_define(args, M.body_armour)
end

function M.ring_define(args)
    return type_define(args, M.ring)
end

function M.gloves_define(args)
    return type_define(args, M.gloves)
end

function M.bracers_define(args)
    return type_define(args, M.bracers)
end

function M.headgear_define(args)
    return type_define(args, M.headgear)
end

function M.ammunition_define(args)
    return type_define(args, M.ammuntion)
end

return M