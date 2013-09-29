local StatPrereqs = import "@StatPrereqs"
local StatEffects = import "@StatEffects"
local CooldownTypes = import ".CooldownTypes"
local ProjectileEffect = import ".ProjectileEffect"
local ContentUtils = import ".ContentUtils"
local Actions = import "@Actions"
local Attacks = import "@Attacks"
local StatusType = import "@StatusType"
local RangedWeaponActions = import "@items.RangedWeaponActions"

local M = nilprotect {} -- Submodule

function M.find_attack(action, --[[Optional]] recursive)
    if recursive then 
        local attacks = M.get_nested_attacks(action)
        assert(#attacks == 1)
        return attacks[1]
    end
    return Actions.get_effect(action, Attacks.AttackEffect)
end

-- Grabs all the attacks from the action, including those nested in projectile effects
function M.get_nested_attacks(action)
    local attacks = {}

    local function add_attack(action)
        local attack = M.find_attack(action)
        if attack then table.insert(attacks, attack) end
    end

    add_attack(action)  -- Lookup direct attack
    local projectile = Actions.get_effect(action, ProjectileEffect) -- Lookup projectile attack
    if projectile then add_attack(projectile.action) end
    local ammo_fire = Actions.get_effect(action, RangedWeaponActions.AmmoFireEffect) -- Lookup ammo fire effect
    if ammo_fire then add_attack(ammo_fire.action) end

    return attacks
end

-- Heuristically determines the likely desirable aptitudes for an action.
-- This is primarily intended for making spell definitions easier.
function M.desirable_user_aptitudes(action)
    local attacks = M.get_nested_attacks(action)
    local apt_table = {}
    for attack in values(attacks) do
        for sub_attack in values(action.sub_attacks) do
            for k,v in pairs (sub_attack.effectiveness_multipliers) do
                if v > 0 then apt_table[k] = true end
            end
            for k,v in pairs (sub_attack.damage_multipliers) do
                if v > 0 then apt_table[k] = true end
            end
        end
    end
    local apt_list = table.key_list(apt_table)
    table.sort(apt_list)
    return apt_list
end

-- Locate cooldown field indicators, and return their associated cooldown types and values
local function find_cooldowns(t)
    local arr=CooldownTypes.cooldown_fields
    local idx,len = 1,#arr
    return function()
        while idx <= len do
            local field = arr[idx]
            idx = idx + 1
            if t[field] then
                return field, CooldownTypes.field_to_cooldown_map[field], t[field]
            end
        end
    end
end

-- Returns prerequisite+effect for the cooldowns found in a table, derived 
-- from eg cooldown_self = 40. Any 'parent cooldowns' 
-- will also have the value added to their cooldown timer. (See CooldownTypes)
-- You can override this behaviour by setting the cooldown type to false.
-- Returns nil if no cooldowns were detected.
function M.derive_cooldowns(args, --[[Optional, default false]] cleanup_members)
    local C = {} -- Requirements
    local found_any = false
    for field, k,v in find_cooldowns(args) do
        found_any = true
        if cleanup_members then args[field] = nil end
        C[k] = v
        for parent in values(CooldownTypes.parent_cooldown_types[k]) do
            C[parent] = C[parent] or v
        end
    end
    if not found_any then return nil end

    -- Filter the 'false' values (this is used in the rare case one wants to disable an implied cooldown)
    for k,v in pairs(C) do 
        if not v then 
            C[k] = nil
        end 
    end

    local prereq = StatPrereqs.CooldownPrereq.create(table.key_list(C))
    local effect = StatEffects.CooldownCostEffect.create(C)
    return prereq, effect
end

-- Returns prerequisite+effect for the stats that are required, and permanently lowered by the action.
-- These are derived from the 'costs' table. Additionally, there are two shortcuts for mp & hp: 'mp_cost' & 'hp_cost'
-- Returns nil if no cooldowns were detected.
function M.derive_stat_costs(args, --[[Optional, default false]] cleanup_members)
    if args.mp_cost or args.hp_cost then
        args.costs = args.costs or {}
        assert(not args.mp_cost or not args.costs.mp, "Duplicate definition of MP cost in action!")
        assert(not args.hp_cost or not args.costs.hp, "Duplicate definition of HP cost in action!")
        args.costs.mp, args.costs.hp = args.mp_cost, args.hp_cost
    end
    if not args.costs then return nil end
    local prereq = StatPrereqs.StatPrereq.create(args.costs)
    local effect = StatEffects.StatCostEffect.create(args.costs)
    if cleanup_members then args.costs, args.mp_cost, args.hp_cost = nil end
    return prereq, effect
end

function M.derive_distance_prereq(args, --[[Optional, default false]] cleanup_members)
    if args.range or args.min_distance then
        local ret = StatPrereqs.DistancePrereq.create(args.range, args.min_distance)
        if cleanup_members then args.range, args.min_distance = nil end
        return ret
    end
    return nil
end

local function derive_status_prereqs(args)
    if not args.user_statuses_cant_have and not args.user_statuses_must_have then
        return nil
    end
    return StatPrereqs.UserStatusPrereq.create(args.user_statuses_cant_have, args.user_statuses_must_have)
end

M.CustomEffect = newtype()

function M.CustomEffect:init(resource, prereq, on_use)
    self.prereq = prereq
    self.on_use = on_use
    self.context_resource = resource
end

function M.CustomEffect:apply(user, target)
    self.on_use()
end

local DEFAULT_MELEE_RANGE = 10
local DEFAULT_MELEE_COOLDOWN = 45

local function add_prereq(action, prerequisite)
    if prerequisite then table.insert(action.prerequisites, prerequisite) end
end
local function add_effect(action, effect)
    if effect then table.insert(action.effects, effect) end
end

local function add_prereq_and_effect(action, prereq, effect)
    add_prereq(action, prereq)
    add_effect(action, effect)
end

M.USER_ACTION_COMPONENTS = { -- Ideal for eg the action_wield of a ranged weapon / action_use of ranged spell.
    derive_range = true,
    derive_cooldown = true,
    derive_equipment_prereq = true,
    derive_stat_cost = true,
    derive_user_status_prereq = true, derive_user_status_add = true,
    derive_projectile_create = true
}
M.TARGET_ACTION_COMPONENTS = { -- Ideal for melee/touch attacks
    derive_attack = true, 
    derive_target_status_add = true
}
M.ALL_ACTION_COMPONENTS = {} -- Ideal for melee/touch attacks
table.merge(M.ALL_ACTION_COMPONENTS, M.USER_ACTION_COMPONENTS)
table.merge(M.ALL_ACTION_COMPONENTS, M.TARGET_ACTION_COMPONENTS)

-- Derive different action components. This is used for items, spells, and miscellaneous abilities.
-- Components recognized by this routine:
-- cooldowns and stat costs (see derive_cooldowns and derive_stat_costs)
-- distance requirements (see derive_distance_prereq),
-- attacks (see ContentUtils.derive_attack_effect)
-- projectiles (see ProjectileEffect.derive_projectile)
function M.derive_action(args, --[[Optional]] options, --[[Optional, default false]] cleanup_members)
    options = options or M.ALL_ACTION_COMPONENTS
    local action = { 
        target_type = args.target_type or Actions.TARGET_HOSTILE,
        prerequisites = args.prerequisites or {}, effects = args.effects or {},
        -- Optional arguments
        name = args.name or nil, sprite = args.sprite or nil,
        -- on_prerequisite and on_use are special in that they take a context object when used, allowing more dynamic action content. 
        on_prerequisite = args.on_prerequisite or nil, 
        on_use = args.on_use or nil
    }
    if options.derive_equipment_prereq and args.equipment_prereq then
        local E = args.equipment_prereq
        local slot, type, name = E.slot, E.type, E.name
        if not name and type then name = type.name end
        if not slot and type then slot = type.slot end
        assert(name and slot)
        local equip_prereq = StatPrereqs.EquipmentPrereq.create(slot, name, type, E.trait, E.amount or 1)
        add_prereq(action, equip_prereq)
    end
    -- Damaging attack effect
    if options.derive_attack then
        add_effect(action, ContentUtils.derive_attack_effect(args.attack or args, cleanup_members))
    end
    if options.derive_range then
        args.range = args.range or DEFAULT_MELEE_RANGE
        -- Min & max distance requirement
        add_prereq(action, M.derive_distance_prereq(args, cleanup_members))
    end
    if options.derive_cooldown then
        args.cooldown_offensive = args.cooldown_offensive or (DEFAULT_MELEE_COOLDOWN * (args.delay or 1))
        add_prereq_and_effect(action, M.derive_cooldowns(args, cleanup_members))
    end
    -- Stat costs and cooldowns
    if options.derive_stat_cost then
        add_prereq_and_effect(action, M.derive_stat_costs(args, cleanup_members))
    end
    if options.derive_user_status_prereq then
        add_prereq(action, derive_status_prereqs(args))
    end
    if options.derive_user_status_add then
        for s in values(args.user_statuses_added) do
            table.insert(action.effects, StatEffects.UserStatusEffect.create(unpack(s)))
        end
    end
    if options.derive_target_status_add then
        for s in values(args.target_statuses_added) do
            table.insert(action.effects, StatEffects.TargetStatusEffect.create(unpack(s)))
        end
    end
    if options.derive_projectile_create and args.created_projectile then
        -- Projectile effect
        local P = args.created_projectile
        P.sprite = P.sprite or args.sprite -- Inherit sprite from outer action
        local effect = ProjectileEffect.derive_projectile_effect(P, cleanup_members)
        add_effect(action, assert(effect))
    end

    if cleanup_members then
        args.target_type, args.prerequisites, args.effects = nil
    end

    return action
end

return M