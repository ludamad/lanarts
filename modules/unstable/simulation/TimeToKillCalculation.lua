-- Calculate the steps ('time') required to kill a monster.
-- Intended for a rough, comparable figure of a character's damage output when using a certain weapon / spell / action.

local StatContext = import "@StatContext"
local LogUtils = import "lanarts.LogUtils"
local Actions = import "@Actions"
local ActionUtils = import "@stats.ActionUtils"

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


--local function mock_object()

function M.calculate_time_to_kill(action_context, target)
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

return M