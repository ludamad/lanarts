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
    if group.randart_list then
        local type = random_choice(group)
        assert(type, "Got nil from random choice!")
        local amount = 1
        if items[type].type == "projectile" then
            amount = random(15,33)
        end
        return {[attribute] = type, amount = amount}
    end
    local total_chance = 0 
    for i=1,#group do
        if group[i] ~= nil then
            if not group[i].chance then
                pretty(group[i])
            end
            total_chance = total_chance + group[i].chance
        end
    end
    if total_chance == 0 then
        pretty(group)
    end
    assert(total_chance ~= 0)
    local rand_x = random(0, total_chance)
    for i=1,#group do
        if group[i] ~= nil then
            rand_x = rand_x - group[i].chance
            if rand_x <= 0 then
                return nested_chance(group[i], attribute)
            end
        end
    end
    pretty(group)
    return nil
end

-- Does not generate randarts (other than those provided in 'group'):
function M.raw_item_generate(group)
    local entry = nested_chance(group, "item")
    if not entry then
        return nil
    end
    local amount = resolve_range(entry.amount)
    return { type = entry.item, amount = amount}
end

function M.randart_generate(randart_power_level) 
    local ItemGroups = require "maps.ItemGroups"
    local group = require("maps.ItemGroups").randart_items[randart_power_level]
    return M.raw_item_generate(group)
end

local RANDART_CHANCE = 2
local LANARTS_ONLY_RANDARTS = (os.getenv("LANARTS_RANDARTS") ~= nil)
function M.item_generate(group, --[[Optional]] randart_power_level, --[[Optional]] randart_chance)
    randart_chance = randart_chance or RANDART_CHANCE
    randart_power_level = randart_power_level or 1
    if LANARTS_ONLY_RANDARTS or randomf() <= randart_chance / 100 then
        return M.randart_generate(randart_power_level)
    end
    return M.raw_item_generate(group)
end

function M.item_object_generate(map, group, --[[Optional]] randart_power_level)
    local item = M.item_generate(group, randart_power_level)
    map_utils.random_item(map, item.type, item.amount)
end

return M
