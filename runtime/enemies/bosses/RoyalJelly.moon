DataW = require "DataWrapped"
{:draw_boss_name} = require "enemies.bosses.BossUtils"

DataW.enemy_create {
    name: "Royal Jelly"
    appear_message: "The Royal Jelly quivers with rage!"
    defeat_message: "The Royal Jelly splatters apart!"
    sprite: "spr_bosses.royal_jelly"
    radius: 20
    xpaward: 120
    unique: true
    types: {"Green"}
    stats: {
        attacks: {{weapon: "Basic Melee"}}
        hp: 160
        hpregen: 0.1
        movespeed: 2
        strength: 18
        magic: 15
        defence: 8
        willpower: 8
    }
    on_post_draw: draw_boss_name
}
