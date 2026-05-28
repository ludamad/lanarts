local B2GenerateUtils = require("maps.B2GenerateUtils")
local GenerateUtils = require("maps.GenerateUtils")
local DebugUtils = require("maps.DebugUtils")
local SourceMap = require("core.SourceMap")
local MapRegionShapes = require("maps.MapRegionShapes")
local Map = require("core.Map")
local MapUtils = require("maps.MapUtils")
local World = require("core.World")
local Vaults = require("maps.Vaults")
local MapRegion, combine_map_regions, map_region_bbox, map_regions_bbox
do
  local _obj_0 = require("maps.MapRegion")
  MapRegion, combine_map_regions, map_region_bbox, map_regions_bbox = _obj_0.MapRegion, _obj_0.combine_map_regions, _obj_0.map_region_bbox, _obj_0.map_regions_bbox
end
local Spread, Shape
do
  local _obj_0 = require("maps.MapElements")
  Spread, Shape = _obj_0.Spread, _obj_0.Shape
end
local make_polygon_points
make_polygon_points = function(rng, w, h, n_points)
  return GenerateUtils.skewed_ellipse_points(rng, {
    0,
    0
  }, {
    30,
    30
  }, n_points or rng:random(4, 8))
end
local expand_if_nested
expand_if_nested = function(value)
  if type(value) == "table" and type(value.type) == "string" then
    return value.type, value
  else
    return value, { }
  end
