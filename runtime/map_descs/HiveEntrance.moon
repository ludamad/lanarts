Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"

Vaults = require "maps.Vaults"
SourceMap = require "core.SourceMap"
{:place_feature} = require "maps.01_Overworld"

{:MapNode, :MapDesc} = require "maps.MapDesc"

-- Generation constants and data
{ :FLAG_INNER_PERIMETER } = Vaults
{:center} = require "maps.MapRegionUtils"
{:load_map_polys, :node_paint_group} = require "maps.MapNodeFills"

CONTENT_MAP = nilprotect {
    A: Tilesets.hive.floor, B: Tilesets.hive.floor_alt
    C: Tilesets.orc.floor, D: Tilesets.hive.floor_alt
    E: Tilesets.hive.floor, F: Tilesets.orc.floor
}

node_place_hive_entrance_polys = () =>
    xy = center @region_set
    parts = load_map_polys @rng, "HiveEntrance", xy[1], xy[2], 70, 70, @rng\randomf(-math.pi, math.pi)
    for name, region in spairs parts
        region.group = @new_group()
        region\apply {
            map: @map
            operator: {
                group: region.group
                add: SourceMap.FLAG_SEETHROUGH
                remove: SourceMap.FLAG_SOLID
                content: CONTENT_MAP[name]
            }
        }
    bbox = parts.A\bbox()
    {x, y} = MapUtils.random_square(@map, bbox, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
    append @map.player_candidate_squares, {x*32+16,y*32+16}
    for back_link in *@desc.back_links
        xy = MapUtils.random_square(@map, bbox, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        back_link(@map, xy)
    for forward_link in *@desc.forward_links
        xy = MapUtils.random_square(@map, nil, {matches_group: parts.F.group, matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        forward_link(@map, xy)
    fill_all = () ->
        bbox = nil
        selector = {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}}
        for i=1,8
            sqr = MapUtils.random_square(@map, bbox, selector)
            if not sqr
                return false
            item = ItemUtils.item_generate (if i < 8 then ItemGroups.basic_items else ItemGroups.enchanted_items)
            MapUtils.spawn_item(@map, item.type, item.amount, sqr)
        return true
    add_honeycombs = (regions) ->
        item_placer = (map_, xy_) ->
            if map_.rng\chance(0.1)
                MapUtils.spawn_item(@map, "Honeycomb", 1, xy_)
        for i=1,20
            vault = SourceMap.area_template_create(Vaults.honeycomb {:item_placer})
            if not place_feature(@map, vault, @region_set.regions)
                continue
        return true
    fill_with_mob = (part, enemies) ->
        {:group} = part
        bbox = part\bbox()
        selector = {matches_group: {group, group}, matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}}
        monster_focus_point = MapUtils.random_square(@map, bbox, selector)
        if not monster_focus_point
            return false
        for type in *enemies
            enemy = MapUtils.random_enemy(@map, type, bbox, selector)
            if not enemy
                return false
            enemy.monster_wander_position = () => monster_focus_point
        return true
    if not add_honeycombs()
        return false
    if not fill_all()
        return false
    if not fill_with_mob(parts.B, {"Giant Bee", "Giant Bee", "Giant Bee"})
        return false
    if not fill_with_mob(parts.E, {"Giant Bee", "Giant Bee", "Giant Bee", "Queen Bee"})
        return false
    if not fill_with_mob(parts.F, {"Giant Bee", "Giant Bee", "Giant Bee", "Queen Bee", "Queen Bee"})
        return false
    return true

return MapDesc.create {
    map_label: "Hive Entrance"
    size: {130, 130}
    default_content: Tilesets.hive.wall
    children: {
        MapNode.create {
            place: () => node_paint_group(@)
            children: {
                MapNode.create {
                    place: node_place_hive_entrance_polys
                }
            }
        }
    }
}
