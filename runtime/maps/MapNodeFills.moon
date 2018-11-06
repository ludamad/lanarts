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
{:MapNode, :MapDesc} = require "maps.MapDesc"
Places = require "maps.Places"

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

{:center, :find_bbox, :find_square, :selector_filter, :selector_map} = require "maps.MapRegionUtils"

{:generate_map_node} = require "maps.01_Overworld"

MAX_TUNNEL_TRIES = 100
-- TODO find a home for this
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

node_paint_group = () =>
    @apply {
        operator: {group: @group}
    }
    matches_group = {@group, @group}
    @region_set = selector_map @region_set, (s) ->
        return table.merge s, {:matches_group}
    return true

_load_map_poly = memoized (name) ->
    json = require "json"
    success, data = json.parse(file_as_string "map_descs/#{name}.json")
    assert success
    parts = {}
    for {:objects} in *data.layers
        for idx, {:x, :y, :name, :polygon} in ipairs (objects or {})
            ox, oy = x, y
            if not polygon
                continue
            -- Convert to internal polygon format:
            tuple_format_polygon = [{x+ox, y+oy} for {:x, :y} in *polygon]
            -- Decompose into convex polygons:
            convex_polys = PolyPartition.decompose(tuple_format_polygon)
            if name == ""
                error("Need to specify name!")
                name = tostring(idx)
            parts[name] = MapRegion.create(convex_polys)
    return parts

resolve_parts = (rng, parts) ->
    possibilities = {}
    for name, part in spairs parts
        {prefix, idx} = name\split("_")
        possibilities[prefix] or= {}
        append possibilities[prefix], part
    for k, choices in spairs possibilities
        possibilities[k] = rng\random_choice(choices)
    return possibilities

load_map_polys = (rng, name, nx, ny, nw, nh, angle) ->
    parts = table.deep_clone(_load_map_poly(name))
    parts = resolve_parts(rng, parts)
    full_map = combine_map_regions [region for _, region in spairs parts]
    bbox = full_map\bbox()
    -- Correct into a w by h shape situated at x,y
    for points in *full_map.polygons
        for i=1,#points
            points[i] = MapRegionShapes.transform_point(points[i], nx, ny, nw, nh, bbox)
    full_map\rotate angle, {nx, ny}
    return parts

return nilprotect {:load_map_polys, :node_paint_group, :node_connect_rect_rooms}
