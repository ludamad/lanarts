-- Calculate the steps ('time') required to kill a monster.
-- Intended for a rough, comparable figure of a character's damage output when using a certain weapon / spell / action.

local StatContext = import "@StatContext"
local LogUtils = import "lanarts.LogUtils"
local Actions = import "@Actions"
local ActionContext = import "@ActionContext"
local ActionUtils = import "@stats.ActionUtils"
local StatContext = import "@StatContext"
local StatPrereqs = import "@StatPrereqs"

local M = nilprotect {} -- Submodule

local MAX_TIME = 5000
function M.calculate_time_to_kill_old(attacker, action, target, source)
    local was_debug = LogUtils.get_debug_mode()
    attacker = {
        base = attacker.base,
        derived = attacker.derived,
        obj = {xy = {0,0}, radius = attacker.obj.radius, traits = {}},
    }
    target = {
        base = target.base,
        derived = target.derived,
        obj = {xy = {1,1}, radius = target.obj.radius, traits = {}},
        traits = {}
    }
    LogUtils.set_debug_mode(false)
    for steps=1,MAX_TIME do
        for v in values{attacker, target} do
            StatContext.on_step(v)
            StatContext.on_calculate(v)
        end
        if Actions.can_use_action(attacker, action, target, source) then
            local attack = ActionUtils.find_attack(action, true)
            attack:apply(attacker, target)
        end
        if target.base.hp <= 0 then
            LogUtils.set_debug_mode(was_debug)
            return steps
        end
    end
    LogUtils.set_debug_mode(was_debug)
    return math.huge
end

local function mock_object(stat_context)
    local obj = {
        xy = {0,0},
        radius = stat_context.obj.radius,
        traits = {}
    }
    setmetatable(obj, {__index = function() return do_nothing end})
    return {
        base = stat_context.base,
        derived = stat_context.derived,
        obj = obj
    }
end

local function mock_stat_context(SC)
    local base, derived = table.deep_clone(SC.base), table.deep_clone(SC.derived)
    return StatContext.stat_context_create(base, derived, mock_object(SC)) 
end

local function action_remove_distance_prereq(A)
    Actions.reset_prerequisite(A, StatPrereqs.DistancePrereq)
end

local function action_collapse_projectile(A)
    if A.created_projectile then
        table.insert_all(A.effects, A.created_projectile.effects)
        A.created_projectile = nil
    end
end

local function mock_action(A)
    local copy = table.deep_clone(A)
    action_collapse_projectile(copy)
    action_remove_distance_prereq(copy)
    return copy
end

local function mock_action_context(AC)
    local copy = table.clone(AC)
    copy.user = mock_stat_context(AC.user)
    copy.derived = mock_action(AC.derived)
    return copy
end

function M.calculate_time_to_kill(action_context, target)
    action_context = mock_action_context(action_context)
    target = mock_stat_context(target)
    local was_debug = LogUtils.get_debug_mode()
    LogUtils.set_debug_mode(false)
    for steps=1,MAX_TIME do
        for v in values{action_context.user, target} do
            StatContext.on_step(v)
            StatContext.on_calculate(v)
        end
        if ActionContext.can_use_action(action_context, target) then
            ActionContext.use_action(action_context, target)
        end
        if target.base.hp <= 0 then
            LogUtils.set_debug_mode(was_debug)
            return steps
        end
    end
    LogUtils.set_debug_mode(was_debug)
    return math.huge
end

return M