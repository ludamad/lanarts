-- Routines for generating items 

local map_utils = require "maps.MapUtils"
local Randarts = require "items.Randarts"
local GlobalData = require "core.GlobalData"

local M = {} -- Submodule

local function resolve_range(r)
    if type(r) == "table" then
        return random(r)
    end
    -- else
    return r
end

local function shop_cost(entry) 
    return items[entry.item].shop_cost
end

local RANDART_CHANCE = 1
function M.item_generate(group, only_with_shop_cost, --[[Optional]] randart_power_level)
    -- For now, a fixed 1 in 100 chance of being a randart
    if true then -- randart_power_level ~= nil then
        if randomf() <= RANDART_CHANCE / 100 then
            randart_power_level = 1
            while randomf() < 0.05 and randart_power_level < 3 do
                randart_power_level = randart_power_level + 1
            end
            local choice = nil
            while true do
                choice = random_choice(Randarts.RANDARTS[randart_power_level])
                --if not GlobalData.randarts_generated[choice] then
                --    GlobalData.randarts_generated[choice] = true
                --    break
                --end
                break -- TODO reconsider randart generation
            end
            return {type = choice, amount = 1}
        end
    end
    local total_chance = 0
    for entry in values(group) do
        if not only_with_shop_cost or shop_cost(entry) then
            total_chance = total_chance + entry.chance
        end
    end
    local num = random(1, total_chance)
    for entry in values(group) do
        if not only_with_shop_cost or shop_cost(entry) then
            num = num - entry.chance
            if num <= 0 then
                local amount = resolve_range(entry.amount)
                return { type = entry.item, amount = amount}
            end
            total_chance = total_chance + entry.chance
        end
    end
    return nil
end

function M.item_object_generate(map, group, --[[Optional]] randart_power_level)
    local item = M.item_generate(group, randart_power_level)
    map_utils.random_item(map, item.type, item.amount)
end

return M
