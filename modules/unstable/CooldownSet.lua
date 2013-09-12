--- Cooldown types have a multiplier, which modifies any cooldown of that type.
-- As well, each cooldown type has a cooldown rate, which determines how quickly the
-- capability governed by the cooldown is met.

local Proficiency = import "@Proficiency"

local M = nilprotect {} -- Submodule

local CooldownSet = newtype()

function CooldownSet:init()
	self.cooldowns = {}
	self.cooldown_rates = {}
	self.cooldown_multipliers = {}
end

function CooldownSet:on_step()
    for k,v in pairs(self.cooldowns) do
        local rate = (self.cooldown_rates[k] or 1)
        local val = v - rate
        if val <= 0 then
            val = nil -- Delete
        end
        self.cooldowns[k] = val
    end
end

local MIN_DIVISOR = 0.25 -- No more than 4x needed cooldown 

local function resolve_action_cooldown(self, s, c, should_add)
    local divisor = 1.0 + Proficiency.calculate_proficiency(s, c.multipliers)
    local mult = 1.0/math.max(MIN_DIVISOR, divisor)
    for type, amount in pairs(c.base_cooldowns) do
        local val = should_add and self.cooldowns[type] or 0
        self.cooldowns[type] = val + amount * mult
    end
end

function CooldownSet:add_action_cooldown(stats, action_cooldown)
    resolve_action_cooldown(self, stats, action_cooldown, true)
end

function CooldownSet:set_action_cooldown(stats, action_cooldown)
    resolve_action_cooldown(self, stats, action_cooldown, false)
end

function CooldownSet:has_cooldown(type)
    local v = self.cooldowns[type]
    return (v ~= nil and v > 0) 
end

function CooldownSet:get_cooldown(type)
    return self.cooldowns[type] or 0
end

local function table_multiply_defaulted(table, key, multiplier)
    table[key] = (table[key] or 1) * multiplier
end

function CooldownSet:multiply_cooldown_multiplier(type, amount)
    table_multiply_defaulted(self.cooldown_multipliers, type, amount)
end

function CooldownSet:multiply_cooldown_rate(type, amount)
    table_multiply_defaulted(self.cooldown_rates, type, amount)

end

M.create = CooldownSet.create

function M.cooldown_create

function M.action_cooldown_create(base_cooldowns, multipliers)
    if #multipliers > 0 then 
        multipliers = Proficiency.proficiency_type_create(multipliers)
    end
    return {base_cooldowns = base_cooldowns, multipliers = multipliers}
end

return M