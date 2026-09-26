-- A room to stage a boss fight in (see engine.StartBossShowcase): the party on the left, the
-- monsters on the right, floored and walled in a tileset from tiles/Tilesets.lua.
SourceMap = require "core.SourceMap"
NewMaps = require "maps.NewMaps"
MapUtils = require "maps.MapUtils"
Tilesets = require "tiles.Tilesets"
World = require "core.World"

ROOM = {20, 14}
PADDING = 6

-- -> game map, player spawn points
generate = (monsters, tileset_name = "pebble") ->
    tileset = Tilesets[tileset_name] or error("No tileset '#{tileset_name}'")
    {w, h} = ROOM
    size = {w + 2 * PADDING, h + 2 * PADDING}
    rng = require("mtwist").create(random(0, 2^30))
    map = SourceMap.map_create {
        :rng, :size, content: tileset.wall, flags: SourceMap.FLAG_SOLID, map_label: "Boss Arena"
        instances: {}, player_candidate_squares: {}, post_maps: {}, wandering_enabled: false
    }
    x1, y1 = PADDING, PADDING
    SourceMap.rectangle_apply {
        :map, area: {x1, y1, x1 + w, y1 + h}, create_subgroup: false
        fill_operator: {add: SourceMap.FLAG_SEETHROUGH, remove: SourceMap.FLAG_SOLID, content: tileset.floor}
    }
    -- A band of the tileset's other floor down the middle, to set the stage
    SourceMap.rectangle_apply {
        :map, area: {x1 + 2, y1 + math.floor(h / 2) - 2, x1 + w - 2, y1 + math.floor(h / 2) + 2}, create_subgroup: false
        fill_operator: {content: tileset.floor_alt}
    }
    for i, name in ipairs monsters
        column = x1 + w - 5 - (i - 1) % 3 * 2
        row = y1 + math.floor(h / 2) + math.floor((i - 1) / 3) * 2 - 1
        MapUtils.spawn_enemy(map, name, {column, row})
    for dy=-1,1
        append map.player_candidate_squares, {(x1 + 3) * 32 + 16, (y1 + math.floor(h / 2) + dy) * 32 + 16}
    game_map = NewMaps.generate_game_map(map)
    require("core.Map").set_vision_radius(game_map, 10) -- the whole room in view
    return game_map, MapUtils.pick_player_squares(map, map.player_candidate_squares)

return {
    :generate
    generate_and_spawn: (monsters, tileset_name) ->
        game_map, spawn_points = generate(monsters, tileset_name)
        World.players_spawn(game_map, spawn_points)
        return game_map
}
