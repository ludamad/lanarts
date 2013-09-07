local StatContext = import "@StatContext"
local AptitudeTypes = import "@content.aptitude_types"

local M = nilprotect {} -- Submodule

M.ANIMAL = "animal" 
local function animal_stat_mod(stats)
    local context = StatContext.stat_context_create(stats)
    StatContext.add_resistance(context, AptitudeTypes.slashing, -2, --[[Permanent]] true)
    StatContext.add_defence(context, AptitudeTypes.slashing, -2, --[[Permanent]] true)
end

M.HUMANOID = "humanoid"
local function humanoid_stat_mod(stats)
end

M.stat_mod_functions = {
    [M.ANIMAL] = animal_stat_mod,
    [M.HUMANOID] = humanoid_stat_mod,
}

return M