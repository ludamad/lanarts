local EquipmentTypes = import ".EquipmentTypes"
local AptitudeTypes = import "@content.AptitudeTypes"
local Attacks = import "@Attacks"

local M = nilprotect {} -- Submodule

EquipmentTypes.weapon_define {
    name = "Dagger",
    weapon = "A small but sharp blade, adept at stabbing purposes.",
    attack = Attacks.attack_create(--[[Effectiveness]] 0, --[[Damage]] 5, {AptitudeTypes.melee, AptitudeTypes.piercing}),
    
}

return M