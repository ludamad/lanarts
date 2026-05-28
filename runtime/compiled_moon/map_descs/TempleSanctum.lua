local OldMaps = require("maps.OldMaps")
local Tilesets = require("tiles.Tilesets")
local Vaults = require("maps.Vaults")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local MapUtils = require("maps.MapUtils")
local place_vault
place_vault = require("maps.VaultUtils").place_vault
return OldMaps.create_map_desc(table.merge(OldMaps.Dungeon2[3], {
  tileset = Tilesets.temple,
  label = "Temple Sanctum",
  on_generate = function(map)
    local _list_0 = {
      "Azurite Key",
      "Scroll of Experience",
      false
    }
    for _index_0 = 1, #_list_0 do
      local type = _list_0[_index_0]
      if not place_vault(map, Vaults.small_item_vault({
        rng = map.rng,
        item_placer = function(map_, xy)
          local amount = 1
          if not type then
            do
              local _obj_0 = ItemUtils.item_generate(ItemGroups.enchanted_items)
              type, amount = _obj_0.type, _obj_0.amount
            end
          end
          return MapUtils.spawn_item(map_, type, amount, xy)
        end,
        tileset = Tilesets.snake
      })) then
        return false
      end
    end
    return true
  end
}))
