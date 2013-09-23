--- Define stats and cooldown effects. These are used to help define actions, which consist of a list of prerequisites and effects. 

local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

------ GENERIC STAT COST (Uses subtraction) --------------------------
local StatCost = newtype()
function StatCost:init(stat_costs) 
    -- Takes mapping stat -> cost. Can take nested tables.
    self.stat_costs = stat_costs
end
function StatCost:apply(user)
    table.defaulted_subtraction(user.derived, self.stat_costs)
end
function M.is_stat_cost(cost)  -- API
    return getmetatable(cost) == StatCost 
end
M.stat_cost_create = StatCost.create -- API

------ GENERIC COOLDOWN COST (Uses addition) -------------------------
local CooldownCost = newtype() --
function CooldownCost:init(cooldown_costs) 
    -- Takes list of required cooldowns
    self.cooldown_costs = cooldown_costs
end
function CooldownCost:apply(user)
    for k,v in values(self.cooldown_costs) do
        StatContext.add_cooldown(user, k, v)
    end
    return true
end
function M.is_cooldown_cost(cost) -- API
    return getmetatable(cost) == CooldownCost 
end
M.cooldown_cost_create = CooldownCost.create -- API

return M