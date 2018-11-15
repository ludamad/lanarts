OldMaps = require "maps.OldMaps"
Tilesets = require "tiles.Tilesets"
Vaults = require "maps.Vaults"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
MapUtils = require "maps.MapUtils"
{:place_vault} = require "maps.VaultUtils"

return OldMaps.create_map_desc {
    tileset: Tilesets.snake
    label: "Ogre Lair"
    layout: {{size: {60,40}, rooms: {padding:0,amount:40,size:{3,7}},tunnels:{padding:0, width:{1,3},per_room: 5}}}
    content: {
        items: {amount: 8, group: ItemGroups.enchanted_items}
        enemies: {
            wandering: true
            amount: 0
            generated: {
              {enemy: "Ogre Mage",         guaranteed_spawns: 5}
              {enemy: "Orc Warrior",       guaranteed_spawns: 3}
              {enemy: "Adder",             guaranteed_spawns: 5}
            }
        }
    }
    on_generate: (map) ->
        ----------------------------------------------------------
        for i=1,3
            item = ItemUtils.randart_generate(1)
            if not place_vault map, Vaults.small_item_vault {
                rng: map.rng
                item_placer: (map_, xy) -> MapUtils.spawn_item(map_, item.type, item.amount, xy)
                tileset: Tilesets.snake
            }
                return false
        ----------------------------------------------------------
        return true
}
