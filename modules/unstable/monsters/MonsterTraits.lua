local StatContext = import "@StatContext"
local Apts = import "@stats.AptitudeTypes"

local M = nilprotect {} -- Submodule

M.stat_mod_functions = {
    ANIMAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_resistance(context, Apts.SLASHING, -2, --[[Permanent]] true)
        StatContext.add_defence(context, Apts.SLASHING, -2, --[[Permanent]] true)
    end,
    HUMANOID = do_nothing,
}

-- Export keys as traits

for k,v in pairs(M.stat_mod_functions) do
    M[k] = k
end

return M