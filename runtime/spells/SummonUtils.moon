SpellObjects = require "objects.SpellObjects"
GameObject = require "core.GameObject"

summon_one_hit_monster = (caster, xy, monster, time_limit=60) ->
    mon = GameObject.enemy_create {type: monster, xy: xy, team: caster.team}
    mon\add_effect "DiesOnEndOrFirstAttack", time_limit
    return mon

return {:summon_one_hit_monster}
