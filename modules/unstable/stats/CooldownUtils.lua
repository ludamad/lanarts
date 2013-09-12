local Attacks = import "@Attacks"
local Types = import ".CooldownTypes"
local Apts = import "@stats.AptitudeTypes"

local M = nilprotect {} -- Submodule

--------------------------------------------------------------------------------
--              Main API                                                      --
--  Cooldown control variables:                                               --
--    - cooldown_type                                                         --
--    - cooldown, cooldown_offensive                                          --
--    - cooldown_melee, cooldown_spell                                        --
--    - cooldown_item, cooldown_ability                                       --
--------------------------------------------------------------------------------

local resolve_action -- Forward declare

function M.resolve_action_cooldown(action, --[[Optional]] args)
    args = args or action
    return resolve_action(action, args.on_prerequisite, args.on_use)
end

-- Augment an attack with a cooldown requirement and 
function M.derive_attack_with_cooldown(args)
    local attack = args.unarmed_attack or args.attack
    local types = args.multipliers or args.types
    assert(types)

    -- First resolve 
    if attack and #attack > 0 then -- Resolve argument table
        attack = Attacks.attack_create(attack)
    end
    if not args.attack then
        attack = Attacks.attack_create(
            args.effectiveness or 0, args.damage or 0, types, 
            --[[Optional]] args.delay, --[[Optional]] args.damage_multiplier
        )
    end

    return M.resolve_action_cooldown(attack)
end

--------------------------------------------------------------------------------
--              Helper functions                                              --
--------------------------------------------------------------------------------

local MIN_DELAY,MAX_DELAY = 0.25,4
local APT_WORTH = 0.05

local FAIL_SELF_MESSAGE = "It's not ready!"
local FAIL_MESSAGE = "You must wait before using this!"

local function make_action(t, --[[Optional]] apt)
    local implied = {t}
    table.insert_all(implied, Types.parent_cooldown_types[t])
    return { required_cooldowns = implied, aptitude = apt }
end

M.GENERIC_ACTION = make_action(Types.ALL_ACTIONS)
M.OFFENSIVE_ACTION = make_action(Types.ALL_ACTIONS)
M.MELEE_ACTION = make_action(Types.MELEE_ACTIONS, Apts.MELEE_SPEED)
M.RANGE_ACTION = make_action(Types.MELEE_ACTIONS, Apts.RANGED_SPEED)
M.SPELL_ACTION = make_action(Types.ALL_ACTIONS, Apts.MAGIC_SPEED)
M.ITEM_ACTION = make_action(Types.ITEM_ACTIONS)
M.ABILITY_ACTION = make_action(Types.ABILITY_ACTIONS)

local function has_cooldowns(action, stats)
    local c = action.cooldown_type or M.GENERIC_ACTION

    local C, has = stats.cooldowns, stats.cooldowns.has_cooldown 
    if not has(C, --[[Self cooldown]] action) then
        return false, FAIL_SELF_MESSAGE
    end 

    -- Check all the implied cooldown types ('parents')
    for type in values(c.required_cooldowns) do
        if not has(C, type) then return false, FAIL_MESSAGE end
    end

    return true
end

local function apply_cooldowns(action, stats)
    local c = action.cooldown_type or M.GENERIC_ACTION

    local C, add = stats.cooldowns, stats.cooldowns.add_cooldown 

    local mult = 1.0
    if c.aptitude then 
        mult = mult + APT_WORTH * stats.aptitudes.effectiveness[c.aptitude]
    end
    local self_mult = action.fixed_self_cooldown and 1.0 or mult
    add(C, action, (action.cooldown_self or action.cooldown) * self_mult)

    -- Check all the implied cooldown types ('parents')
    for type in values(c.required_cooldowns) do
        local field_name = Types.cooldown_field_map[type]
        add(C, type, (action[field_name] or action.cooldown) * mult)
    end
    return true
end

-- Note: Forward declared above
function resolve_action(action, on_preq, on_use)
    function action:on_prerequisite(stats)
        local success, err = true
        if on_preq then success,err = on_preq() end
        if not success then return success,err end

        return has_cooldowns(self, stats)
    end

    function action:on_use(stats)
        local success, err = true
        if on_preq then success,err = on_preq() end
        if not success then return success,err end

        return has_cooldowns(self, stats)
    end

    return action
end

return M