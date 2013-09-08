local Apts = import "@stats.AptitudeTypes"
local Traits,Utils = import ".ItemTraits", import ".ItemUtils"

Utils.weapon_define {
    name = "Dagger",
    description = "A small but sharp blade, adept at stabbing purposes.",
    attack = {--[[Effectiveness]] 6, --[[Damage]] 4, {Apts.MELEE, Apts.PIERCING}}
}