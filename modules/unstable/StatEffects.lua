--- Define stats and cooldown effects. These are used to help define actions, which consist of a list of prerequisites and effects. 

local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

------ GENERIC STAT COST (Uses subtraction) --------------------------
M.StatCostEffect = newtype()
function M.StatCostEffect:init(stat_costs) 
    -- Takes mapping stat -> cost. Can take nested tables.
    self.stat_costs = stat_costs
end
function M.StatCostEffect:apply(user)
    StatContext.permanent_subtract(user, self.stat_costs)
end
------ GENERIC COOLDOWN COST (Uses addition) -------------------------
M.CooldownCostEffect = newtype()
function M.CooldownCostEffect:init(cooldown_costs, --[[Optional]] rate_aptitude, --[[Optional]] apt_worth) 
    -- Takes list of required cooldowns
    self.cooldown_costs = cooldown_costs
    self.rate_aptitude = rate_aptitude or false
    self.apt_worth = apt_worth or 0.05 -- 5% per aptitude point
end
function M.CooldownCostEffect:apply(user)
    local mult = 1.0
    if self.rate_aptitude then
        mult = mult + self.apt_worth * user.derived.aptitudes.effectiveness[self.rate_aptitude]
    end
    for k,v in values(self.cooldown_costs) do
        StatContext.add_cooldown(user, k, v)
    end
    return true
end

------ STATUS EFFECT CAUSATION ----------------------------------------
local function resolved_args(self)
    local resolved_args = {}
    for arg in values(self.args) do
        local is_function = (type(arg) == "function")
        table.insert(resolved_args, is_function and arg(user, target) or arg)
    end
    return unpack(resolved_args)    
end
--- To user of action:
M.UserStatusEffect = newtype()
function M.UserStatusEffect:init(status_type, ...)
    self.status_type = status_type
    self.args = {...}
end
function M.UserStatusEffect:apply(user, target)
    StatContext.update_status(user, self.status_type, unpack(resolved_args(self)))
end
--- To target of action:
M.TargetStatusEffect = newtype()
function M.TargetStatusEffect:init(status_type, ...)
    self.status_type = status_type
    self.args = {...}
end
function M.TargetStatusEffect:apply(user, target)
    StatContext.update_status(target, self.status_type, unpack(resolved_args(self)))
end

return M