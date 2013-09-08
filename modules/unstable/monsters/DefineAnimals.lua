local Apts = import "@stats.AptitudeTypes"

local Utils,Traits = import ".MonsterUtils", import ".MonsterTraits"

local M = nilprotect {} -- Submodule

M.rat = Utils.monster_define {
    name = "Giant Rat",
    description = "A giant, aggressive vermin.",
    traits = {Traits.ANIMAL},

    xp_award = 5,
    radius = 14,

    stats = {
        level = 1,
        hp = 150, 
        hp_regen = 0.03, 
        movement_speed = 2
    },

    unarmed_attack = {--[[Effectiveness]] 0, --[[Damage]] 5, Apts.MELEE},

    appear_message = "A hostile large rat approaches!",
    defeat_message = "Blood splatters as the large rat is destroyed."
}

return M
