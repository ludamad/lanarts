local M = nilprotect {} -- Submodule

M.equipment_slot_capacities = {
    WEAPON = 1,
    BODY_ARMOUR = 1,
    RING = 2,
    GLOVES = 1,
    BOOTS = 1,
    BRACERS = 1,
    AMULET = 1,
    HEADGEAR = 1,
    AMMUNITION = 1
}

local consumable_types = {
    "POTION", 
    "SCROLL"
}
for type,cap in pairs(M.equipment_slot_capacities) do
    M[type] = type
end
for type in values(consumable_types) do
    M[type] = type
end

return M