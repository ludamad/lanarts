local MapNode, MapDesc
do
  local _obj_0 = require("maps.MapDesc")
  MapNode, MapDesc = _obj_0.MapNode, _obj_0.MapDesc
end
local Tile, spread_region_delta_func
do
  local _obj_0 = require("maps.GenerateUtils")
  Tile, spread_region_delta_func = _obj_0.Tile, _obj_0.spread_region_delta_func
end
local NewMaps = require("maps.NewMaps")
local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local Vaults = require("maps.Vaults")
local place_vault_in
place_vault_in = require("maps.VaultUtils").place_vault_in
local SourceMap = require("core.SourceMap")
local OldMaps = require("maps.OldMaps")
local MapCompilerContext, make_on_player_interact
do
  local _obj_0 = require("maps.MapCompilerContext")
  MapCompilerContext, make_on_player_interact = _obj_0.MapCompilerContext, _obj_0.make_on_player_interact
end
local Places = require("maps.Places")
local FLAG_INNER_PERIMETER, FLAG_NO_ENEMY_SPAWN, FLAG_OVERWORLD, FLAG_DOOR_CANDIDATE
FLAG_INNER_PERIMETER, FLAG_NO_ENEMY_SPAWN, FLAG_OVERWORLD, FLAG_DOOR_CANDIDATE = Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_OVERWORLD, Vaults.FLAG_DOOR_CANDIDATE
local create_dungeon_scheme
create_dungeon_scheme = function(tileset)
  return nilprotect({
    floor1 = Tile.create(tileset.floor, false, true, { }, {
      FLAG_OVERWORLD
    }),
    floor2 = Tile.create(tileset.floor_alt, false, true, { }, {
      FLAG_OVERWORLD
    }),
    wall1 = Tile.create(tileset.wall, true, false, { }, {
      FLAG_OVERWORLD
    }),
    wall2 = Tile.create(tileset.wall_alt, true, false, { }, {
      FLAG_OVERWORLD
    })
  })
end
local DUNGEON_TILESET = create_dungeon_scheme(Tilesets.underdungeon)
local underdungeon_items_and_enemies
underdungeon_items_and_enemies = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
    for i = 1, OldMaps.adjusted_item_amount(10) do
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
      map:square_apply(sqr, {
        add = {
          SourceMap.FLAG_HAS_OBJECT
        }
      })
      local item = ItemUtils.item_generate(ItemGroups.basic_items)
      MapUtils.spawn_item(map, item.type, item.amount, sqr)
    end
    OldMaps.generate_from_enemy_entries(map, OldMaps.hard_enemies, 20, area, {
      matches_none = {
        SourceMap.FLAG_SOLID,
        Vaults.FLAG_HAS_VAULT,
        FLAG_NO_ENEMY_SPAWN
      }
    })
  end
  return true
end
local place_doors_and_statues
place_doors_and_statues = function(region_set)
  local map, regions
  map, regions = region_set.map, region_set.regions
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local area = region:bbox()
    local conf = region.conf
    local _list_0 = SourceMap.rectangle_match({
      map = map,
      selector = {
        matches_none = {
          SourceMap.FLAG_HAS_OBJECT,
          SourceMap.FLAG_SOLID
        },
        matches_all = {
          FLAG_DOOR_CANDIDATE
        }
      }
    })
    for _index_1 = 1, #_list_0 do
      local xy = _list_0[_index_1]
      MapUtils.spawn_door(map, xy)
    end
    for i = 1, conf.n_statues do
      local sqr = MapUtils.random_square(map, area, {
        matches_none = {
          FLAG_INNER_PERIMETER,
          SourceMap.FLAG_HAS_OBJECT,
          Vaults.FLAG_HAS_VAULT,
          SourceMap.FLAG_SOLID
        }
      })
      if not sqr then
        break
      end
      map:square_apply(sqr, {
        add = {
          SourceMap.FLAG_SOLID,
          SourceMap.FLAG_HAS_OBJECT
        },
        remove = SourceMap.FLAG_SEETHROUGH
      })
      MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0, 17))
    end
  end
