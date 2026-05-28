local compile_bonuses, BONUSES
do
  local _obj_0 = require("items.Bonuses")
  compile_bonuses, BONUSES = _obj_0.compile_bonuses, _obj_0.BONUSES
end
require("items.BonusesArmour")
require("items.BonusesColor")
require("items.BonusesRing")
require("items.BonusesWeapon")
require("items.BonusesSummons")
require("items.BonusesStats")
local KEY_SPRITES = {
  "spr_rings.agate",
  "spr_rings.brass",
  "spr_rings.bronze",
  "spr_rings.clay",
  "spr_rings.copper",
  "spr_rings.coral",
  "spr_rings.diamond",
  "spr_rings.emerald",
  "spr_rings.ethereal",
  "spr_rings.glass",
  "spr_rings.gold_blue",
  "spr_rings.gold_green",
  "spr_rings.gold",
  "spr_rings.gold_red",
  "spr_rings.granite",
  "spr_rings.iron",
  "spr_rings.ivory",
  "spr_rings.jade",
  "spr_rings.moonstone",
  "spr_rings.opal",
  "spr_rings.pearl",
  "spr_rings.plain_black",
  "spr_rings.plain_red",
  "spr_rings.plain_yellow",
  "spr_rings.ruby",
  "spr_rings.silver",
  "spr_rings.steel",
  "spr_rings.tiger_eye",
  "spr_rings.tourmaline",
  "spr_rings.wooden"
}
local generate
generate = function(name)
  local parts = name:split(" ")
  local i = 1
  local bonuses = { }
  while i <= #parts do
    local potential_bonus = parts[i]
    if not BONUSES[potential_bonus] then
      break
    end
    append(bonuses, potential_bonus)
    i = i + 1
  end
  local base_name = table.concat((function()
    local _accum_0 = { }
    local _len_0 = 1
    for j = i, #parts do
      _accum_0[_len_0] = parts[j]
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)(), ' ')
  local base_obj = items[base_name]
  if not base_obj then
    return
  end
  local definer_method = Data[base_obj.__method]
  local created = compile_bonuses(base_obj, bonuses)
  local name_hash = created.name:hash()
  if created.type == "ring" then
    created.spr_item = KEY_SPRITES[name_hash % #KEY_SPRITES + 1]
  end
  return definer_method(created)
end
return {
  generate = generate
}
