-- Routines for generating items 

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

function M.item_generate(group, only_with_shop_cost)
    local total_chance = 0
    for entry in values(group) do
        if shop_cost(entry) then
            total_chance = total_chance + entry.chance
        end
    end
    local num = random(1, total_chance)
    for entry in values(group) do
        if shop_cost(entry) then
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

return M