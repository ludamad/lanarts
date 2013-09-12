local StatusType = import "@StatusType"
local StatContext = import "@StatContext"
local CooldownSet = import "@CooldownSet"

local CooldownTypes = import ".CooldownTypes"
local Apts = import ".AptitudeTypes"
local StatUtils = import ".StatUtils"
local LogUtils = import "lanarts.LogUtils"
local EventLog = import "core.ui.EventLog"

local M = nilprotect {} -- Submodule

-- Base for time-limited statuses

local TimeLimitedBase = {
    init = function(self, stats, time_left)
        self.time_left = time_left
    end,
    on_update = function(self, stats, time_left)
        self.time_left = math.max(self.time_left, time_left)
    end,
    on_step = function(self, stats)
        self.time_left = self.time_left - 1
        if self.time_left <= 0 then
            if self.on_deregister then
                self:on_deregister(stats)
            end
            return true -- Deregister
        end
    end
}

local function TIME_LIMITED(status_type)
    status_type.base = TimeLimitedBase
    for k,v in pairs(TimeLimitedBase) do
        if not status_type[k] then
            status_type[k] = v
        end
    end
    return status_type
end

-- EXHAUSTION
local EXHAUSTION_MOVEMENT_MULTIPLIER = 0.75
local EXHAUSTION_ATTACK_COOLDOWN_MULTIPLIER = 0.75
M.Exhausted = StatusType.define {
    TIME_LIMITED,
    init = function(self, stats, ...)
        self.base.init(self, stats, ...)
        LogUtils.log_if_player(stats.obj, "$You {is}[are] now exhausted.", {255,200,200})
    end,
    on_calculate = function(self, stats)
        local D = stats.derived
        StatContext.multiply_cooldown_rate(stats, CooldownSet.ALL_ACTIONS, 0.8)
        StatContext.add_damage(stats, Apts.MELEE, -2)
        StatContext.add_defence(stats, Apts.MELEE, -3)
        D.movement_speed = D.movement_speed / 2
    end,
    on_deregister = function(self, stats)
       LogUtils.log_if_player(stats.obj, "$You {is}[are] no longer exhausted.", {200,200,255})
    end
}

-- BERSERKING
local function berserk_extension(effect)
    if effect.extensions < 1 then
        return 30
    elseif effect.extensions < 5 then
        return 20
    end
    return 5
end

local BERSERK_EXHAUSTION_DURATION = 275

M.Berserk = StatusType.define {
    TIME_LIMITED,
    init = function(self, stats, ...)
       self.base.init(self, stats, ...)
       LogUtils.log_if_player(stats.obj, "$You enter{s} a powerful rage!", {200,200,255})
    end,
    on_calculate = function(self, stats)
        local D = stats.derived

        -- Stat bonuses
        StatContext.add_damage(stats, Apts.MELEE, 2)
        StatContext.add_defence(stats, Apts.MELEE, 4 + D.level)

        -- Speed bonsues
        D.movement_speed = D.movement_speed + 1
        StatContext.multiply_cooldown_rate(stats, CooldownSet.MELEE_ACTIONS, 1.35)

        StatUtils.reset_rest_cooldown(stats)
    end,
    on_kill = function(self, stats)
        LogUtils.log_resolved(stats.obj, "<The >{$You's}[Your] rage grows ...", {200,200,255})
    end,
    on_deregister = function(self, stats)
       local B = stats.base
       StatusType.update_hook(B.hooks, M.Exhausted, stats, BERSERK_EXHAUSTION_DURATION)
    end
}

return M