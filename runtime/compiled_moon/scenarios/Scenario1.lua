local MapRegion, combine_map_regions, map_regions_bbox
do
  local _obj_0 = require("maps.MapRegion")
  MapRegion, combine_map_regions, map_regions_bbox = _obj_0.MapRegion, _obj_0.combine_map_regions, _obj_0.map_regions_bbox
end
local place_feature
place_feature = require("maps.TemplateGeneration").place_feature
local World = require("core.World")
local MapCompiler
MapCompiler = require("maps.MapCompiler").MapCompiler
local Vaults = require("maps.Vaults")
local MapUtils = require("maps.MapUtils")
local TileSets = require("tiles.Tilesets")
local GenerateUtils = require("maps.GenerateUtils")
local has_overlaps
has_overlaps = require("maps.B2GenerateUtils").has_overlaps
local SourceMap = require("core.SourceMap")
local Map = require("core.Map")
local Region1 = require("maps.Region1")
local OldMaps = require("maps.OldMaps")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local get_door_candidates
get_door_candidates = require("maps.DoorGeneration").get_door_candidates
local Spread, Shape
do
  local _obj_0 = require("maps.MapElements")
  Spread, Shape = _obj_0.Spread, _obj_0.Shape
end
local place_doors
place_doors = function(self, node)
  local _list_0 = get_door_candidates(self, node)
  for _index_0 = 1, #_list_0 do
    local xy = _list_0[_index_0]
    MapUtils.spawn_door(self.map, xy)
  end
end
local place_encounter_behind_door
place_encounter_behind_door = function(self, node, enemies)
  local _list_0 = get_door_candidates(self, node)
  for _index_0 = 1, #_list_0 do
    local xy = _list_0[_index_0]
    MapUtils.spawn_door(self.map, xy, nil, Vaults._rune_door_closed, "dummykey")
  end
  local enemy_candidate_squares = self:node_match(node, {
    padding = 10,
    selector = {
      matches_all = {
        SourceMap.FLAG_TUNNEL
      },
      matches_none = {
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID,
        FLAG_DOOR_CANDIDATE
      }
    }
  })
  if #enemy_candidate_squares < #enemies then
    return false
  end
  for i = 1, 0 do
    MapUtils.spawn_enemy(self.map, enemies[i], enemy_candidate_squares[i])
  end
end
local N_JUNCTION_TRIES = 4
local junction
junction = function(nodes)
  local connector_node = Shape({
    shape = "deformed_ellipse",
    size = {
      20,
      20
    },
    properties = {
      wall_tile = TileSets.hive.wall
    }
  })
  return Spread({
    regions = table.tconcat({
      connector_node
    }, nodes),
    spread_scheme = function(self)
      local jw, jh
      do
        local _obj_0 = self:as_size(connector_node)
        jw, jh = _obj_0[1], _obj_0[2]
      end
      local regions
      do
        local _accum_0 = { }
        local _len_0 = 1
        for _index_0 = 1, #nodes do
          local node = nodes[_index_0]
          _accum_0[_len_0] = self:as_region(node)
          _len_0 = _len_0 + 1
        end
        regions = _accum_0
      end
      append(regions, self:as_region(connector_node))
      for _index_0 = 1, #nodes do
        local node = nodes[_index_0]
        local region = self:as_region(node)
        region:rotate(self.rng:randomf(-math.pi, math.pi))
        local angle = self:property(node, "angle", self.rng:randomf(-math.pi, math.pi))
        local drift = self:property(node, "drift", self.rng:randomf(1.4, 1.7))
        local w, h
        do
          local _obj_0 = self:as_size(node)
          w, h = _obj_0[1], _obj_0[2]
        end
        for i = 1, N_JUNCTION_TRIES do
          local dx, dy = (w * drift + jw) / 1.5 * math.cos(angle), (h * drift + jh) / 1.5 * math.sin(angle)
          region:translate(dx, dy)
          if has_overlaps({
            regions = regions
          }) then
            if i == N_JUNCTION_TRIES then
              return false
            end
            region:translate(-dx, -dy)
            angle = self.rng:randomf(-math.pi, math.pi)
            drift = self.rng:randomf(1.4, 1.7)
          else
            break
          end
        end
      end
      return true
    end,
    properties = {
      tunnel_tile = TileSets.lair.floor
    },
    connection_scheme = function(self)
      for _index_0 = 1, #nodes do
        local node = nodes[_index_0]
        self:_connect_regions('direct_light', {
          self:as_region(node),
          self:as_region(connector_node)
        })
      end
    end
  })
end
local create_scenario
create_scenario = function(rng)
  local initial_room = Shape({
    shape = 'deformed_ellipse',
    size = {
      20,
      20
    },
    properties = {
      wall_tile = TileSets.hive.wall
    },
    paint = function(self, node)
      return self:tile_paint(node, TileSets.hive.floor)
    end
  })
  local enemy_enclosure1 = Shape({
    shape = 'deformed_ellipse',
    size = {
      20,
      20
    },
    properties = {
      wall_tile = TileSets.hive.wall,
      floor_tile = TileSets.hive.floor_alt
    }
  })
  local enemy_enclosure2 = Shape({
    shape = 'deformed_ellipse',
    size = {
      20,
      20
    },
    properties = {
      wall_tile = TileSets.hive.wall,
      floor_tile = TileSets.hive.floor_alt
    },
    place_objects = place_doors
  })
  local root_node = junction({
    initial_room,
    junction({
      enemy_enclosure1,
      enemy_enclosure2
    })
  })
  root_node.place_objects = function(self, node)
    local item_placer
    item_placer = function(map, xy)
      local item = ItemUtils.item_generate(ItemGroups.basic_items)
      return MapUtils.spawn_item(map, item.type, item.amount, xy)
    end
    for i = 1, 2 do
      local vault = SourceMap.area_template_create(Vaults.small_item_vault({
        rng = self.rng,
        item_placer = item_placer,
        tileset = TileSets.lair
      }))
      if not place_feature(self, node, vault) then
        break
      end
      vault = SourceMap.area_template_create(Vaults.stone_henge({
        rng = self.rng,
        item_placer = item_placer,
        tileset = TileSets.lair
      }))
      if not place_feature(self, node, vault) then
        break
      end
    end
    local val = place_encounter_behind_door(self, initial_room, (function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 10 do
        if self.rng:random(0, 4) ~= 0 then
          _accum_0[_len_0] = 'Centaur Hunter'
        elseif self.rng:random(0, 4) == 0 then
          _accum_0[_len_0] = 'Centaur Marksman'
        else
          _accum_0[_len_0] = 'Giant Spider'
        end
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
    if val == false then
      return false
    end
  end
  place_feature = require("maps.TemplateGeneration").place_feature
  local random_player_square
  random_player_square = function(self)
    return MapUtils.random_square(self.map, self:as_bbox(initial_room), {
      matches_group = self:as_group_set(initial_room),
      matches_none = {
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID,
        SourceMap.FLAG_TUNNEL
      }
    })
  end
  return newtype({
    parent = MapCompiler,
    root_node = root_node,
    tileset = TileSets.lair,
    generate = do_nothing,
    random_player_square = random_player_square
  })
end
return {
  create_scenario = create_scenario
}
