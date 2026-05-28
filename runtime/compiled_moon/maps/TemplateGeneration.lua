local SourceMap = require("core.SourceMap")
local MapUtils = require("maps.MapUtils")
local Vaults = require("maps.Vaults")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_TEMPORARY, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_TEMPORARY, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_TEMPORARY, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local place_feature
place_feature = function(compiler, node, template)
  local map = compiler.map
  local attempt_placement
  attempt_placement = function(template)
    local orient = map.rng:random_choice({
      SourceMap.ORIENT_DEFAULT,
      SourceMap.ORIENT_FLIP_X,
      SourceMap.ORIENT_FLIP_Y,
      SourceMap.ORIENT_TURN_90,
      SourceMap.ORIENT_TURN_180,
      SourceMap.ORIENT_TURN_270
    })
    local w, h
    do
      local _obj_0 = template.size
      w, h = _obj_0[1], _obj_0[2]
    end
    if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180 then
      w, h = h, w
    end
    local bbox = compiler:as_bbox(node, math.max(w, h))
    local top_left_xy = MapUtils.random_square(map, bbox, {
      matches_group = compiler:as_group_set(node)
    })
    local apply_args = {
      map = map,
      top_left_xy = top_left_xy,
      orientation = orient
    }
    if template:matches(apply_args) then
      template:apply(apply_args)
      return true
    end
    return false
  end
  local attempt_placement_n_times
  attempt_placement_n_times = function(template, n)
    for i = 1, n do
      if attempt_placement(template) then
        return true
      end
    end
    return false
  end
  if attempt_placement_n_times(template, 100) then
    return true
  end
  return false
end
return {
  place_feature = place_feature
}
