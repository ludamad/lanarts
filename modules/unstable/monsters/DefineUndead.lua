local Apts = import "@stats.AptitudeTypes"
local Mons = import ".MonsterDefineUtils" 
local monster_define = Mons.monster_define 

monster_define {
    name = "Gnome Skeleton",
    sprite = "sprites/gnome_skeleton.png%32x32",
    monster_kind = Mons.UNDEAD,
    description = ".",

    challenge_rating = 1,
    radius = 5,

    movement_speed = 1,
    hp = 30, hp_regen = 0.00, 

    effectiveness = 1, damage = 10,
    delay = 1.00
}