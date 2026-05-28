local ObjectUtils = require("objects.ObjectUtils")
local DataW = require("DataWrapped")
require("spells.DefineFireSpells")
require("spells.DefineWhiteSpells")
local SummonUtils = require("spells.SummonUtils")
local TypeEffectUtils = require("spells.TypeEffectUtils")
return DataW.spell_create({
  name = "Skeleton Army",
  types = {
    "Black"
  },
  description = "Briefly summon an army of skeletons. Grow stronger based on caster's willpower.",
  can_cast_with_held_key = true,
  spr_spell = "spr_enemies.undead.skeleton_ugly_thing",
  can_cast_with_cooldown = false,
  mp_cost = 10,
  cooldown = 25,
  spell_cooldown = 550,
  fallback_to_melee = false,
  prereq_func = function(obj)
    return true
  end,
  action_func = function(obj)
    play_sound("sound/lifelink.ogg")
    local n_summoned = 0
    local bpower = TypeEffectUtils.get_power(obj, "Black")
    return ObjectUtils.nearby_square_iterate(obj, function(xy)
      local mon = SummonUtils.summon_one_hit_monster(obj, xy, "SummonedSkeleton", 100 + bpower * 10)
      mon.stats.strength = obj.stats.willpower / 4
      n_summoned = n_summoned + 1
      return n_summoned >= 10 + obj.stats.level * 3 + bpower * 2
    end)
  end
})
