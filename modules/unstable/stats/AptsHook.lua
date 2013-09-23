-- A permanent hook for all stat contexts in Lanarts.
-- Performs additional configuring of derived stats.

local M = nilprotect {} -- Submodule

M.hook = {
--    init = function(self, stats, ...)
--       self.base.init(self, stats, ...)
--       LogUtils.event_log_player(stats.obj, "$You enter{s} a powerful rage!", {200,200,255})
--    end,
--    on_step = function(self, stats, ...)
--    
--    end,
--    on_calculate = function(self, stats)
--        -- Derive additional stats
--
--        -- Offensive cooldown rate
--        multiply_cooldown_rate
--    end
--    on_calculate = function(self, stats)
--        local D = stats.derived
--
--        -- Stat bonuses
--        StatContext.add_damage(stats, Apts.MELEE, 2)
--        StatContext.add_defence(stats, Apts.MELEE, 4 + D.level)
--
--        -- Speed bonsues
--        D.movement_speed = D.movement_speed + 1
--        StatContext.multiply_cooldown_rate(stats, Cooldowns.MELEE_ACTIONS, 1.35)
--
--        CooldownUtils.reset_rest_cooldown(stats)
--    end,
--    on_kill = function(self, stats)
--        LogUtils.event_log_resolved(stats.obj, "<The >{$You's}[Your] rage grows ...", {200,200,255})
--    end,
--    on_deregister = function(self, stats)
--       local B = stats.base
--       StatusType.update_hook(B.hooks, M.Exhausted, stats, BERSERK_EXHAUSTION_DURATION)
--    end
}

return M