-- The item sprite is 'auto-calculated' from the sprite name.

local ItemType = import "@ItemType"
local Apts = import "@stats.AptitudeTypes"
local StatContext = import "@StatContext"
local Actions = import "@Actions"
local ActionUtils = import "@stats.ActionUtils"
local Attacks = import "@Attacks"
local ItemTraits = import ".ItemTraits"
local Proficiency = import "@Proficiency"
local ContentUtils = import "@stats.ContentUtils"
local CooldownUtils = import "@stats.CooldownUtils"
local ItemRandomDescriptions = import ".ItemRandomDescriptions"

local M = nilprotect {} -- Submodule

local function equipment_action_use(type)
    local capacity = ItemTraits.equipment_slot_capacities[type]
    assert(capacity) 
    return {
        prerequisites = {},
        effects = {},
        on_use = function(item_slot, stats)
            -- De-equip if equipped
            if item_slot.equipped then
                item_slot.equipped = false
                return 0 -- Use 0 copies
            end
    
            -- Equip if not equipped
            local equipped_items = StatContext.get_equipped_items(stats, type)
            if #equipped_items == capacity then
                assert(#equipped_items > 0)
                StatContext.deequip_item(stats, equipped_items[1])
            end
            StatContext.equip_item(stats, item_slot)
            return 0 -- Use 0 copies
        end
    }
end

-- Filters melee, magic, ranged. Useful for determining identify skills, which should not be affected by these aptitudes.
function M.filter_main_aptitudes(apts)
    local new_apts = {}
    for value in values(apts) do
        if value ~= Apts.MELEE and value ~= Apts.MAGIC and value ~= Apts.RANGED then
            table.insert(new_apts, value)
        end
    end
    return new_apts
end

local function add_default_types(t, args, difficulty, --[[Optional]] types)
    if not types then
        local default_types = args.types or {Apts.MAGIC_ITEMS}
        types = M.filter_main_aptitudes(default_types)
    end
    local P = Proficiency
    table.insert(t,P.proficiency_requirement_create(P.proficiency_type_create(types), args.difficulty))
end

local function type_define(args, type, --[[Optional]] on_init, --[[Optional]] not_equipment)
    args.base_equip_bonuses = args.base_equip_bonuses or {}
    args.base_equip_bonuses.aptitudes = ContentUtils.resolve_aptitude_bonuses(args, args.base_equip_bonuses.aptitudes)

    args.traits = args.traits or {}
    table.insert(args.traits, type)
    table.insert_all(args.traits, args.types or {})

    if not not_equipment then
        table.insert(args.traits, ItemType.EQUIPMENT_TRAIT)
    end

    -- Proficiency and identification
    if not args.proficiency_requirements then
        args.proficiency_requirements = {}
        local prof_types = args.proficiency_types
        if not prof_types then
            prof_types = args.types and args.types or {ItemTraits.default_equipment_slot_types[type]}
        end
        if args.difficulty and _G.type(args.difficulty) ~= 'table' then
            add_default_types(args.proficiency_requirements, args, args.dfficulty, prof_types)
        end
    end
    args.needs_identification = args.needs_identification or true

    if args.identify_difficulty and args.identify_types then
        if not args.identify_requirements then
            args.identify_requirements = {}
        end
        add_default_types(args.identify_requirements, args, args.identify_dfficulty, args.identify_types)
    end

    args.sprite = ContentUtils.resolve_sprite(args)
    -- Derive item-use action. Defaulted to equipping for equipment types. 
    if not_equipment or args.action_use then
        args.action_use = ActionUtils.derive_action(args.action_use or args, --[[Cleanup]] true)
    else
        args.action_use = equipment_action_use(type)
    end
    assert(args.action_use)
    args.stackable = args.stackable or (ItemTraits.equipment_slot_capacities[type] == nil)
    args.on_init = args.on_init or on_init

    assert(_G.type(args.proficiency_requirements) == 'table')
    return ItemType.define(args)
end

local function bonus_str1(val) return (val >= 0) and '+'..val or val end
local function bonus_str2(b1,b2) return ("%s,%s"):format(bonus_str1(b1 or 0), bonus_str1(b2 or 0)) end

local function resolve_identify_requirements(self, id_types, difficulty)
    local P = Proficiency

    local types = M.filter_main_aptitudes(self.types)
    table.insert_all(types, id_types)
    self.identify_requirements = {P.proficiency_requirement_create(types, difficulty)}
end

local function init_identify_requirements(self, id_types, difficulty)
    local P = Proficiency

    local types = M.filter_main_aptitudes(self.types)
    table.insert_all(types, id_types)
    self.identify_requirements = {P.proficiency_requirement_create(types, difficulty)}
end
function M.resolve_weapon_bonuses(self)
    local b1, b2 = self.effectiveness_bonus or 0, self.damage_bonus or 0
    self.unidentified_name = self.unidentified_name or self.type.name
    self.name =  bonus_str2(b1,b2) .. ' ' .. self.type.name

    local difficulty = ((b1*b1+b2*b2) ^ 0.75) + random(-1,3) + (self.difficulty or 0)
    init_identify_requirements(self, {Apts.WEAPON_IDENTIFICATION}, random_round(difficulty))
    self.action_wield = table.deep_clone(self.action_wield)
    local attack = assert(Actions.get_effect(self.action_wield, Attacks.AttackEffect))
    Attacks.attack_add_effectiveness_and_damage(attack, b1, b2)
end

local DEFAULT_MELEE_RANGE = 10
function M.weapon_define(args)
    -- Define weapon attack in convenient manner
    assert(args.types and args.difficulty and args.gold_worth)
    if not args.proficiency_types then
        args.proficiency_types = {}
        table.insert_all(args.proficiency_types, args.types)
        table.insert(args.proficiency_types, Apts.WEAPON_PROFICIENCY)
    end
    args.range = args.range or DEFAULT_MELEE_RANGE
    args.action_wield = ActionUtils.derive_action(args, --[[Cleanup]] true)
    return type_define(args, ItemTraits.WEAPON, M.resolve_weapon_bonuses)
end

function M.resolve_armour_bonuses(self)
    local b = self.bonus or 0
    self.unidentified_name = self.unidentified_name or self.type.name
    self.name =  bonus_str1(b) .. ' ' .. self.type.name

    local difficulty = (self.bonus ^ 1.5) + random(-1,3) + (self.difficulty or 0)
    init_identify_requirements(self, {Apts.ARMOUR}, random_round(difficulty))
    -- Twice as much resistance as defence allocated
    local res_b, def_b = math.ceil(b * 2 / 3), math.floor(b / 3)
    self.equipment_bonuses = {
        aptitudes = ContentUtils.resolve_aptitude_bonuses { [Apts.MELEE] = {0,0,res_b,def_b} }
    }
end

function M.body_armour_define(args)
    assert(args.difficulty and args.gold_worth)
    return type_define(args, ItemTraits.BODY_ARMOUR, M.resolve_armour_bonuses)
end

function M.ring_define(args)
    return type_define(args, ItemTraits.RING)
end

function M.gloves_define(args)
    return type_define(args, ItemTraits.GLOVES, M.resolve_armour_bonuses)
end

function M.bracers_define(args)
    return type_define(args, ItemTraits.BRACERS, M.resolve_armour_bonuses)
end

function M.headgear_define(args)
    return type_define(args, ItemTraits.HEADGEAR, M.resolve_armour_bonuses)
end

function M.boots_define(args)
    return type_define(args, ItemTraits.BOOTS, M.resolve_armour_bonuses)
end

function M.ammunition_define(args)
    return type_define(args, ItemTraits.AMMUNTION)
end

function M.potion_define(args)
    assert(args.on_use)
    return type_define(args, ItemTraits.POTION, --[[on_init]] nil, --[[not equipment?]] true)
end

function M.scroll_define(args)
    assert(args.on_use)
    return type_define(args, ItemTraits.SCROLL, --[[on_init]] nil, --[[not equipment?]] true)
end

return M