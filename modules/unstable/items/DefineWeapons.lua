local EquipmentTypes = import ".equipment_types"
local AptitudeTypes = import "@content.aptitude_types"
local Attacks = import "@Attacks"

local M = nilprotect {} -- Submodule

EquipmentTypes.weapon_define {
    name = "Dagger",
    weapon = "A small but sharp blade, adept at stabbing purposes.",
    attack = Attacks.attack_create(--[[Effectiveness]] 0, --[[Damage]] 5, {AptitudeTypes.melee, AptitudeTypes.piercing}),
    
}

return M