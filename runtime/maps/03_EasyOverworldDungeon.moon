import map_place_object, ellipse_points,
    LEVEL_PADDING, Region, RVORegionPlacer,
    random_rect_in_rect, random_ellipse_in_ellipse,
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func,
    towards_region_delta_func,
    rectangle_points,
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

GeometryUtils = require "maps.GeometryUtils"
MapRegionShapes = require("maps.MapRegionShapes")
{:MapRegion, :combine_map_regions, :from_bbox} = require "maps.MapRegion"

PolyPartition = require "core.PolyPartition"
DebugUtils = require "maps.DebugUtils"
NewMaps = require "maps.NewMaps"
NewDungeons = require "maps.NewDungeons"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
import make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
    from MapUtils

MapSequence = require "maps.MapSequence"
Vaults = require "maps.Vaults"
World = require "core.World"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

{:MapCompilerContext, :make_on_player_interact} = require "maps.MapCompilerContext"
Places = require "maps.Places"

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

{:center, :find_bbox, :find_square, :selector_filter, :selector_map} = require "maps.MapRegionUtils"

{:generate_map_node} = require "maps.01_Overworld"

PADDING = 10
ROOT = false -- represents the map root 

event_log = (...) -> print string.format(...)
rng = rawget(_G, 'rng') or require("mtwist").create(os.time() * 1000 + os.clock())
rawset(_G, 'rng', rng)

shrink = (bbox, x, y) ->
    {x1,y1,x2,y2} = bbox
    return {
        x1+x
        y1+y
        x2-x
        y2-y
    }

MapDesc = newtype {
    init: (@map_args) =>
        @parent = false
        @children = @map_args.children
        @map_args.children = nil
        @region_set = false
    compile: (portal_spawns) =>
        map_args = table.merge {
            :rng
            size: {100, 100}
            default_content: Tilesets.orc.wall
            default_flags: {SourceMap.FLAG_SOLID} --, SourceMap.FLAG_SEETHROUGH}
            map_label: "Dungeon"
        }, @map_args
        map_args.size = vector_add(map_args.size, {PADDING*2, PADDING*2})
        map = NewMaps.source_map_create map_args
        map.portal_spawns = portal_spawns

        room_selector = {
            matches_all: SourceMap.FLAG_SOLID
            matches_none: SourceMap.FLAG_PERIMETER
        }
        @region_set = {
            :map
            regions: {
                from_bbox(
                    PADDING
                    PADDING
                    map.size[1] - PADDING
                    map.size[2] - PADDING)\with_selector(room_selector)
            }
        }
        for child in *@children
            child.parent = @
            if not child\compile()
                return nil
        if not NewMaps.check_connection(map)
            event_log("Failed connectivity check")
            return nil
        return map
}

MapNode = newtype {
    init: (args) =>
        @children = args.children or {}
        @place = args.place
    new_group: () =>
        @map.next_group += 1
        return @map.next_group
    compile: () =>
        @region_set = @parent.region_set
        @group = @new_group()
        if not @place()
            return false
        for child in *@children
            child.parent = @
            if not child\compile()
                return false
        return true
    chance: (prob) => @region_set.map.rng\chance(prob)
    apply: (args) =>
        args.map = @map
        for region in *@region_set.regions
            region\apply(args)
    get: {
        map: () => @region_set.map
        rng: () => @region_set.map.rng
    }
}
 
node_paint_group = () =>
    @apply {
        operator: {group: @group}
    }
    matches_group = {@group, @group}
    @region_set = selector_map @region_set, (s) ->
        return table.merge s, {:matches_group}
    return true

MAX_TUNNEL_TRIES = 100
node_connect_rect_rooms = () =>
    regions = @region_set.regions
    for region in *regions
        tries_without_tunnel = 0
        for j=1,2
            while tries_without_tunnel < MAX_TUNNEL_TRIES
                success = SourceMap.try_tunnel_apply {
                    map: @map
                    rng: @rng
                    start_area: region.inner_bbox
                    start_selector: region.selector
                    validity_selector: {
                        fill_selector: {
                            matches_group: {@parent.group, -1}
                            matches_all: SourceMap.FLAG_SOLID
                            matches_none: SourceMap.FLAG_TUNNEL
                        }
                        perimeter_selector: {
                            matches_all: SourceMap.FLAG_SOLID
                            matches_none: SourceMap.FLAG_TUNNEL
                        }
                    }
                    completion_selector: {
                        fill_selector: {
                            matches_none: { SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER, SourceMap.FLAG_TUNNEL }
                        }
                        perimeter_selector: {
                            matches_none: SourceMap.FLAG_SOLID
                        }
                    }
                    fill_operator: {
                        add: {SourceMap.FLAG_TUNNEL, SourceMap.FLAG_SEETHROUGH}
                        remove: SourceMap.FLAG_SOLID
                        content: Tilesets.pebble.floor
                    }
                    perimeter_operator: {
                        matches_all: SourceMap.FLAG_SOLID
                        add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL, SourceMap.FLAG_PERIMETER}
                        remove: SourceMap.FLAG_SEETHROUGH
                        content: Tilesets.pebble.wall
                    }
                    perimeter_width: 1
                    max_length: (tries_without_tunnel % 10 + 1) * 10
                    size_range: {1,1}
                    tunnels_per_room_range: {1,1}
                }
                if success
                    tries_without_tunnel = 0
                    break
                else
                    tries_without_tunnel += 1
    return true

