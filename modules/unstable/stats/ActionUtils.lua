local StatPrereqs = import "@StatPrereqs"
local StatEffects = import "@StatEffects"
local CooldownTypes = import ".CooldownTypes"
local ProjectileEffect = import ".ProjectileEffect"
local ContentUtils = import ".ContentUtils"
local Actions = import "@Actions"
local Attacks = import "@Attacks"

local M = nilprotect {} -- Submodule

-- Grabs all the attacks from the action, including those nested in projectile effects
function M.get_nested_attacks(action)
    local attacks = {}

    local function add_attack(action)
        local attack = Actions.get_effect(action, Attacks.AttackEffect)
        if attack then table.insert(attacks, attack) end
    end

    -- Lookup direct attack
    add_attack(action)

    -- Lookup projectile attack
    local projectile = Actions.get_effect(action, ProjectileEffect)
    if projectile then
        add_attack(projectile.action)
    end

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
        while idx < len do
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

local function add_if_not_nil(action, prerequisite, effect)
    if prerequisite then table.insert(action.prerequisites, prerequisite) end
    if effect then table.insert(action.effects, effect) end
end

-- Derive different action components. This is used for items, spells, and miscellaneous abilities.
-- Components recognized by this routine:
-- cooldowns and stat costs (see derive_cooldowns and derive_stat_costs)
-- distance requirements (see derive_distance_prereq),
-- attacks (see ContentUtils.derive_attack)
-- projectiles (see ProjectileEffect.derive_projectile)
function M.derive_action(args, --[[Optional, default false]] cleanup_members)
    local action = { 
        target_type = args.target_type or Actions.TARGET_HOSTILE,
        prerequisites = args.prerequisites or {}, effects = args.effects or {},
        -- Optional arguments
        name = args.name or nil, sprite = args.sprite or nil 
    }

    -- Stat costs and cooldowns
    add_if_not_nil(action, M.derive_stat_costs(args, cleanup_members))
    add_if_not_nil(action, M.derive_cooldowns(args, cleanup_members))
    -- Min & max distance requirement
    add_if_not_nil(action, M.derive_distance_prereq(args, cleanup_members), --[[No associated effect]] nil)
    -- Damaging attack effect
    add_if_not_nil(action, --[[No associated prereq]] nil, ContentUtils.derive_attack(args.attack or args, cleanup_members))
    -- Projectile effect
    local P = args.created_projectile
    if P then
        P.sprite = P.sprite or args.sprite -- Inherit sprite from outer action
        local effect = ProjectileEffect.derive_projectile_effect(P, cleanup_members)
        add_if_not_nil(action, --[[No associated prereq]] nil, effect)
    end
    if cleanup_members then
        args.target_type, args.prerequisites, args.effects = nil
    end

    pretty_print(action.effects)

    return action
end

return M