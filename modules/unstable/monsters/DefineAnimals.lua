local Apts = import "@stats.AptitudeTypes"
local animal_define = (import ".MonsterDefineUtils").animal_define 

animal_define {
    name = "Giant Rat",
    description = "A giant, aggressive vermin.",
    appear_message = "A hostile large rat approaches!",
    defeat_message = "Blood splatters as the large rat is destroyed.",

    xp_award = 5,
    radius = 14,

    level = 1, movement_speed = 2,
    hp = 150, hp_regen = 0.03, 

    effectiveness = 1, damage = 10, 
    delay = 1.4
}