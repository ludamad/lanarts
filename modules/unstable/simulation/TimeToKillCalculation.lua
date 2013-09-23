-- Calculate the steps ('time') required to kill a monster.
-- Intended for a rough, comparable figure of a character's damage output when using a certain weapon / spell / action.

local StatContext = import "@StatContext"
local LogUtils = import "lanarts.LogUtils"

local M = nilprotect {} -- Submodule

local MAX_TIME = 5000
function M.calculate_time_to_kill(attacker, action, target)
    local was_debug = LogUtils.get_debug_mode()
    LogUtils.set_debug_mode(false)
    for steps=1,MAX_TIME do
        for v in values{attacker, target} do
            StatContext.on_step(v)
            StatContext.on_calculate(v)
        end
        if action:on_prerequisite(attacker, target) then
            action:on_use(attacker, target)
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