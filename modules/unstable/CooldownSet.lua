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

function CooldownSet:on_step(--[[Optional]] rates)
    rates = rates or self.cooldown_rates
    for k,v in pairs(self.cooldowns) do
        local rate = (rates[k] or 1)
        local val = v - rate
        if val <= 0 then val = 0 end
        self.cooldowns[k] = val
    end
end

function CooldownSet:__copy(b)
    if not rawget(b, "cooldowns") then
        table.clear(b) 
        setmetatable(b, CooldownSet)
        CooldownSet.init(b)
    else
        for c, v in pairs(b.cooldowns) do
            if not self.cooldowns[c] then b.cooldowns[c] = 0 end
        end
        for c, v in pairs(b.cooldown_rates) do
            if not self.cooldown_rates[c] then b.cooldowns[c] = 1 end
        end
    end
    for c, v in pairs(self.cooldowns) do b.cooldowns[c] = v end
    for c, v in pairs(self.cooldown_rates) do b.cooldown_rates[c] = v end
end

local MIN_DIVISOR = 0.25 -- No more than 4x needed cooldown 

function CooldownSet:apply_cooldown(type, cooldown, --[[Optional]] f)
    f = f or math.max
    self.cooldowns[type] = f(self.cooldowns[type] or 0, cooldown) 
end

function CooldownSet:add_cooldown(type, cooldown)
    local amount = cooldown * (self.cooldown_rates[type] or 1)
    self.cooldowns[type] = (self.cooldowns[type] or 0) + amount
    return amount 
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
