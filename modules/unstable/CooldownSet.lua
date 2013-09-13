--- Cooldown types have a multiplier, which modifies any cooldown of that type.
-- As well, each cooldown type has a cooldown rate, which determines how quickly the
-- capability governed by the cooldown is met.

local Proficiency = import "@Proficiency"

local M = nilprotect {} -- Submodule

local CooldownSet = newtype()

function CooldownSet:init()
	self.cooldowns = {}
	self.cooldown_rates = {}
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

function CooldownSet:apply_cooldown(type, cooldown, --[[Optional]] f)
    f = f or math.max
    self.cooldowns[type] = f(self.cooldowns[type] or 0, cooldown) 
end

function CooldownSet:add_cooldown(type, cooldown)
    self.cooldowns[type] = self.cooldowns[type] or 0 + cooldown 
end

function CooldownSet:has_cooldown(type)
    local v = self.cooldowns[type]
    return (v ~= nil and v > 0) 
end

function CooldownSet:get_cooldown(type)
    return self.cooldowns[type] or 0
end

function CooldownSet:multiply_cooldown_rate(type, v)
    self.cooldown_rates[type] = (self.cooldown_rates[type] or 1) * v
end

M.create = CooldownSet.create

return M