node_place_bbox = () =>
    size = {10, 10}
    if @chance(0.2)
        size = {5, 5}
    result = find_bbox(@region_set, size)
    if not result
        return false
    {:bbox} = result
    group = @new_group()
    region = from_bbox(unpack(bbox))
    region.inner_bbox = shrink(bbox, 1, 1)
    region.selector = {matches_group: {group, group}}
    append regions, region
    SourceMap.rectangle_apply {
        map: @map
        area: bbox
        fill_operator: {
            :group
            add: SourceMap.FLAG_SEETHROUGH
            remove: SourceMap.FLAG_SOLID
            content: Tilesets.pebble.floor
        }
        perimeter_width: 0
        perimeter_operator: {
            add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER}
            remove: SourceMap.FLAG_SEETHROUGH
            content: Tilesets.pebble.wall
        }
    }
    return true

_load_map_poly = memoized (name) ->
    json = require "json"
    success, data = json.parse(file_as_string "maps/templates/#{name}.json")
    assert success
    parts = {}
    for {:objects} in *data.layers
        for idx, {:x, :y, :name, :polygon} in ipairs (objects or {})
            ox, oy = x, y
            if not polygon
                continue
            convex_polys = PolyPartition.decompose [{x+ox, y+oy} for {:x, :y} in *polygon]
            if name == ""
                error("Need to specify name!")
                name = tostring(idx)
            parts[name] = MapRegion.create(convex_polys)
    return parts

resolve_parts = (rng, parts) ->
    possibilities = {}
    for name, part in pairs parts
        {prefix, idx} = name\split("_")
        possibilities[prefix] or= {}
        append possibilities[prefix], part
    for k, choices in pairs possibilities
        possibilities[k] = rng\random_choice(choices)
    return possibilities

load_map_polys = (name, nx, ny, nw, nh, angle) ->
    parts = table.deep_clone(_load_map_poly(name))
    parts = resolve_parts(rng, parts)
    full_map = combine_map_regions [region for _, region in pairs parts]
    bbox = full_map\bbox()
    -- Correct into a w by h shape situated at x,y
    for points in *full_map.polygons
        for i=1,#points
            points[i] = MapRegionShapes.transform_point(points[i], nx, ny, nw, nh, bbox)
    full_map\rotate angle, {nx, ny}
    return parts

node_place_map_polys = () =>
    xy = center @region_set
    parts = load_map_polys "03_WindingPath", xy[1], xy[2], 40, 30, @rng\randomf(-math.pi, math.pi)
    for name, region in spairs parts
        region.group = @new_group()
        content = nilprotect {
            A: Tilesets.pebble.floor, B: Tilesets.pebble.floor_alt
            C: Tilesets.orc.floor, D: Tilesets.pebble.floor_alt
            E: Tilesets.pebble.floor
        }
        region\apply {
            map: @map
            operator: {
                group: region.group
                add: SourceMap.FLAG_SEETHROUGH
                remove: SourceMap.FLAG_SOLID
                content: content[name]
            }
        }
    bbox = parts.A\bbox()
    {x, y} = MapUtils.random_square(@map, bbox, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
    append @map.player_candidate_squares, {x*32+16,y*32+16}
    for spawn in *@map.portal_spawns
        xy = MapUtils.random_square(@map, bbox, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        spawn(@map, xy)
    bbox = parts.E\bbox()
    monster_focus_point = MapUtils.random_square(@map, bbox, {matches_group: {parts.E.group, parts.E.group}})
    if not monster_focus_point
        return false
    for i=1,10
        enemy = MapUtils.random_enemy(@map, "Orc Warrior", bbox, {matches_group: {parts.E.group, parts.E.group}})
        if not enemy
            break
        enemy.monster_wander_position = () => monster_focus_point
    return true


node_place_easy_overworld_rooms = () =>
    -- Ensure it is a range
    n_rooms = 5
    event_log("(RNG #%d) generating %d rooms", @rng\amount_generated(), n_rooms)
    regions = {}
    for i=1,n_rooms
        size = {10, 10}
        if @chance(0.2)
            size = {5, 5}
        result = find_bbox(@region_set, size)
        if not result
            return false
        {:bbox} = result
        group = @new_group()
        region = from_bbox(unpack(bbox))
        region.inner_bbox = shrink(bbox, 1, 1)
        region.selector = {matches_group: {group, group}}
        append regions, region
        SourceMap.rectangle_apply {
            map: @map
            area: bbox
            fill_operator: {
                :group
                add: SourceMap.FLAG_SEETHROUGH
                remove: SourceMap.FLAG_SOLID
                content: Tilesets.pebble.floor
            }
            perimeter_width: 0
            perimeter_operator: {
                add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER}
                remove: SourceMap.FLAG_SEETHROUGH
                content: Tilesets.pebble.wall
            }
        }
    event_log("(RNG #%d) after generating %d rooms", @rng\amount_generated(), n_rooms)
    @region_set = {map: @map, :regions}
    if not node_connect_rect_rooms(@)
        return false
    return true

EasyOverworldDungeon = MapDesc.create {
    size: {60, 60}
    children: {
        MapNode.create {
            place: () => node_paint_group(@)
            children: {
                MapNode.create {
                    place: node_place_map_polys
                    --place: () => node_place_easy_overworld_rooms(@)
                }
            }
        }
    }
}

MAX_MAP_TRIES = 100
generate = (portal_spawns) ->
    map = NewMaps.try_n_times MAX_MAP_TRIES, () -> EasyOverworldDungeon\compile(portal_spawns)
    game_map = NewMaps.generate_game_map(map)
    for post_poned in *map.post_game_map
        post_poned(game_map)
    --World.players_spawn(game_map, map.player_candidate_squares)
    return game_map

return {:generate}
