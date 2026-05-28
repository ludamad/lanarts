local Tilesets = require("tiles.Tilesets")
local MapUtils = require("maps.MapUtils")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local Vaults = require("maps.Vaults")
local SourceMap = require("core.SourceMap")
local place_vault, place_vault_in
do
  local _obj_0 = require("maps.VaultUtils")
  place_vault, place_vault_in = _obj_0.place_vault, _obj_0.place_vault_in
end
local MapNode, MapDesc
do
  local _obj_0 = require("maps.MapDesc")
  MapNode, MapDesc = _obj_0.MapNode, _obj_0.MapDesc
end
local FLAG_INNER_PERIMETER
FLAG_INNER_PERIMETER = Vaults.FLAG_INNER_PERIMETER
local center
center = require("maps.MapRegionUtils").center
local load_map_polys, node_paint_group
do
  local _obj_0 = require("maps.MapNodeFills")
  load_map_polys, node_paint_group = _obj_0.load_map_polys, _obj_0.node_paint_group
end
local CONTENT_MAP = nilprotect({
  A = Tilesets.hive.floor,
  B = Tilesets.hive.floor_alt,
  C = Tilesets.orc.floor,
  D = Tilesets.hive.floor_alt,
  E = Tilesets.hive.floor,
  F = Tilesets.orc.floor
})
local node_place_hive_entrance_polys
node_place_hive_entrance_polys = function(self)
  local xy = center(self.region_set)
  local parts = load_map_polys(self.rng, "HiveEntrance", xy[1], xy[2], 70, 70, self.rng:randomf(-math.pi, math.pi))
  for name, region in spairs(parts) do
    region.group = self:new_group()
    region:apply({
      map = self.map,
      operator = {
        group = region.group,
        add = SourceMap.FLAG_SEETHROUGH,
        remove = SourceMap.FLAG_SOLID,
        content = CONTENT_MAP[name]
      }
    })
  end
  local bbox = parts.A:bbox()
  local x, y
  do
    local _obj_0 = MapUtils.random_square(self.map, bbox, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    x, y = _obj_0[1], _obj_0[2]
  end
  append(self.map.player_candidate_squares, {
    x * 32 + 16,
    y * 32 + 16
  })
  local _list_0 = self.desc.back_links
  for _index_0 = 1, #_list_0 do
    local back_link = _list_0[_index_0]
    xy = MapUtils.random_square(self.map, bbox, {
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    back_link(self.map, xy)
  end
  local _list_1 = self.desc.forward_links
  for _index_0 = 1, #_list_1 do
    local forward_link = _list_1[_index_0]
    xy = MapUtils.random_square(self.map, nil, {
      matches_group = parts.F.group,
      matches_none = {
        FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
    forward_link(self.map, xy)
  end
  local fill_all
  fill_all = function()
    bbox = nil
    local selector = {
      matches_none = {
        SourceMap.FLAG_HAS_OBJECT,
        SourceMap.FLAG_SOLID
      }
    }
    for i = 1, 8 do
      local sqr = MapUtils.random_square(self.map, bbox, selector)
      if not sqr then
        return false
      end
      local item = ItemUtils.item_generate(((function()
        if i < 8 then
          return ItemGroups.basic_items
        else
          return ItemGroups.enchanted_items
        end
      end)()))
      MapUtils.spawn_item(self.map, item.type, item.amount, sqr)
    end
    return true
  end
  local add_honeycombs
  add_honeycombs = function(regions)
    local item_placer
    item_placer = function(map_, xy_)
      if map_.rng:chance(0.1) then
        return MapUtils.spawn_item(self.map, "Honeycomb", 1, xy_)
      end
    end
    for i = 1, 20 do
      local _continue_0 = false
      repeat
        if not place_vault_in(self.region_set, Vaults.honeycomb({
          item_placer = item_placer
        })) then
          _continue_0 = true
          break
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return true
  end
  local fill_with_mob
  fill_with_mob = function(part, enemies)
    local group
    group = part.group
    bbox = part:bbox()
    local selector = {
      matches_group = {
        group,
        group
      },
      matches_none = {
        SourceMap.FLAG_HAS_OBJECT,
        SourceMap.FLAG_SOLID
      }
    }
    local monster_focus_point = MapUtils.random_square(self.map, bbox, selector)
    if not monster_focus_point then
      return false
    end
    for _index_0 = 1, #enemies do
      local type = enemies[_index_0]
      local enemy = MapUtils.random_enemy(self.map, type, bbox, selector)
      if not enemy then
        return false
      end
      enemy.monster_wander_position = function(self)
        return monster_focus_point
      end
    end
    return true
  end
  if not add_honeycombs() then
    return false
  end
  if not fill_all() then
    return false
  end
  if not fill_with_mob(parts.B, {
    "Giant Bee",
    "Giant Bee",
    "Giant Bee"
  }) then
    return false
  end
  if not fill_with_mob(parts.E, {
    "Giant Bee",
    "Giant Bee",
    "Giant Bee",
    "Queen Bee"
  }) then
    return false
  end
  if not fill_with_mob(parts.F, {
    "Giant Bee",
    "Giant Bee",
    "Giant Bee",
    "Queen Bee",
    "Queen Bee"
  }) then
    return false
  end
  return true
end
return MapDesc.create({
  map_label = "Hive Entrance",
  size = {
    130,
    130
  },
  default_content = Tilesets.hive.wall,
  children = {
    MapNode.create({
      place = function(self)
        return node_paint_group(self)
      end,
      children = {
        MapNode.create({
          place = node_place_hive_entrance_polys
        })
      }
    })
  }
})
