import map_place_object, ellipse_points,
    LEVEL_PADDING, Region, RVORegionPlacer,
    random_rect_in_rect, random_ellipse_in_ellipse,
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func,
    towards_region_delta_func,
    rectangle_points,
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

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
PolygonTriangulation = require "maps.PolygonTriangulation"

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
    compile: () =>
        map_args = table.merge {
            :rng
            size: {100, 100}
            default_content: Tilesets.crypt.wall
            default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
            map_label: "Dungeon"
        }, @map_args
        map_args.size = vector_add(map_args.size, {PADDING*2, PADDING*2})
        map = NewMaps.source_map_create map_args

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
        --if not NewMaps.check_connection(map)
        --    event_log("Failed connectivity check")
        --    return nil
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
        for {:name, :polygon} in *(objects or {})
            if not polygon
                continue
            --d_triangles = PolygonTriangulation.triangulate unpack for {:x, :y} in *polygon
            --    PolygonTriangulation.Point(x, y)
            --triangles = for {:p1, :p2, :p3} in *d_triangles
            --    for {:x, :y} in *{p1, p2, p3}
            --        {x, y}
            convex_polys = PolyPartition.decompose [{x, y} for {:x, :y} in *polygon]
            parts[name or (#parts+1)] = MapRegion.create(convex_polys)
    return parts

load_map_polys = (name, nx, ny, nw, nh) ->
    parts = table.deep_clone(_load_map_poly(name))
    parts.full_map = combine_map_regions [region for _, region in pairs parts]
    bbox = parts.full_map\bbox()
    -- Correct into a w by h shape situated at x,y
    for points in *parts.full_map.polygons
        for i=1,#points
            points[i] = MapRegionShapes.transform_point(points[i], nx, ny, nw, nh, bbox)
    return parts

node_place_spiral = () =>
    xy = center @region_set
    polygons = MapRegionShapes.get_shape_polygons('bart', xy[1], xy[2], 40, 40)
    region = MapRegion.create(polygons)
    region\rotate @rng\randomf(0, math.pi), xy
    region\apply {
        map: @map
        operator: {
            group: @group
            add: SourceMap.FLAG_SEETHROUGH
            remove: SourceMap.FLAG_SOLID
            content: Tilesets.pebble.floor
        }
    }
    @region_set = {map: @map, regions: {region}}
    return true

node_place_map_polys = () =>
    xy = center @region_set
    polys = load_map_polys "03_WindingPath", xy[1], xy[2], 150, 100
    polys.full_map\rotate @rng\randomf(0, math.pi*2), xy
    for poly in *{
        polys.full_map
        --polys.back
        --polys.front
        --polys.body
    }
        poly\apply {
            map: @map
            operator: {
                group: @new_group()
                add: SourceMap.FLAG_SEETHROUGH
                remove: SourceMap.FLAG_SOLID
                content: Tilesets.pebble.floor
            }
        }
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
    size: {300, 300}
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
return NewMaps.try_n_times MAX_MAP_TRIES, () -> EasyOverworldDungeon\compile()