end
local MapCompiler = newtype({
  init = function(self, args)
    self.result = { }
    self.operators = { }
    self.post_poned = { }
    self.label = assert((args.label or self.label))
    self.root_node = assert((args.root or self.root_node))
    self.rng = args.rng or require('mtwist').create(random(0, 2 ^ 30))
    self._children = { }
    self._regions = { }
    self._group_sets = { }
    self._combined_region = { }
    self._bboxes = { }
    self._spawn_players = args.spawn_players or false
    self._next_group_id = 1
  end,
  add = function(self, selector, operator)
    return append(self.operators, {
      selector = selector,
      operator = operator
    })
  end,
  with_map = function(self, f)
    return append(self.post_poned, f)
  end,
  random_player_square = function(self, player)
    return MapUtils.random_square(self.map, nil)
  end,
  get_player_spawn_points = function(self)
    log_verbose("get_player_spawn_points " .. tostring(self.label) .. " " .. tostring(#World.players))
    local spawn_points
    do
      local _accum_0 = { }
      local _len_0 = 1
      local _list_0 = World.players
      for _index_0 = 1, #_list_0 do
        local player = _list_0[_index_0]
        log_verbose("get_player_spawn_points " .. tostring(player.name))
        local sqr = nil
        for i = 1, 40 do
          sqr = self:random_player_square(player)
          if sqr then
            break
          end
        end
        if sqr == nil then
          return nil
        end
        local x, y
        x, y = sqr[1], sqr[2]
        local _value_0 = {
          x * 32 + 16,
          y * 32 + 16
        }
        _accum_0[_len_0] = _value_0
        _len_0 = _len_0 + 1
      end
      spawn_points = _accum_0
    end
    return spawn_points
  end,
  get_map_region = function(self, node)
    assert(self._regions[node], "Must run compile_map_topology on node first!")
    local regions
    do
      local _accum_0 = { }
      local _len_0 = 1
      local _list_0 = self._children[node]
      for _index_0 = 1, #_list_0 do
        local child = _list_0[_index_0]
        _accum_0[_len_0] = self:get_map_region(child)
        _len_0 = _len_0 + 1
      end
      regions = _accum_0
    end
    return combine_map_regions(regions)
  end,
  _recalculate_perimeter = function(self)
    SourceMap.perimeter_apply({
      map = self.map,
      candidate_selector = {
        matches_all = SourceMap.FLAG_SOLID
      },
      inner_selector = {
        matches_none = SourceMap.FLAG_SOLID
      },
      operator = {
        add = SourceMap.FLAG_PERIMETER
      }
    })
    return SourceMap.perimeter_apply({
      map = self.map,
      candidate_selector = {
        matches_none = {
          SourceMap.FLAG_SOLID
        }
      },
      inner_selector = {
        matches_all = {
          SourceMap.FLAG_PERIMETER,
          SourceMap.FLAG_SOLID
        }
      },
      operator = {
        add = Vaults.FLAG_INNER_PERIMETER
      }
    })
  end,
  _generate_shape = function(self, scheme, x, y, w, h)
    local scheme_args
    scheme, scheme_args = expand_if_nested(scheme)
    local _exp_0 = scheme
    if 'deformed_ellipse' == _exp_0 then
      local points = scheme_args.n_points or self.rng:random(5, 8)
      local polygon = GenerateUtils.skewed_ellipse_points(self.rng, {
        x,
        y
      }, {
        w,
        h
      }, points)
      return {
        polygon
      }
    elseif 'ellipse' == _exp_0 then
      local points = scheme_args.n_points or self.rng:random(5, 8)
      local polygon = GenerateUtils.ellipse_points_0(self.rng, {
        x,
        y
      }, {
        w,
        h
      }, points)
      return {
        polygon
      }
    elseif 'rectangle' == _exp_0 then
      if self.rng:randomf() > .5 then
        w, h = h, w
      end
      local polygon = GenerateUtils.ellipse_points_0(self.rng, {
        x,
        y
      }, {
        w,
        h
      }, 4)
      return {
        polygon
      }
    else
      local parts = scheme:split(':')
      local name = parts[1]
      if parts[2] == 'convex' then
        local polygon = MapRegionShapes.get_shape_convex_hull(name, x, y, w, h)
        return {
          polygon
        }
      else
        local polygons = MapRegionShapes.get_shape_polygons(name, x, y, w, h)
        return polygons
      end
    end
  end,
  _spread_regions = function(self, scheme, regions)
    local timer = timer_create()
    if type(scheme) == "function" then
      return scheme(self, node)
    end
    self:scatter_regions(regions)
    local scheme_args
    scheme, scheme_args = expand_if_nested(scheme)
    local _exp_0 = scheme
    if 'box2d' == _exp_0 then
      B2GenerateUtils.spread_map_regions({
        rng = self.rng,
        regions = regions,
        n_iterations = 100,
        mode = 'towards_center',
        clump_once_near = true
      })
    elseif 'box2d_solid_center' == _exp_0 then
      B2GenerateUtils.spread_map_regions({
        rng = self.rng,
        regions = regions,
        fixed_polygons = self:_generate_shape('deformed_ellipse', 0, 0, 4, 4),
        n_iterations = 100,
        mode = 'towards_fixed_shapes',
        clump_once_near = true
      })
    elseif 'rvo_spread' == _exp_0 or 'rvo_center' == _exp_0 or 'rvo_ring' == _exp_0 then
      local spreader = GenerateUtils.RVORegionPlacer.create()
      local adapters
      do
        local _accum_0 = { }
        local _len_0 = 1
        for _index_0 = 1, #regions do
          local r = regions[_index_0]
          _accum_0[_len_0] = GenerateUtils.MapRegionAdapter.create(r)
          _len_0 = _len_0 + 1
        end
        adapters = _accum_0
      end
      local velocity_func
      local _exp_1 = scheme
      if 'rvo_spread' == _exp_1 then
        velocity_func = GenerateUtils.spread_region_delta_func
      elseif 'rvo_center' == _exp_1 then
        velocity_func = GenerateUtils.center_region_delta_func
      elseif 'rvo_ring' == _exp_1 then
        velocity_func = GenerateUtils.ring_region_delta_func
      end
      for _index_0 = 1, #adapters do
        local r = adapters[_index_0]
        spreader:add(r, velocity_func(nil, self.rng, {
          center = function(self)
            return 0, 0
          end
        }))
      end
      for i = 1, scheme_args.iters or 1000 do
        spreader:step()
      end
      for i, r in ipairs(regions) do
        local x, ox, y, oy
        do
          local _obj_0 = adapters[i]
          x, ox, y, oy = _obj_0.x, _obj_0.ox, _obj_0.y, _obj_0.oy
        end
        r:translate(x - ox, y - oy)
      end
    else
      error("Unexpected")
    end
    return log_verbose("Spread regions time: " .. tostring(timer:get_milliseconds()) .. "ms")
  end,
  property = function(self, node, property, default)
    if default == nil then
      default = nil
    end
    if node.properties[property] == nil then
      return default
    end
    return ode.properties[property]
  end,
  connect_map_regions = function(self, regions, n_connections)
    if n_connections == nil then
      n_connections = 2
    end
    return B2GenerateUtils.connect_map_regions({
      rng = self.rng,
      regions = regions,
      n_connections = n_connections
    })
  end,
  _connect_regions = function(self, scheme, regions)
    local scheme_args
    scheme, scheme_args = expand_if_nested(scheme)
    local _exp_0 = scheme
    if 'direct_light' == _exp_0 then
      local timer = timer_create()
      self:connect_map_regions(regions, 2)
      return log_verbose("Connect regions time: " .. tostring(timer:get_milliseconds()) .. "ms")
    elseif 'direct' == _exp_0 then
      local timer = timer_create()
      self:connect_map_regions(regions, #regions * 2)
      return log_verbose("Connect regions time: " .. tostring(timer:get_milliseconds()) .. "ms")
    elseif 'minimum_spanning_arc_and_line' == _exp_0 then
      return self:with_map(function(map)
        local adapters
        do
          local _accum_0 = { }
          local _len_0 = 1
          for _index_0 = 1, #regions do
            local r = regions[_index_0]
            _accum_0[_len_0] = GenerateUtils.MapRegionAdapter.create(r)
            _len_0 = _len_0 + 1
          end
          adapters = _accum_0
        end
        local edges = GenerateUtils.subregion_minimum_spanning_tree(adapters, function()
          local r1 = self.rng:random(scheme_args.acceptable_dist or 12)
          local r2 = self.rng:random(scheme_args.acceptable_dist or 12)
          return r1 + r2
        end)
        map.arc_chance = 0.2
        local NewMaps = require("maps.NewMaps")
        return NewMaps.connect_edges(map, self.rng, {
          floor1 = self.tileset.floor,
          floor2 = self.tileset.floor_alt,
          connect_line_width = function()
            if self.rng:randomf() < 0.2 then
              return 2
            end
            return 1
          end
        }, {
          1,
          1,
          map.size[1] - 1,
          map.size[2] - 1
        }, edges)
      end)
    elseif 'random_tunnels' == _exp_0 then
      return error("Unexpected")
    elseif 'none' == _exp_0 then
      return nil
    else
      return error("Unexpected")
    end
  end,
  _flatten = function(self, l, accum)
    if accum == nil then
      accum = { }
    end
    if getmetatable(l) then
      append(accum, l)
    else
      for _index_0 = 1, #l do
        local elem = l[_index_0]
        self:_flatten(elem, accum)
      end
    end
    return accum
  end,
  scatter_regions = function(self, regions, scatter_x, scatter_y)
    if scatter_x == nil then
      scatter_x = 20
    end
    if scatter_y == nil then
      scatter_y = 20
    end
    for _index_0 = 1, #regions do
      local region = regions[_index_0]
      local x, y = self.rng:random(-scatter_x, scatter_x), self.rng:random(-scatter_y, scatter_y)
      region:translate(x, y)
    end
    return nil
  end,
  _prepare_children_topology = function(self, parent, regions, children_set)
    local _list_0 = self:_flatten(regions)
    for _index_0 = 1, #_list_0 do
      local region_node = _list_0[_index_0]
      append(children_set, region_node)
      self:_prepare_map_topology(region_node, parent)
    end
  end,
  _prepare_map_topology = function(self, node, parent)
    if parent == nil then
      parent = nil
    end
    local map_regions = { }
    local children_set = { }
    local combined_region = nil
    node.parent = parent
    node.properties = node.properties or { }
    if parent ~= nil then
      for k, v in spairs(parent.properties) do
        local _continue_0 = false
        repeat
          if node.properties[k] ~= nil then
            _continue_0 = true
            break
          end
          node.properties[k] = v
          _continue_0 = true
        until true
        if not _continue_0 then
          break
        end
      end
    end
    local _exp_0 = getmetatable(node)
    if Spread == _exp_0 then
      local name, regions, connection_scheme, spread_scheme
      name, regions, connection_scheme, spread_scheme = node.name, node.regions, node.connection_scheme, node.spread_scheme
      assert(#regions > 0)
      self:_prepare_children_topology(node, regions, children_set)
      local child_regions
      do
        local _accum_0 = { }
        local _len_0 = 1
        for _index_0 = 1, #regions do
          local child = regions[_index_0]
          _accum_0[_len_0] = self._combined_region[child]
          _len_0 = _len_0 + 1
        end
        child_regions = _accum_0
      end
      assert(#child_regions > 0)
      if type(spread_scheme) == "function" then
        local val = node.spread_scheme(self, node)
        if val == false then
          return false
        end
      else
        local val = self:_spread_regions(spread_scheme, child_regions)
        if val == false then
          return false
        end
      end
      if type(connection_scheme) == "function" then
        local val = node.connection_scheme(self, node)
        if val == false then
          return false
        end
      else
        local val = self:_connect_regions(connection_scheme, child_regions)
        if val == false then
          return false
        end
      end
      assert(#child_regions > 0)
      combined_region = combine_map_regions(child_regions)
      assert(#combined_region.polygons > 0)
      DebugUtils.visualize_map_regions({
        regions = child_regions,
        title = "After connection / spread"
      })
    elseif Shape == _exp_0 then
      local name, shape, size, spread_scheme
      name, shape, size, spread_scheme = node.name, node.shape, node.size, node.spread_scheme
      local w, h
      w, h = size[1], size[2]
      shape = self:_generate_shape(shape, -w / 2, -h / 2, w, h)
      local region = MapRegion.create(shape)
      assert(#region.polygons > 0)
      append(map_regions, region)
      if node.regions then
        local scatter_x, scatter_y = math.min(20, w / 10), math.min(20, h / 10)
        self:_prepare_children_topology(node, node.regions, children_set)
        local child_regions
        do
          local _accum_0 = { }
          local _len_0 = 1
          local _list_0 = node.regions
          for _index_0 = 1, #_list_0 do
            local child = _list_0[_index_0]
            _accum_0[_len_0] = self._combined_region[child]
            _len_0 = _len_0 + 1
          end
          child_regions = _accum_0
        end
        self:scatter_regions(child_regions, scatter_x, scatter_y)
        B2GenerateUtils.spread_map_regions({
          rng = self.rng,
          regions = child_regions,
          fixed_polygons = table.reversed(shape),
          n_iterations = 100,
          mode = 'towards_center',
          clump_once_near = true
        })
        combined_region = combine_map_regions(child_regions)
        combined_region = combine_map_regions({
          region,
          combined_region
        })
        assert(#combined_region.polygons > 0)
      else
        combined_region = region
        assert(#combined_region.polygons > 0)
      end
      DebugUtils.visualize_map_regions({
        regions = map_regions,
        title = "Shape"
      })
    else
      error("Unknown node")
    end
    self._regions[node] = map_regions
    self._children[node] = children_set
    self._combined_region[node] = assert(combined_region, "Need combined_region ~= nil!")
    assert(#self._combined_region[node].polygons > 0)
    local _list_0 = self._combined_region[node].polygons
    for _index_0 = 1, #_list_0 do
      local polygon = _list_0[_index_0]
      for _index_1 = 1, #polygon do
        local _des_0 = polygon[_index_1]
        local x, y
        x, y = _des_0[1], _des_0[2]
        assert(math.abs(x) ~= math.huge and math.abs(y) ~= math.huge)
      end
    end
    return true
  end,
  as_owned_regions = function(self, node)
    return self._regions[node]
  end,
  as_children = function(self, node)
    return self._children[node]
  end,
  as_region = function(self, node)
    return self._combined_region[node]
  end,
  as_size = function(self, node)
    local bbox = self:as_bbox(node)
    local w, h = (bbox[3] - bbox[1]), (bbox[4] - bbox[2])
    return {
      w,
      h
    }
  end,
  as_bbox = function(self, node, padding)
    if padding == nil then
      padding = 0
    end
    self._bboxes[node] = map_region_bbox(self._combined_region[node])
    if padding ~= 0 then
      local w, h
      do
        local _obj_0 = self.map.size
        w, h = _obj_0[1], _obj_0[2]
      end
      local x1, y1, x2, y2
      do
        local _obj_0 = self._bboxes[node]
        x1, y1, x2, y2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
      end
      return {
        math.max(0, math.min(x1 - padding, w)),
        math.max(0, math.min(y1 - padding, h)),
        math.max(0, math.min(x2 + padding, w)),
        math.max(0, math.min(y2 + padding, h))
      }
    end
    return self._bboxes[node]
  end,
  as_group_set = function(self, node)
    return self._group_sets[node]
  end,
  fill_unconnected = function(self)
    local w, h
    do
      local _obj_0 = self.map.size
      w, h = _obj_0[1], _obj_0[2]
    end
    return SourceMap.area_fill_unconnected({
      map = self.map,
      seed = {
        w / 2,
        h / 2
      },
      unfilled_selector = {
        matches_none = {
          SourceMap.FLAG_SOLID
        }
      },
      mark_operator = {
        add = {
          SourceMap.FLAG_RESERVED2
        }
      },
      marked_selector = {
        matches_all = {
          SourceMap.FLAG_RESERVED2
        }
      },
      fill_operator = {
        content = self.tileset.wall,
        add = SourceMap.FLAG_SOLID,
        remove = SourceMap.FLAG_SEETHROUGH
      }
    })
  end,
  apply = function(self, node, args, tunnel_args)
    assert(not args.map, "Passing map here redundant!")
    args.map = self.map
    if tunnel_args then
      tunnel_args.map = self.map
    end
    local _list_0 = self._regions[node]
    for _index_0 = 1, #_list_0 do
      local region = _list_0[_index_0]
      region:tunnel_apply(tunnel_args or args)
      region:inner_apply(args)
    end
  end,
  for_all_nodes = function(self, func)
    local recurse
    recurse = function(node)
      local _list_0 = self._children[node]
      for _index_0 = 1, #_list_0 do
        local child = _list_0[_index_0]
        recurse(child)
      end
      return func(self, node)
    end
    return recurse(self.root_node)
  end,
  tile_paint = function(self, node, floor_tile, tunnel_tile)
    if not floor_tile then
      floor_tile = ((function()
        if node.group % 2 == 1 then
          return self.tileset.floor
        else
          return self.tileset.floor_alt
        end
      end)())
    end
    if not tunnel_tile then
      tunnel_tile = ((function()
        if node.group % 2 == 1 then
          return self.tileset.floor_alt
        else
          return self.tileset.floor
        end
      end)())
    end
    return self:apply(node, {
      operator = {
        remove = {
          SourceMap.FLAG_SOLID,
          SourceMap.FLAG_TUNNEL
        },
        add = SourceMap.FLAG_SEETHROUGH,
        content = floor_tile,
        group = node.group
      }
    }, {
      operator = {
        remove = SourceMap.FLAG_SOLID,
        add = {
          SourceMap.FLAG_SEETHROUGH,
          SourceMap.FLAG_TUNNEL
        },
        content = tunnel_tile,
        group = node.group
      }
    })
  end,
  tile_paint_perimeter = function(self, node, wall_tile)
    if wall_tile == nil then
      wall_tile = nil
    end
    if not wall_tile then
      wall_tile = ((function()
        if node.group % 2 == 1 then
          return self.tileset.wall
        else
          return self.tileset.wall_alt
        end
      end)())
    end
    return SourceMap.perimeter_apply({
      map = self.map,
      candidate_selector = {
        matches_all = SourceMap.FLAG_SOLID
      },
      inner_selector = {
        matches_none = SourceMap.FLAG_SOLID,
        matches_group = self._group_sets[node]
      },
      operator = {
        content = wall_tile
      }
    })
  end,
  paint_children = function(self, node)
    local _list_0 = self._children[node]
    for _index_0 = 1, #_list_0 do
      local child = _list_0[_index_0]
      local val = self:paint_node(child)
      if val == false then
        return false
      end
    end
    return true
  end,
  paint_node = function(self, node)
    node.group = SourceMap.group_create(self.map, {
      0,
      0,
      0,
      0
    }, 0)
    if node.paint then
      local val = node.paint(self, node)
      if val == false then
        return false
      end
    else
      self:tile_paint(node, node.properties.floor_tile, node.properties.tunnel_tile)
    end
    if not self:paint_children(node) then
      return false
    end
    local group_child_max = SourceMap.map_num_groups(self.map) - 1
    self._group_sets[node] = {
      node.group,
      group_child_max
    }
    self:tile_paint_perimeter(node, node.properties.wall_tile)
    return true
  end,
  _shuffle = function(self, tbl)
    for i = #tbl, 1, -1 do
      local rand = self.rng:random(1, #tbl + 1)
      tbl[i], tbl[rand] = tbl[rand], tbl[i]
    end
    return tbl
  end,
  node_match = function(self, node, args, shuffle)
    if shuffle == nil then
      shuffle = true
    end
    args.map = self.map
    args.area = self:as_bbox(node, args.padding)
    args.selector.matches_group = self:as_group_set(node)
    local squares = SourceMap.rectangle_match(args)
    if shuffle then
      self:_shuffle(squares)
    end
    return squares
  end,
  place_children_objects = function(self, node)
    local _list_0 = self._children[node]
    for _index_0 = 1, #_list_0 do
      local child = _list_0[_index_0]
      local val = self:place_node_objects(child)
      if val == false then
        return false
      end
    end
    return true
  end,
  place_node_objects = function(self, node)
    if node.place_objects then
      local val = node.place_objects(self, node)
      if val == false then
        return false
      end
    end
    return self:place_children_objects(node)
  end,
  _prepare_source_map = function(self, label, padding, content)
    local bbox = map_region_bbox(self._combined_region[self.root_node])
    local w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
    if math.abs(w) == math.huge or math.abs(h) == math.huge then
      return false
    end
    self._combined_region[self.root_node]:translate(-bbox[1] + padding, -bbox[2] + padding)
    self:for_all_nodes(function(self, node)
      local _list_0 = self._combined_region[node].polygons
      for _index_0 = 1, #_list_0 do
        local polygon = _list_0[_index_0]
        for _index_1 = 1, #polygon do
          local _des_0 = polygon[_index_1]
          local x, y
          x, y = _des_0[1], _des_0[2]
          assert(x >= padding - 0.1 and x <= w + padding + 0.1, "pad=" .. tostring(padding) .. ", " .. tostring(x) .. ", " .. tostring(w))
          assert(y >= padding - 0.1 and y <= h + padding + 0.1, "pad=" .. tostring(padding) .. ", " .. tostring(y) .. ", " .. tostring(h))
        end
      end
      local _list_1 = self._combined_region[node].tunnels
      for _index_0 = 1, #_list_1 do
        local _des_0 = _list_1[_index_0]
        local polygon
        polygon = _des_0.polygon
        for _index_1 = 1, #polygon do
          local _des_1 = polygon[_index_1]
          local x, y
          x, y = _des_1[1], _des_1[2]
          assert(x >= padding - 0.1 and x <= w + padding + 0.1, "pad=" .. tostring(padding) .. ", " .. tostring(x) .. ", " .. tostring(w))
          assert(y >= padding - 0.1 and y <= h + padding + 0.1, "pad=" .. tostring(padding) .. ", " .. tostring(y) .. ", " .. tostring(h))
        end
      end
    end)
    self.map = SourceMap.map_create({
      rng = self.rng,
      label = label,
      size = {
        w + padding * 2,
        h + padding * 2
      },
      flags = SourceMap.FLAG_SOLID,
      instances = { },
      content = content
    })
    if not self:paint_node(self.root_node) then
      return false
    end
    self:_recalculate_perimeter()
    if not self:place_node_objects(self.root_node) then
      return false
    end
    if self._spawn_players then
      local spawn_points = self:get_player_spawn_points()
      if not spawn_points then
        return false
      end
      self._player_spawn_points = spawn_points
    end
    local _list_0 = self.post_poned
    for _index_0 = 1, #_list_0 do
      local callback = _list_0[_index_0]
      callback(self.map)
    end
    SourceMap.erode_diagonal_pairs({
      map = self.map,
      rng = self.rng,
      selector = {
        matches_all = SourceMap.FLAG_SOLID
      }
    })
    if not SourceMap.area_fully_connected({
      map = self.map,
      unfilled_selector = {
        matches_none = {
          SourceMap.FLAG_SOLID
        }
      },
      mark_operator = {
        add = {
          SourceMap.FLAG_RESERVED2
        }
      },
      marked_selector = {
        matches_all = {
          SourceMap.FLAG_RESERVED2
        }
      }
    }) then
      log_verbose("ABORT: connection check failed")
      return false
    end
    self:_recalculate_perimeter()
    return true
  end,
  random_square = function(self, area)
    if area == nil then
      area = nil
    end
    return MapUtils.random_square(self.map, area, {
      matches_none = {
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
  end,
  random_square_not_near_wall = function(self, area)
    if area == nil then
      area = nil
    end
    return MapUtils.random_square(self.map, area, {
      matches_none = {
        Vaults.FLAG_INNER_PERIMETER,
        SourceMap.FLAG_HAS_OBJECT,
        Vaults.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    })
  end,
  prepare = function(self, args)
    if args == nil then
      args = { }
    end
    local MAX_TRIES = 1000
    for i = 1, MAX_TRIES do
      local _continue_0 = false
      repeat
        table.clear(self._children)
        table.clear(self._regions)
        table.clear(self._group_sets)
        table.clear(self._combined_region)
        table.clear(self._bboxes)
        local padding = args.padding or 10
        local content = args.content or ((function()
          if rawget(self, 'tileset') then
            return self.tileset.wall
          else
            return 0
          end
        end)())
        if not self:_prepare_map_topology(self.root_node) then
          _continue_0 = true
          break
        end
        local success = self:_prepare_source_map(self.label, padding, content)
        if success then
          return true
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return false
  end,
  compile = function(self)
    local gmap = Map.create({
      map = self.map,
      label = self.map.label,
      instances = self.map.instances,
      wandering_enabled = true
    })
    if self._spawn_players then
      World = require("core.World")
      World.players_spawn(gmap, self._player_spawn_points)
      if os.getenv("LANARTS_XP") then
        local _list_0 = World.players
        for _index_0 = 1, #_list_0 do
          local p = _list_0[_index_0]
          p.instance:gain_xp(tonumber(os.getenv("LANARTS_XP")))
        end
      end
    end
    return gmap
  end,
  linker = function(self)
    local desc = {
      generate = function(_self, back_links, forward_links)
        if back_links == nil then
          back_links = { }
        end
        if forward_links == nil then
          forward_links = { }
        end
        local args = {
          back_links = back_links,
          forward_links = forward_links
        }
        if not self:prepare() then
          error("Map generation completely failed!!")
        end
        self:generate(args)
        return self:compile(args)
      end,
      map_label = self.label
    }
    return require("maps.MapLinks").MapLinker.create(desc)
  end
})
return {
  MapCompiler = MapCompiler
}
