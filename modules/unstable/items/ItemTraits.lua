local M = {} -- Submodule

local equipment_types = {
    "WEAPON",
    "BODY_ARMOUR",
    "RING",
    "GLOVES",
    "BOOTS",
    "BRACERS",
    "AMULET",
    "HEADGEAR",
    "AMMUNITION",
}

local consumable_types = {
    "POTION", 
    "SCROLL"
}

for type in iter_combine(values(equipment_types), values(consumable_types)) do
    M[type] = type
end

return M