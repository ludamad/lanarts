local Apts = import "@stats.AptitudeTypes"
local Mons = import ".MonsterDefineUtils" 
local monster_define = Mons.monster_define 

monster_define {
    name = "Giant Rat",
    monster_kind = Mons.ANIMAL,
    description = "A giant, aggressive vermin.",
    appear_message = "A hostile large rat approaches!",
    defeat_message = "Blood splatters as the large rat is destroyed.",

    challenge_rating = 1,
    radius = 5,

    movement_speed = 2,
    hp = 10, hp_regen = 0.03, 

    effectiveness = 1, damage = 10,
    delay = 1.00
}