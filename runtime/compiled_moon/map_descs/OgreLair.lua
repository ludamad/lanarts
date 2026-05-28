local OldMaps = require("maps.OldMaps")
local Tilesets = require("tiles.Tilesets")
local Vaults = require("maps.Vaults")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local MapUtils = require("maps.MapUtils")
local place_vault
place_vault = require("maps.VaultUtils").place_vault
return OldMaps.create_map_desc({
  tileset = Tilesets.snake,
  label = "Ogre Lair",
  layout = {
    {
      size = {
        60,
        40
      },
      rooms = {
        padding = 0,
        amount = 40,
        size = {
          3,
          7
        }
      },
      tunnels = {
        padding = 0,
        width = {
          1,
          3
        },
        per_room = 5
      }
    }
  },
  content = {
    items = {
      amount = 8,
      group = ItemGroups.enchanted_items
    },
    enemies = {
      wandering = true,
      amount = 0,
      generated = {
        {
          enemy = "Ogre Mage",
          guaranteed_spawns = 5
        },
        {
          enemy = "Orc Warrior",
          guaranteed_spawns = 3
        },
        {
          enemy = "Adder",
          guaranteed_spawns = 5
        }
      }
    }
  },
  on_generate = function(map)
    for i = 1, 3 do
      local item = ItemUtils.randart_generate(1)
      if not place_vault(map, Vaults.small_item_vault({
        rng = map.rng,
        item_placer = function(map_, xy)
          return MapUtils.spawn_item(map_, item.type, item.amount, xy)
        end,
        tileset = Tilesets.snake
      })) then
        return false
      end
    end
    return true
  end
})
