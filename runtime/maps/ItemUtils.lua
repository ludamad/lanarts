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

local function nested_chance(group, attribute) 
    if group[attribute] then
        return group
    end
    local total_chance = 0 
    for _,entry in ipairs(group) do
        total_chance = total_chance + entry.chance
    end
    local rand_x = randomf(0, total_chance)
    for _,entry in ipairs(group) do
        rand_x = rand_x - entry.chance
        if rand_x <= 0 then
            return nested_chance(entry, attribute)
        end
    end
    return nil
end

local RANDART_CHANCE = 1
function M.item_generate(group, only_with_shop_cost, --[[Optional]] randart_power_level, --[[Optional]] randart_chance)
    randart_chance = randart_chance or RANDART_CHANCE
    -- For now, a fixed 1 in 100 chance of being a randart
    if true then -- randart_power_level ~= nil then
        if randomf() <= randart_chance / 100 then
            randart_power_level = randart_power_level or 1
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
    local entry = nested_chance(group, "item")
    if not entry then
        return nil
    end
    local amount = resolve_range(entry.amount)
    return { type = entry.item, amount = amount}
end

function M.item_object_generate(map, group, --[[Optional]] randart_power_level)
    local item = M.item_generate(group, randart_power_level)
    map_utils.random_item(map, item.type, item.amount)
end

return M
