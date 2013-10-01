local Apts = import "@stats.AptitudeTypes"
local animal_define = (import ".MonsterDefineUtils").animal_define 

animal_define {
    name = "Giant Rat",
    description = "A giant, aggressive vermin.",
    appear_message = "A hostile large rat approaches!",
    defeat_message = "Blood splatters as the large rat is destroyed.",

    challenge_rating = 5,
    radius = 14,

    movement_speed = 2,
    hp = 30, hp_regen = 0.03, 

    effectiveness = 1, damage = 10, 
    delay = 1.00
}