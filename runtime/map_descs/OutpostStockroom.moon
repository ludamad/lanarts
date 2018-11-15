Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
SourceMap = require "core.SourceMap"
OldMaps = require "maps.OldMaps"
Vaults = require "maps.Vaults"
{ :place_vault } = require "maps.VaultUtils"
{ :FLAG_INNER_PERIMETER } = Vaults
Region1 = require "maps.Region1"

return OldMaps.create_map_desc table.merge OldMaps.Dungeon1[2], {
    tileset: Tilesets.pebble
    label: "Outpost Stockroom"
    on_generate: (map) ->
        -------------------------
        -- Place key vault     --
        for i=1,2
            if not place_vault map, Vaults.small_item_vault {
                rng: map.rng
                item_placer: (map_, xy) ->
                    item = ItemUtils.item_generate ItemGroups.basic_items
                    MapUtils.spawn_item(map_, item.type, item.amount, xy)
                tileset: Tilesets.pebble
            }
                return false

        area = {0,0,map.size[1],map.size[2]}
        for i=1,4
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                return false
            Region1.generate_store(map, sqr)

        sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
        if not sqr
            return false
        return true
}
