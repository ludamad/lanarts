OldMaps = require "maps.OldMaps"
Tilesets = require "tiles.Tilesets"
Vaults = require "maps.Vaults"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
MapUtils = require "maps.MapUtils"
{:place_vault} = require "maps.VaultUtils"

return OldMaps.create_map_desc table.merge OldMaps.Dungeon2[3], {
    tileset: Tilesets.temple
    label: "Temple Sanctum"
    on_generate: (map) ->
        ---------------------------------------------------------------------
        -- Place key vault, along with a gold vault and 2 regular items.   --
        for type in *{"Azurite Key", "Scroll of Experience", false}
            if not place_vault map, Vaults.small_item_vault {
                rng: map.rng
                item_placer: (map_, xy) ->
                    amount = 1
                    if not type
                        {:type, :amount} = ItemUtils.item_generate ItemGroups.enchanted_items
                    MapUtils.spawn_item(map_, type, amount, xy)
                tileset: Tilesets.snake
            }
                return false
        return true
}
