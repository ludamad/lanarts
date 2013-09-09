-- The item sprite is 'auto-calculated' from the sprite name.

local ItemType = import "@ItemType"
local Apts = import "@stats.AptitudeTypes"
local StatContext = import "@StatContext"
local Attacks = import "@Attacks"
local ItemTraits = import ".ItemTraits"
local ItemProficiency = import ".ItemProficiency"
local ContentUtils = import "@stats.ContentUtils"

local M = nilprotect {} -- Submodule

local function on_use_equipment(type)
    local capacity = ItemTraits.equipment_slot_capacities[type]
    assert(capacity) 
    return function(item_slot, stats)
        local equipped_items = StatContext.get_equipped_items(stats, type)
        if #equipped_items == capacity then
            assert(#equipped_items > 0)
            StatContext.deequip_item(stats, equipped_items[1])
        end
        StatContext.equip_item(stats, item_slot)
        return 0 -- Use 0 copies
    end
end

local function type_define(args, type, --[[Optional]] not_equipment)
    args.traits = args.traits or {}
    table.insert(args.traits, type)
    if not not_equipment then
        table.insert(args.traits, ItemType.EQUIPMENT_TRAIT)
    end
    if not args.proficiency_requirements then
        args.proficiency_requirements = {}
        local types = args.proficiency_types
        if not types then
            local default_types = args.types or {Apts.MAGIC_ITEMS}
            types = {}
            for value in values(default_types) do
                if value ~= Apts.MELEE and value ~= Apts.MAGIC then
                    table.insert(types, value)
                end
            end
        end
        local IP = ItemProficiency
        table.insert(args.proficiency_requirements, 
            IP.item_proficiency_requirement_create { IP.item_proficiency_type_create(types), args.difficulty }
        )
    end
    args.sprite = ContentUtils.derive_sprite(args.name)
    args.on_use = args.on_use or on_use_equipment(type)
    assert(_G.type(args.proficiency_requirements) == 'table')
    return ItemType.define(args)
end

function M.weapon_define(args)
    -- Unwound weapon in convenient manner
    assert(args.types and args.difficulty and args.gold_worth)
    args.traits = args.traits or {}
    for type in values(args.types) do 
        table.insert(args.traits, type)
    end
    if args.attack and #args.attack > 0 then
        args.attack = Attacks.attack_create(unpack(args.attack))
    end
    if not args.attack then
        args.attack = Attacks.attack_create(args.effectiveness or 0,args.damage or 0, args.multipliers or args.types, --[[Optional]] args.delay, --[[Optional]] args.defence_modifier)
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