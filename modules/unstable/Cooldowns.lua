--- Cooldown types have a multiplier, which modifies any cooldown of that type.
-- As well, each cooldown type has a cooldown rate, which determines how quickly the
-- capability governed by the cooldown is met.

local M = nilprotect {} -- Submodule

local Cooldowns = newtype()

function Cooldowns:init()
	self.cooldowns = {}
	self.cooldown_rates = {}
	self.cooldown_multipliers = {}
end

function Cooldowns:on_step()
    for k,v in pairs(self.cooldowns) do
        local rate = (self.cooldown_rates[k] or 1)
        local val = v - rate
        if val <= 0 then
            val = nil -- Delete
        end
        self.cooldowns[k] = val
    end
end


function Cooldowns:add_cooldown(type, amount)
    local mult = (self.cooldown_multipliers[type] or 1)
    local val = (self.cooldowns[type] or 0) + amount * mult
    self.cooldowns[type] = val
end

function Cooldowns:set_cooldown(type, amount)
    local mult = (self.cooldown_multipliers[type] or 1)
    self.cooldowns[type] = amount * mult
end

function Cooldowns:has_cooldown(type)
    local v = self.cooldowns[type]
    return (v ~= nil and v > 0) 
end

function Cooldowns:get_cooldown(type)
    return self.cooldowns[type] or 0
end

local function table_multiply_defaulted(table, key, multiplier)
    table[key] = (table[key] or 1) * multiplier
end

function Cooldowns:multiply_cooldown_multiplier(type, amount)
    table_multiply_defaulted(self.cooldown_multipliers, type, amount)
end

function Cooldowns:multiply_cooldown_rate(type, amount)
    table_multiply_defaulted(self.cooldown_rates, type, amount)

end

M.create = Cooldowns.create

-- Cooldown types
M.ALL_ACTIONS = "all_actions_cooldown"
M.OFFENSIVE_ACTIONS = "offensive_actions_cooldown"
M.MELEE_ACTIONS = "melee_actions_cooldown"
M.SPELL_ACTIONS = "spell_actions_cooldown"

return M