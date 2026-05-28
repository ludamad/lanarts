local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local SourceMap = require("core.SourceMap")
local OldMaps = require("maps.OldMaps")
local Vaults = require("maps.Vaults")
local place_vault
place_vault = require("maps.VaultUtils").place_vault
local FLAG_INNER_PERIMETER
FLAG_INNER_PERIMETER = Vaults.FLAG_INNER_PERIMETER
local Region1 = require("maps.Region1")
return OldMaps.create_map_desc(table.merge(OldMaps.Dungeon1[2], {
  tileset = Tilesets.pebble,
  label = "Outpost Stockroom",
  on_generate = function(map)
    for i = 1, 2 do
      if not place_vault(map, Vaults.small_item_vault({
        rng = map.rng,
        item_placer = function(map_, xy)
          local item = ItemUtils.item_generate(ItemGroups.basic_items)
          return MapUtils.spawn_item(map_, item.type, item.amount, xy)
        end,
        tileset = Tilesets.pebble
      })) then
        return false
      end
    end
    local area = {
      0,
      0,
      map.size[1],
      map.size[2]
    }
    for i = 1, 4 do
      local sqr = MapUtils.random_square(map, area, {
        matches_none = {
          FLAG_INNER_PERIMETER,
          SourceMap.FLAG_HAS_OBJECT,
          SourceMap.FLAG_SOLID
        }
      })
      if not sqr then
        return false
      end
      Region1.generate_store(map, sqr)
    end
    local sqr = MapUtils.random_square(map, area, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        SourceMap.FLAG_SOLID
      }
    })
    if not sqr then
      return false
    end
    return true
  end
}))
