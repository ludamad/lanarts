-- The item sprite is 'auto-calculated' from the sprite name.

local ItemType = import "@ItemType"
local Attacks = import "@Attacks"
local ItemTraits = import ".ItemTraits"
local ContentUtils = import "@stats.ContentUtils"

local M = nilprotect {} -- Submodule

local function on_use_equipment()
    print "TODO" --TODO
end

local function type_define(args, type, --[[Optional]] not_equipment)
    args.traits = args.traits or {}
    table.insert(args.traits, type)
    if not not_equipment then
        table.insert(args.traits, ItemType.EQUIPMENT_TRAIT)
    end
    args.sprite = ContentUtils.derive_sprite(args.name)
    args.on_use = args.on_use or on_use_equipment 
    return ItemType.define(args)
end

function M.weapon_define(args)
    -- Unwound weapon in convenient manner
    if args.attack and #args.attack > 0 then
        args.attack = Attacks.attack_create(unpack(args.attack))
    end 
    return type_define(args, ItemTraits.WEAPON)
end

function M.body_armour_define(args)
    return type_define(args, ItemTraits.BODY_ARMOUR)
end

function M.ring_define(args)
    return type_define(args, ItemTraits.RING)
end

function M.gloves_define(args)
    return type_define(args, ItemTraits.GLOVES)
end

function M.bracers_define(args)
    return type_define(args, ItemTraits.BRACERS)
end

function M.headgear_define(args)
    return type_define(args, ItemTraits.HEADGEAR)
end

function M.ammunition_define(args)
    return type_define(args, ItemTraits.AMMUNTION)
end

function M.potion_define(args)
    assert(args.on_use)
    return type_define(args, ItemTraits.POTION, true)
end

function M.scroll_define(args)
    assert(args.on_use)
    return type_define(args, ItemTraits.SCROLL, true)
end

return M