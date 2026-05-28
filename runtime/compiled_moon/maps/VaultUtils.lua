local SourceMap = require("core.SourceMap")
local MapUtils = require("maps.MapUtils")
local N_TRIES = 100
local attempt_placement
attempt_placement = function(region_set, template)
  local map, regions
  map, regions = region_set.map, region_set.regions
  local orient = map.rng:random_choice({
    SourceMap.ORIENT_DEFAULT,
    SourceMap.ORIENT_FLIP_X,
    SourceMap.ORIENT_FLIP_Y,
    SourceMap.ORIENT_TURN_90,
    SourceMap.ORIENT_TURN_180,
    SourceMap.ORIENT_TURN_270
  })
  event_log("(RNG #%d) orient=%d", map.rng:amount_generated(), orient)
  for _index_0 = 1, #regions do
    local r = regions[_index_0]
    local w, h
    do
      local _obj_0 = template.size
      w, h = _obj_0[1], _obj_0[2]
    end
    if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180 then
      w, h = h, w
    end
    local x1, y1, x2, y2
    do
      local _obj_0 = r:bbox()
      x1, y1, x2, y2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
    end
    x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
    x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
    local top_left_xy = MapUtils.random_square(map, {
      x1,
      y1,
      x2,
      y2
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
  end
  return false
end
local attempt_placement_n_times
attempt_placement_n_times = function(region_set, template, n)
  for i = 1, n do
    event_log("(RNG #%d) placement=%d", region_set.map.rng:amount_generated(), i)
    if attempt_placement(region_set, template) then
      return true
    end
  end
  return false
end
local place_vault_in
place_vault_in = function(region_set, vault, n_tries)
  if n_tries == nil then
    n_tries = N_TRIES
  end
  vault.map = region_set.map
  local template = SourceMap.area_template_create(vault)
  return attempt_placement_n_times(region_set, template, n_tries)
end
local place_vault
place_vault = function(map, vault, n_tries)
  local regions = map.regions
  return place_vault_in({
    map = map,
    regions = regions
  }, vault, n_tries)
end
return nilprotect({
  place_vault = place_vault,
  place_vault_in = place_vault_in
})
