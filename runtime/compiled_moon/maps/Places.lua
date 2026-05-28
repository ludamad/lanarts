local MapRegion, combine_map_regions, map_regions_bbox
do
  local _obj_0 = require("maps.MapRegion")
  MapRegion, combine_map_regions, map_regions_bbox = _obj_0.MapRegion, _obj_0.combine_map_regions, _obj_0.map_regions_bbox
end
local World = require("core.World")
local MapCompiler
MapCompiler = require("maps.MapCompiler").MapCompiler
local MapUtils = require("maps.MapUtils")
local TileSets = require("tiles.Tilesets")
local GenerateUtils = require("maps.GenerateUtils")
local SourceMap = require("core.SourceMap")
local Map = require("core.Map")
local Region1 = require("maps.Region1")
local OldMaps = require("maps.OldMaps")
local ItemUtils = require("maps.ItemUtils")
local ItemGroups = require("maps.ItemGroups")
local Spread, Shape
do
  local _obj_0 = require("maps.MapElements")
  Spread, Shape = _obj_0.Spread, _obj_0.Shape
end
local cave
cave = function(n_outer)
  if n_outer == nil then
    n_outer = 3
  end
  return Spread({
    regions = {
      (function()
        local _accum_0 = { }
        local _len_0 = 1
        for i = 1, 5 do
          _accum_0[_len_0] = Shape({
            shape = 'deformed_ellipse',
            size = {
              5,
              10
            }
          })
          _len_0 = _len_0 + 1
        end
        return _accum_0
      end)(),
      (function()
        local _accum_0 = { }
        local _len_0 = 1
        for i = 1, n_outer do
          _accum_0[_len_0] = Shape({
            shape = 'deformed_ellipse',
            size = {
              7,
              12
            }
          })
          _len_0 = _len_0 + 1
        end
        return _accum_0
      end)()
    },
    spread_scheme = 'rvo_center',
    connection_scheme = 'direct'
  })
