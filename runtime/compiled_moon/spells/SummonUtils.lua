local SpellObjects = require("objects.SpellObjects")
local GameObject = require("core.GameObject")
local summon_one_hit_monster
summon_one_hit_monster = function(caster, xy, monster, time_limit)
  if time_limit == nil then
    time_limit = 60
  end
  local mon = GameObject.enemy_create({
    type = monster,
    xy = xy,
    team = caster.team
  })
  mon:add_effect("DiesOnEndOrFirstAttack", time_limit)
  return mon
end
return {
  summon_one_hit_monster = summon_one_hit_monster
}
