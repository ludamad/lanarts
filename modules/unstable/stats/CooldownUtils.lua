local Attacks = import "@Attacks"
local Types = import ".CooldownTypes"
local ContentUtils = import ".ContentUtils"
local Apts = import "@stats.AptitudeTypes"
local StatContext = import "@StatContext"

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

-- Augment an attack with a cooldown requirement and application
function M.derive_attack_with_cooldown(args)
    local attack = ContentUtils.derive_attack(args)
    -- Copy over all cooldown* members
    for k,v in pairs(args) do
        if k:find("cooldown") == 1 then
            attack[k] = attack[k] or args[k]
        end
    end
    attack.cooldown = attack.cooldown or Types.default_cooldown_table[Types.OFFENSIVE_ACTIONS] * (attack.delay or 1) 

    return M.resolve_action_cooldown(attack)
end

-- Function for applying to all cooldowns
function M.multiply_all_cooldown_rates(stats, rate, --[[Optional]] permanent)
    for type,_cooldown in pairs(Types.default_cooldown_table) do
        StatContext.multiply_cooldown_rate(stats, type, rate, permanent)
    end
end

function M.reset_rest_cooldown(stats)
    local cooldown = Types.default_cooldown_table[Types.REST_ACTION]
    StatContext.apply_cooldown(stats, Types.REST_ACTION, cooldown)
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

local function meets_cooldown_prereqs(action, stats)
    local c = action.cooldown_type or M.GENERIC_ACTION

    if StatContext.has_cooldown(stats, --[[Self cooldown]] action) then
        return false, FAIL_SELF_MESSAGE
    end 

    -- Check all the implied cooldown types ('parents')
    for type in values(c.required_cooldowns) do
        if StatContext.has_cooldown(stats, type) then return false, FAIL_MESSAGE end
    end

    return true
end

local function apply_cooldowns(action, stats)
    local c = action.cooldown_type or M.GENERIC_ACTION

    local mult = 1.0
    if c.aptitude then 
        mult = mult + APT_WORTH * stats.derived.aptitudes.effectiveness[c.aptitude]
    end
    local self_mult = action.fixed_self_cooldown and 1.0 or mult
    StatContext.add_cooldown(stats, action, (action.cooldown_self or action.cooldown) * self_mult)

    -- Check all the implied cooldown types ('parents')
    for type in values(c.required_cooldowns) do
        local field_name = Types.cooldown_field_map[type]
        StatContext.add_cooldown(stats, type, (action[field_name] or action.cooldown) * mult)
    end
end

-- Note: Forward declared above
function resolve_action(action, on_preq, on_use)
    function action:on_prerequisite(...)
        local success, err = true
        if on_preq then success,err = on_preq(self, ...) end
        if not success then return success,err end

        return meets_cooldown_prereqs(self, ...)
    end

    function action:on_use(...)
        local val = on_use and (on_use(self, ...))
        return apply_cooldowns(self, ...) or val
    end

    return action
end

return M