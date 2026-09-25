OldMaps = require "maps.OldMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
Vaults = require "maps.Vaults"
{ :place_vault } = require "maps.VaultUtils"

return OldMaps.create_map_desc table.merge OldMaps.Dungeon4[1], {
    tileset: Tilesets.pixulloch
    label: "Pixullochia Entrance"
    on_generate: (map) ->
        -------------------------
        -- Guarded treasure room, failure to place is not fatal --
        place_vault map, Vaults.crypt_encounter_vault {
            rng: map.rng
            enemy_placer: (map_, xy) ->
                -- Only guard about half of the 'e' squares
                if map_.rng\chance(.5)
                    enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
                    MapUtils.spawn_enemy(map_, enemy, xy)
            door_placer: (map_, xy) ->
                MapUtils.spawn_door(map_, xy)
            item_placer: (map_, xy) ->
                item = ItemUtils.item_generate ItemGroups.enchanted_items
                MapUtils.spawn_item(map_, item.type, item.amount, xy)
            tileset: Tilesets.pixulloch
        }
        return true
}
