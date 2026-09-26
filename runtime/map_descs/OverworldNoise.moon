----
-- Overworld variant "wildlands": terrain from layered noise.
-- Elevation decides forest (low), meadow (middle) and rocky ridges (high), falling off into forest
-- at the edges. Moisture varies the meadows between lush grass, dry grass and bare dirt, and a fine
-- detail field scatters small copses.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :WALKABLE, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    seed = () -> rng\random(0, 2^30)

    -- Elevation: the map starts as forest, so the lowest band leaves it be
    SourceMap.noise_apply {:map, :area, seed: seed(), scale: 26, octaves: 4, persistence: 0.5, falloff: 0.15, bands: {
        {max: -0.06}
        paint(TILES.grass, {max: 0.55})
        paint(TILES.stone, {max: 1})
    }}
    -- Moisture, over the meadows only
    on_grass = (fields) ->
        fields.matches_content = TILES.grass.id
        return fields
    SourceMap.noise_apply {:map, :area, seed: seed(), scale: 18, octaves: 3, bands: {
        paint(TILES.dirt, on_grass {max: -0.5})
        paint(TILES.grass_dry, on_grass {max: -0.15})
        {max: 0.3}
        paint(TILES.grass_lush, on_grass {max: 1})
    }}
    -- Copses: small stands of trees out on open ground
    SourceMap.noise_apply {:map, :area, seed: seed(), scale: 3.5, octaves: 2, bands: {
        {max: 0.72}
        paint(TILES.trees, {max: 1, matches_none: {SourceMap.FLAG_SOLID}})
    }}
    -- Fill pinholes: open squares walled in on nearly every side become forest
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.trees, {matches_none: {SourceMap.FLAG_SOLID}, neighbours: {matches_all: {SourceMap.FLAG_SOLID}}, min_neighbours: 7})
    }}

    forest_rim(map, rng, area, 0.4)
    -- Meadows of any size are joined up by paths; only specks are lost to the forest
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout, "ringwood")
