SourceMap = require "core.SourceMap"

MapUtils = require "maps.MapUtils"
Vaults = require "maps.Vaults"
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE, :FLAG_TEMPORARY
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

place_feature = (compiler, node, template) ->
   map = compiler.map
   -- Function to try a single placement, returns success:
   attempt_placement = (template) ->
       orient = map.rng\random_choice {
           SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y,
           SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
       }
       {w, h} = template.size
       -- Account for rotation in size:
       if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180
           w, h = h, w
       bbox = compiler\as_bbox(node, math.max(w,h))
       top_left_xy = MapUtils.random_square map, bbox, {
           matches_group: compiler\as_group_set node
       }
       apply_args = {:map, :top_left_xy, orientation: orient}
       if template\matches(apply_args)
           template\apply(apply_args)
           return true
       return false
   -- Function to try placement n times, returns success:
   attempt_placement_n_times = (template, n) ->
       for i=1,n
           if attempt_placement(template)
               return true
       return false
   -- Try to create the template object using our placement routines:
   if attempt_placement_n_times(template, 100)
       -- Exit, as we have handled the first overworld component
       return true
   return false

return {:place_feature}
