----
-- Overworld variant "badlands": rough country grown with a cellular automaton.
-- The valley floor starts as dry grass with rock scattered at random, then the classic cave rules
-- (rock with few rock neighbours crumbles, open ground hemmed in by rock fills) run until the
-- rock gathers into ridges and outcrops. Noise lays bare dirt over the floor and a few hardy
-- trees cling on. Separate basins are joined by trails.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :WALKABLE, :layout_area, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

ROCK = {matches_content: TILES.stone.id}

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area

    SourceMap.rectangle_apply {:map, area: {x1, y1, x2, y2}, fill_operator: paint(TILES.grass_dry), create_subgroup: false}
    -- Scatter rock: every square matches, so the rule's chance alone decides
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.stone, {neighbours: {}, chance: 0.49})
    }}
    -- Cave rules: rock needs company to stand, open ground surrounded by rock fills in.
    -- The map's edge counts as rock, so the badlands close in towards the forest
    SourceMap.automaton_apply {:map, :rng, :area, iterations: 5, outside_counts: true, rules: {
        paint(TILES.grass_dry, {matches_content: TILES.stone.id, neighbours: ROCK, max_neighbours: 3})
        paint(TILES.stone, {matches_none: {SourceMap.FLAG_SOLID}, neighbours: ROCK, min_neighbours: 5})
    }}
    -- Bare dirt in the dry stretches, grass where it's a little wetter
    on_floor = (fields) ->
        fields.matches_content = TILES.grass_dry.id
        return fields
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 14, octaves: 3, bands: {
        paint(TILES.dirt, on_floor {max: -0.25})
        {max: 0.45}
        paint(TILES.grass, on_floor {max: 1})
    }}
    -- The badlands give way to forest towards the rim, along a ragged line
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 12, octaves: 3, falloff: 0.45, bands: {
        paint(TILES.trees, {max: -0.45})
    }}
    -- A few hardy trees
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 3, octaves: 2, bands: {
        {max: 0.78}
        paint(TILES.trees, {max: 1, matches_none: {SourceMap.FLAG_SOLID}})
    }}

    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout, "craters")
