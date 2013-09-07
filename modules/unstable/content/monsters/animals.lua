local MonsterType = import "@MonsterType"
local Stats = import "@Stats"

local AptitudeTypes = import "@content.aptitude_types"

local DefineUtils = import ".monster_define_utils"
local Traits = import ".monster_traits"
local Attacks = import "@Attacks"

local M = nilprotect {} -- Submodule

M.rat = DefineUtils.define {
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

    attacks = {Attacks.attack_create(0, 5, dup(AptitudeTypes.melee, 4))},

    appear_message = "A hostile large rat approaches!",
    defeat_message = "Blood splatters as the large rat is destroyed."
}

return M