end
local DragonLairFoyer = newtype({
  parent = MapCompiler,
  root_node = Spread({
    regions = (function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 5 do
        _accum_0[_len_0] = Spread({
          regions = {
            (function()
              local _accum_1 = { }
              local _len_1 = 1
              for i = 1, 1 do
                _accum_1[_len_1] = Shape({
                  shape = 'deformed_ellipse',
                  size = {
                    5,
                    10
                  }
                })
                _len_1 = _len_1 + 1
              end
              return _accum_1
            end)(),
            (function()
              local _accum_1 = { }
              local _len_1 = 1
              for i = 1, 1 do
                _accum_1[_len_1] = Shape({
                  shape = 'deformed_ellipse',
                  size = {
                    7,
                    12
                  }
                })
                _len_1 = _len_1 + 1
              end
              return _accum_1
            end)(),
            (function()
              local _accum_1 = { }
              local _len_1 = 1
              for i = 1, 1 do
                _accum_1[_len_1] = Shape({
                  shape = 'deformed_ellipse',
                  size = {
                    12,
                    12
                  }
                })
                _len_1 = _len_1 + 1
              end
              return _accum_1
            end)()
          },
          spread_scheme = 'box2d_solid_center',
          connection_scheme = 'direct'
        })
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)(),
    spread_scheme = 'box2d_solid_center',
    connection_scheme = 'direct'
  }),
  tileset = TileSets.lair,
  enemies = {
    ["Fire Bat"] = 10
  },
  generate = function(self, args)
    for enemy, amount in spairs(self.enemies) do
      for i = 1, amount do
        local sqr = MapUtils.random_square(self.map, nil)
        MapUtils.spawn_enemy(self.map, enemy, sqr)
      end
    end
    for i = 1, 5 do
      ItemUtils.item_object_generate(self.map, ItemGroups.basic_items)
    end
    for i = 1, 3 do
      local sqr = self:random_square_not_near_wall()
      self.map:square_apply(sqr, {
        add = {
          SourceMap.FLAG_SOLID,
          SourceMap.FLAG_HAS_OBJECT
        },
        remove = SourceMap.FLAG_SEETHROUGH
      })
      MapUtils.spawn_decoration(self.map, OldMaps.statue, sqr, self.rng:random(0, 17))
    end
  end
})
local DragonLair = newtype({
  parent = MapCompiler,
  root_node = Spread({
    regions = {
      cave(),
      cave()
    },
    spread_scheme = 'box2d_solid_center',
    connection_scheme = 'direct_light'
  }),
  tileset = TileSets.lair,
  enemies = {
    ["Purple Dragon"] = 1,
    ["Fire Bat"] = 8
  },
  generate = function(self, args)
    for enemy, amount in spairs(self.enemies) do
      for i = 1, amount do
        local sqr = MapUtils.random_square(self.map, nil)
        MapUtils.spawn_enemy(self.map, enemy, sqr)
      end
    end
    for i = 1, 3 do
      local sqr = MapUtils.random_square(self.map, nil)
      Region1.generate_store(self.map, sqr)
    end
    for i = 1, 10 do
      ItemUtils.item_object_generate(self.map, ItemGroups.basic_items)
    end
    for i = 1, 9 do
      local sqr = self:random_square_not_near_wall()
      self.map:square_apply(sqr, {
        add = {
          SourceMap.FLAG_SOLID,
          SourceMap.FLAG_HAS_OBJECT
        },
        remove = SourceMap.FLAG_SEETHROUGH
      })
      MapUtils.spawn_decoration(self.map, OldMaps.statue, sqr, self.rng:random(0, 17))
    end
    for i = 1, 1 do
      local sqr = self:random_square_not_near_wall()
      local randarts = { }
      for j = 1, 1 do
        append(randarts, ItemUtils.randart_generate(2))
      end
      append(randarts, {
        type = "Magentite Key"
      })
      MapUtils.spawn_chest(self.map, sqr, randarts)
    end
    for i = 1, 3 do
      local sqr = self:random_square_not_near_wall()
      MapUtils.spawn_healing_square(self.map, sqr)
    end
    for i = 1, 2 do
      local sqr = self:random_square_not_near_wall()
      local items
      do
        local _accum_0 = { }
        local _len_0 = 1
        for i = 1, 2 do
          _accum_0[_len_0] = {
            type = "Gold",
            amount = self.rng:random(2, 5)
          }
          _len_0 = _len_0 + 1
        end
        items = _accum_0
      end
      MapUtils.spawn_chest(self.map, sqr, items)
    end
  end
})
local SimpleRoom = newtype({
  parent = MapCompiler,
  root_node = Shape({
    shape = 'deformed_ellipse',
    size = {
      20,
      20
    }
  }),
  tileset = TileSets.lair,
  generate = function(self, args)
    local enemies = args.enemies or { }
    for enemy, amount in spairs(enemies) do
      for i = 1, amount do
        local sqr = MapUtils.random_square(self.map, nil)
        MapUtils.spawn_enemy(self.map, enemy, sqr)
      end
    end
    local items = args.items or { }
    for type, amount in spairs(items) do
      if type == "Gold" or _G.items[type].drop_chance ~= nil then
        local sqr = MapUtils.random_square(self.map, nil)
        if sqr then
          MapUtils.spawn_item(self.map, type, amount, sqr)
        end
      else
        for i = 1, amount do
          local sqr = MapUtils.random_square(self.map, nil)
          if sqr then
            MapUtils.spawn_item(self.map, type, 1, sqr)
          end
        end
      end
    end
  end
})
local ChickenCoop = newtype({
  parent = MapCompiler,
  root_node = Shape({
    shape = 'windows',
    size = {
      60,
      60
    }
  }),
  tileset = TileSets.lair,
  generate = function(self, args)
    local enemies = args.enemies or { }
    enemies["Chicken"] = 100
    for enemy, amount in spairs(enemies) do
      for i = 1, amount do
        local sqr = MapUtils.random_square(self.map, nil)
        MapUtils.spawn_enemy(self.map, enemy, sqr)
      end
    end
    local items = args.items or { }
    for type, amount in spairs(items) do
      if type == "Gold" or _G.items[type].drop_chance ~= nil then
        local sqr = MapUtils.random_square(self.map, nil)
        MapUtils.spawn_item(self.map, type, amount, sqr)
      else
        for i = 1, amount do
          local sqr = MapUtils.random_square(self.map, nil)
          MapUtils.spawn_item(self.map, type, 1, sqr)
        end
      end
    end
  end
})
local create_isolated
create_isolated = function(args)
  local MapCompilerContext
  MapCompilerContext = require("maps.MapCompilerContext").MapCompilerContext
  local cc = MapCompilerContext.create()
  cc:register(args.label, args.template)
  return cc:get(args)
end
return nilprotect({
  DragonLair = DragonLair,
  DragonLairFoyer = DragonLairFoyer,
  SimpleRoom = SimpleRoom,
  create_isolated = create_isolated,
  ChickenCoop = ChickenCoop
})
