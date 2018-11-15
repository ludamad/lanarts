----
-- Placement functions for vaults defined in Vaults.moon
----
SourceMap = require "core.SourceMap"
MapUtils = require "maps.MapUtils"

N_TRIES = 100

attempt_placement = (region_set, template) ->
    {:map, :regions} = region_set
    orient = map.rng\random_choice {
        SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y,
        SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
    }
    event_log("(RNG #%d) orient=%d", map.rng\amount_generated(), orient)
    for r in *regions
        {w, h} = template.size
        -- Account for rotation in size:
        if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180
            w, h = h, w
        {x1, y1, x2, y2} = r\bbox()
        -- Expand just outside the bounds of the region:
        x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
        -- Ensure we are within the bounds of the world map:
        x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
        top_left_xy = MapUtils.random_square(map, {x1, y1, x2, y2})
        apply_args = {:map, :top_left_xy, orientation: orient }
        if template\matches(apply_args)
            template\apply(apply_args)
            return true
    return false
-- Function to try placement n times, returns success:
attempt_placement_n_times = (region_set, template, n) ->
    for i=1,n
        event_log("(RNG #%d) placement=%d", region_set.map.rng\amount_generated(), i)
        if attempt_placement(region_set, template)
            return true
    return false

place_vault_in = (region_set, vault, n_tries=N_TRIES) ->
    vault.map = region_set.map
    template = SourceMap.area_template_create(vault)
    return attempt_placement_n_times(region_set, template, n_tries)

place_vault = (map, vault, n_tries) ->
    regions = map.regions
    return place_vault({:map, :regions}, vault, n_tries)

return nilprotect {
    :place_vault
    :place_vault_in
}
