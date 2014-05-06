local Proficiency = import "@Proficiency"
local M = nilprotect {} -- Submodule

M._IDENTIFIED = {}

local ID_POINTS_PER_APT_POINT = 100
local ITEM_EQUIP_ID_POINT_BONUS = 300

function M.set_identification_points(stats, item_slot, val)
    local B = stats.base
    if item_slot.stackable then
        B.identify_table[item_slot.type] = val
    else
        B.identify_table[item_slot] = val
    end        
end

local function get_id_points(stats, item_slot)
    local B = stats.base
    if item_slot.stackable then
        return B.identify_table[item_slot.type]
    else
        return B.identify_table[item_slot]
    end        
end

local function ensure_identify_table(stats, item_slot)
    local B = stats.base
    if not B.identify_table then
        B.identify_table = setmetatable({}, {__mode='k', __copy = do_nothing})
    end
    if not get_id_points(stats, item_slot) then
        M.set_identification_points(stats, item_slot, 0)
    end
end

function M.is_identified(stats, item_slot)
    if not item_slot.needs_identification then 
        return true 
    end
    ensure_identify_table(stats, item_slot)
    local id_points = get_id_points(stats,item_slot)
    if id_points == M._IDENTIFIED then return true end
    if item_slot.equipped then
        id_points = id_points + ITEM_EQUIP_ID_POINT_BONUS 
    end
    local fail, total = Proficiency.resolve_proficiency_requirements(item_slot.identify_requirements or {}, stats)
    if id_points >= fail * ID_POINTS_PER_APT_POINT then
        if item_slot.on_identify then 
            item_slot:on_identify(stats) 
        end
        M.set_identification_points(stats, item_slot, M._IDENTIFIED)
        return true
    end
    return false
end

function M.add_identification_points(stats, item_slot, points)
    if not item_slot.needs_identification then 
        return
    end
    ensure_identify_table(stats, item_slot)
    local id_points = get_id_points(stats, item_slot)
    if id_points ~= M._IDENTIFIED then 
        M.set_identification_points(stats, item_slot, id_points + points)
    end
end

-- Accounts for identification.
-- TODO: Sprite
function M.name_and_description(stats, item_slot)
    if M.is_identified(stats, item_slot) then
        return item_slot.name, item_slot.description                
    else
        return item_slot.unidentified_name or item_slot.name, item_slot.unidentified_description or item_slot.description
    end
end

return M