end
local underdungeon_features
underdungeon_features = function(region_set, back_links, forward_links)
  local place_purple_dragon_lair
  place_purple_dragon_lair = function()
    local name = "Purple Dragon Lair"
    local create_compiler_context
    create_compiler_context = function()
      local cc = MapCompilerContext.create()
      cc:register(name, Places.DragonLair)
      append(region_set.map.post_game_map, function(game_map)
        return cc:register("root", function()
          return game_map
        end)
      end)
      return cc
    end
    local cc = create_compiler_context()
    return place_vault_in(region_set, Vaults.graghs_lair_entrance({
      tileset = Tilesets.hell,
      dungeon_placer = function(map, xy)
        local portal = MapUtils.spawn_portal(map, xy, "spr_gates.volcano_portal")
        local other_portal = cc:add_pending_portal(name, function(feature, compiler)
          return MapUtils.random_portal(compiler.map, nil, "spr_gates.volcano_exit")
        end)
        other_portal:connect({
          feature = portal,
          label = "root"
        })
        portal.on_player_interact = make_on_player_interact(cc, other_portal)
        return portal
      end,
      door_placer = function(map, xy)
        return MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
      end,
      enemy_placer = function(map, xy)
        local enemy = OldMaps.enemy_generate({
          {
            enemy = "Fire Bat",
            chance = 100
          }
        })
        return MapUtils.spawn_enemy(map, enemy, xy)
      end
    }))
  end
  if not place_purple_dragon_lair() then
    return false
  end
  if not underdungeon_items_and_enemies(region_set) then
    return false
  end
  local _list_0 = table.tconcat(back_links, forward_links)
  for _index_0 = 1, #_list_0 do
    local link = _list_0[_index_0]
    local xy = MapUtils.random_square(region_set.map, nil, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    if not xy then
      return false
    end
    link(region_set.map, xy)
  end
  return true
end
return MapDesc.create({
  map_label = "Underdungeon",
  size = {
    200,
    200
  },
  default_content = Tilesets.underdungeon.wall,
  default_flags = {
    SourceMap.FLAG_SOLID
  },
  arc_chance = 0.05,
  children = {
    MapNode.create({
      place = function(self)
        local number_regions = 5 + self.rng:random(0, 4) + self.rng:random(0, 4) + self.rng:random(0, 4)
        local connect_line_width
        connect_line_width = function()
          return 1 + self.rng:random(0, 4) + self.rng:random(0, 4) + self.rng:random(0, 4) + self.rng:random(0, 4)
        end
        local room_radius
        room_radius = function()
          return 5 + self.rng:random(0, 4) + self.rng:random(0, 4) + self.rng:random(0, 4)
        end
        local factor = math.sqrt(number_regions / 5)
        local size = {
          100,
          100
        }
        local template = nilprotect({
          default_wall = Tile.create(Tilesets.underdungeon.wall, true, true, {
            SourceMap.FLAG_SOLID
          }),
          subtemplates = {
            nilprotect({
              is_overworld = true,
              size = size,
              number_regions = number_regions,
              floor1 = DUNGEON_TILESET.floor1,
              floor2 = DUNGEON_TILESET.floor2,
              wall1 = DUNGEON_TILESET.wall1,
              wall2 = DUNGEON_TILESET.wall2,
              rect_room_num_range = {
                4,
                8
              },
              rect_room_size_range = {
                14,
                20
              },
              rvo_iterations = 100,
              connect_line_width = connect_line_width,
              room_radius = room_radius,
              region_delta_func = spread_region_delta_func,
              n_statues = 4 * factor
            })
          },
          outer_conf = nilprotect({
            floor1 = DUNGEON_TILESET.floor1,
            floor2 = DUNGEON_TILESET.floor2,
            connect_line_width = connect_line_width
          }),
          shell = 25
        })
        if not NewMaps.map_try_create(self.map, self.map.rng, template) then
          return false
        end
        NewMaps.generate_door_candidates(self.map, self.map.rng, self.map.regions)
        append(self.map.post_maps, function()
          return place_doors_and_statues(self.region_set)
        end)
        if not underdungeon_features(self.region_set, self.desc.back_links, self.desc.forward_links) then
          return false
        end
        return true
      end
    })
  }
})
