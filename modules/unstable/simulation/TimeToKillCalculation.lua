-- Calculate the steps ('time') required to kill a monster.
-- Intended for a rough, comparable figure of a character's damage output when using a certain weapon / spell / action.

local StatContext = import "@StatContext"
local LogUtils = import "lanarts.LogUtils"
local Actions = import "@Actions"
local ActionContext = import "@ActionContext"
local ActionUtils = import "@stats.ActionUtils"
local ProjectileEffect = import "@stats.ProjectileEffect"
local RangedWeaponActions = import "@items.RangedWeaponActions"
local StatContext = import "@StatContext"
local StatPrereqs = import "@StatPrereqs"
local EventLog = import "core.ui.EventLog"
local StatUtils = import "@stats.StatUtils"

local M = nilprotect {} -- Submodule

local function mock_object(stat_context)
    local obj = {
        xy = {0,0},
        base_stats = stat_context.base,
        radius = stat_context.obj.radius,
        traits = {}
    }
    setmetatable(obj, {__index = function() return do_nothing end})
    return obj
end

local function mock_stat_context(SC)
    local base, derived = table.deep_clone(SC.base), table.deep_clone(SC.derived)
    return StatContext.stat_context_create(base, derived, mock_object(SC)) 
end

local function action_remove_distance_prereq(A)
    Actions.reset_prerequisite(A, StatPrereqs.DistancePrereq)
end

local function action_collapse_nested(A, effect_type)
    local effect = Actions.reset_effect(A, effect_type) -- Grab and remove
    if effect then
        table.insert_all(A.effects, effect.action.effects)
    end
end

local function mock_action(A)
    local copy = table.deep_clone(A)
    action_collapse_nested(copy, ProjectileEffect)
    action_collapse_nested(copy, RangedWeaponActions.AmmoFireEffect)
    action_remove_distance_prereq(copy)
    return copy
end

local function mock_action_context(AC)
    local copy = table.clone(AC)
    copy.user = mock_stat_context(AC.user)
    copy.base = mock_action(AC.base)
    copy.derived = mock_action(AC.derived)
    return copy
end

local function simulate(action_context, target, max_time)
    action_context = mock_action_context(action_context)
    target = mock_stat_context(target)

    for steps=1,max_time do
        perf.timing_begin("**Stat context stepping player **")
        StatUtils.stat_context_on_step(action_context.user)
        perf.timing_end("**Stat context stepping player **")
        perf.timing_begin("**Stat context calculate player **")
        StatContext.on_calculate(action_context.user)
        perf.timing_end("**Stat context calculate player **")

        perf.timing_begin("**Stat context stepping monster **")
        StatUtils.stat_context_on_step(target)
        perf.timing_end("**Stat context stepping monster **")
        perf.timing_begin("**Stat context calculate monster **")
        StatContext.on_calculate(target)
        perf.timing_end("**Stat context calculate monster **")

        perf.timing_begin("**Action use**")
        if ActionContext.can_use_action(action_context, target) then
            ActionContext.use_action(action_context, target)
        end
        perf.timing_end("**Action use**")
        if target.base.hp <= 0 then 
            return steps
        end
    end

    pretty(action_context.derived)
    pretty(target.base)
    assert(false)
    return math.huge
end

local MAX_TIME = 5000
function M.calculate_time_to_kill(action_context, target, --[[Optional]] max_time)
    -- Silence the system
    local was_debug = LogUtils.get_debug_mode()
    local log_add_prev = EventLog.add

    LogUtils.set_debug_mode(false)
    EventLog.add = do_nothing

    local steps = simulate(action_context, target, max_time or MAX_TIME)

    -- Louden the system
    LogUtils.set_debug_mode(was_debug)
    EventLog.add = log_add_prev

    return steps
end

return M