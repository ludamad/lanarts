local StatContext = import "@StatContext"

local CooldownTypes = import ".cooldown_types"

local M = nilprotect {} -- Submodule

function M.reset_rest_cooldown(stats)
    StatContext.set_cooldown(stats, CooldownTypes.REST_ACTION, CooldownTypes.REST_COOLDOWN_AMOUNT)
end

return M