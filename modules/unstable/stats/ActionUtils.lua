local StatPrereqs = import "@StatPrereqs"
local StatEffects = import "@StatEffects"
local CooldownTypes = import ".CooldownTypes"

local M = nilprotect {} -- Submodule

-- Returns prerequisite+effect for the cooldowns found in a table
-- Returns nil if no cooldowns were detected.
function M.derive_cooldowns(args, --[[Optional, default false]] cleanup_members)
    for k,v in CooldownTypes.cooldown_pairs(args) do
        if cleanup_members then args[k] = nil end
        
    end
end

local function add_if_not_nil(action)

-- Derive different action components. This is used for items, spells, and miscellaneous abilities.
-- Components recognized by this routine:
--
--  Stat costs, derived from 'costs', 'mp_cost', 'hp_cost'
--    Table of stats that are required, and permanently lowered by the action.
--    Since by far this makes the most sense with hp & mp, there are two shortcuts here (Just mp_cost & hp_cost).
--
--  Cooldowns, derived from eg "[CooldownTypes.ALL_ACTIONS] = 40"
--    Cooldowns that are required to be 0, and the value that should be added to the cooldown timer.
--    Any 'parent cooldowns' (See CooldownTypes) are also added, unless explicitly referenced.
--    You can override this behaviour by setting the cooldown type to 0.

function M.resolve_action(args, --[[Optional, default false]] cleanup_members)
    local action = { 
        prerequisites = {}, effects = {},
        -- Optional arguments 
        name = args.name, sprite = args.sprite 
    }

    return action
end

return M