--- Define stat and cooldown requirements, divided into prerequisites and effects.
-- These are used to help define actions, which consist of a list of prerequisites and effects. 

local StatContext = import "@StatContext"

-- GENERIC STAT REQUIREMENT (>= stat requirement)

local M = nilprotect {} -- Submodule

local function resolve_stat_prereq(prereqs, derived_stats)
    for k,v in prereqs do
        if type(v) == "table" and not resolve_stat_prereq(v, derived_stats[k]) then
            return false
        end
        if derived_stats[k] < v then
            return false
        end
    end
    return true
end

local StatPrereq = newtype()
function StatPrereq:init(stat_prereqs) 
    -- Takes mapping stat -> prereq. Can take nested tables.
    self.stat_prereqs = stat_prereqs
end
function StatPrereq:check(user) 
    return resolve_stat_prereq(self.stat_prereqs, user.derived)
end

-- Exposed API:
function M.is_stat_prereq(prereq) 
    return getmetatable(prereq) == StatPrereq 
end
M.stat_prereq_create = StatPrereq.create

-- GENERIC STAT COST (Uses subtraction)

local StatCost = newtype()
function StatCost:init(stat_costs) 
    -- Takes mapping stat -> cost. Can take nested tables.
    self.stat_costs = stat_costs
end
function StatCost:apply(user)
    table.defaulted_subtraction(user.derived, self.stat_costs)
end

-- Exposed API:
function M.is_stat_cost(cost) 
    return getmetatable(cost) == StatCost 
end
M.stat_cost_create = StatCost.create

-- GENERIC COOLDOWN REQUIREMENT (<= 0)

local CooldownPrereq = newtype()
function CooldownPrereq:init(cooldown_prereq_list) 
    -- Takes list of required cooldowns
    self.cooldown_prereq_list = cooldown_prereq_list
end
function CooldownPrereq:check(user)
    for c in values(self.cooldown_prereq_list) do
        if StatContext.has_cooldown(user, c) then
            return false
        end
    end
    return true
end

-- Exposed API:
function M.is_cooldown_prereq(prereq) 
    return getmetatable(prereq) == CooldownPrereq 
end
M.cooldown_prereq_create = CooldownPrereq.create

-- GENERIC COOLDOWN COST (Uses addition)

local CooldownCost = newtype()
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

-- Exposed API:
function M.is_cooldown_cost(cost) 
    return getmetatable(cost) == CooldownCost 
end
M.cooldown_cost_create = CooldownCost.create

return M