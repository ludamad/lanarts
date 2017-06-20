EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"
DataW = require "DataWrapped"

DataW.spell_create {
    name: "Storm Cloud",
    description: ""
    types: {"White"}
    prereq_func: (caster) -> return true
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
    projectile: {
        speed: 5
        damage_multiplier: 2.00
    }
    mp_cost: 10
    spell_cooldown: 400
    cooldown: 35
}
