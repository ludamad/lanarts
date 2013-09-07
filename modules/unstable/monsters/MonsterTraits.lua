local StatContext = import "@StatContext"
local AptitudeTypes = import "@content.aptitude_types"

local M = nilprotect {} -- Submodule

M.stat_mod_functions = {
    ANIMAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_resistance(context, AptitudeTypes.slashing, -2, --[[Permanent]] true)
        StatContext.add_defence(context, AptitudeTypes.slashing, -2, --[[Permanent]] true)
    end,
    HUMANOID = do_nothing,
}

-- Export keys as traits

for k,v in pairs(M.stat_mod_functions) do
    M[k] = k
end

return M