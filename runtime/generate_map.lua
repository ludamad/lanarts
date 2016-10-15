local map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator
do
  local _obj_0 = require("@generate_util")
  map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator = _obj_0.map_place_object, _obj_0.ellipse_points, _obj_0.LEVEL_PADDING, _obj_0.Region, _obj_0.RVORegionPlacer, _obj_0.random_rect_in_rect, _obj_0.random_ellipse_in_ellipse, _obj_0.ring_region_delta_func, _obj_0.default_region_delta_func, _obj_0.spread_region_delta_func, _obj_0.random_region_add, _obj_0.subregion_minimum_spanning_tree, _obj_0.region_minimum_spanning_tree, _obj_0.Tile, _obj_0.tile_operator
end
local print_map, make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
do
  local _obj_0 = require("@map_util")
  print_map, make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper = _obj_0.print_map, _obj_0.make_tunnel_oper, _obj_0.make_rectangle_criteria, _obj_0.make_rectangle_oper
end
local TileMap
TileMap = require("core").TileMap
local FLAG_ALTERNATE = TileMap.FLAG_CUSTOM1
local FLAG_INNER_PERIMETER = TileMap.FLAG_CUSTOM2
local FLAG_DOOR_CANDIDATE = TileMap.FLAG_CUSTOM3
local FLAG_OVERWORLD = TileMap.FLAG_CUSTOM4
local FLAG_ROOM = TileMap.FLAG_CUSTOM5
local OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE = 150, 150
local OVERWORLD_CONF
OVERWORLD_CONF = function(rng)
  return {
    map_label = "Plain Valley",
    size = {
      45,
      45
    },
    number_regions = rng:random(35, 40),
    floor1 = Tile.create('grass1', false, true),
    floor2 = Tile.create('grass2', false, true),
    wall1 = Tile.create('tree', true, true, {
      FLAG_OVERWORLD
    }),
    wall2 = Tile.create('dungeon_wall', true, false),
    rect_room_num_range = {
      0,
      0
    },
    rect_room_size_range = {
      10,
      15
    },
    rvo_iterations = 150,
    n_shops = rng:random(2, 4),
    n_stairs_down = 0,
    n_stairs_up = 0,
    connect_line_width = function()
      return rng:random(2, 6)
    end,
    region_delta_func = ring_region_delta_func,
    room_radius = function()
      local r = 2
      local bound = rng:random(1, 10)
      for j = 1, rng:random(0, bound) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end,
    monster_weights = function()
      return {
        ["Giant Rat"] = 0,
        ["Chicken"] = 0,
        ["Cloud Elemental"] = 1,
        ["Turtle"] = 8,
        ["Spriggan"] = 2
      }
    end,
    n_statues = 4
  }
end
local DUNGEON_CONF
DUNGEON_CONF = function(rng)
  local C = { }
  local _exp_0 = rng:random(3)
  if 0 == _exp_0 then
    C.floor1 = Tile.create('grey_floor', false, true)
    C.floor2 = Tile.create('reddish_grey_floor', false, true)
    C.wall1 = Tile.create('dungeon_wall', true, false, { }, {
      FLAG_OVERWORLD
    })
    C.wall2 = Tile.create('crypt_wall', true, false, { }, {
      FLAG_OVERWORLD
    })
  elseif 1 == _exp_0 then
    C.floor1 = Tile.create('crystal_floor1', false, true)
    C.floor2 = Tile.create('crystal_floor2', false, true)
    C.wall1 = Tile.create('crystal_wall', true, false, { }, {
      FLAG_OVERWORLD
    })
    C.wall2 = Tile.create('crystal_wall2', true, false, { }, {
      FLAG_OVERWORLD
    })
  elseif 2 == _exp_0 then
    C.floor1 = Tile.create('pebble_floor1', false, true)
    C.floor2 = Tile.create('pebble_floor2', false, true)
    C.wall1 = Tile.create('pebble_wall1', true, false, { }, {
      FLAG_OVERWORLD
    })
    C.wall2 = Tile.create('pebble_wall3', true, false, { }, {
      FLAG_OVERWORLD
    })
  end
  local _exp_1 = rng:random(3)
  if 0 == _exp_1 then
    C.number_regions = rng:random(5, 10)
    C.room_radius = function()
      local r = 4
      for j = 1, rng:random(0, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      2,
      3
    }
    C.rect_room_size_range = {
      10,
      15
    }
  elseif 1 == _exp_1 then
    C.number_regions = rng:random(5, 20)
    C.room_radius = function()
      local r = 2
      for j = 1, rng:random(0, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      2,
      10
    }
    C.rect_room_size_range = {
      7,
      15
    }
  elseif 2 == _exp_1 then
    C.number_regions = rng:random(2, 7)
    C.room_radius = function()
      local r = 2
      for j = 1, rng:random(0, 10) do
        r = r + rng:randomf(0, 1)
      end
      return r
    end
    C.rect_room_num_range = {
      10,
      15
    }
    C.rect_room_size_range = {
      7,
      15
    }
  end
  return table.merge(C, {
    map_label = "A Dungeon",
    size = {
      85,
      85
    },
    rvo_iterations = 20,
    n_shops = rng:random(2, 4),
    n_stairs_down = 3,
    n_stairs_up = 0,
    connect_line_width = function()
      return 2 + ((function()
        if rng:random(5) == 4 then
          return 1
        else
          return 0
        end
      end)())
    end,
    region_delta_func = default_region_delta_func,
    monster_weights = function()
      return {
        ["Giant Rat"] = 8,
        ["Cloud Elemental"] = 1,
        ["Chicken"] = 1
      }
    end,
    n_statues = 4
  })
end
local make_rooms_with_tunnels
make_rooms_with_tunnels = function(map, rng, conf, area)
  local oper = TileMap.random_placement_operator({
    size_range = conf.rect_room_size_range,
    rng = rng,
    area = area,
    amount_of_placements_range = conf.rect_room_num_range,
    create_subgroup = false,
    child_operator = function(map, subgroup, bounds)
      local queryfn
      queryfn = function()
        local query = make_rectangle_criteria()
        return query(map, subgroup, bounds)
      end
      oper = make_rectangle_oper(conf.floor2.id, conf.wall2.id, conf.wall2.seethrough, queryfn)
      if oper(map, subgroup, bounds) then
        append(map.rectangle_rooms, bounds)
        return true
      end
      return false
    end
  })
  oper(map, TileMap.ROOT_GROUP, area)
  local tunnel_oper = make_tunnel_oper(rng, conf.floor1.id, conf.wall1.id, conf.wall1.seethrough)
  tunnel_oper(map, TileMap.ROOT_GROUP, area)
  return map
end
local connect_edges
connect_edges = function(map, rng, conf, area, edges)
  for _index_0 = 1, #edges do
    local _des_0 = edges[_index_0]
    local p1, p2
    p1, p2 = _des_0[1], _des_0[2]
    local tile = conf.floor1
    local flags = { }
    local rad1, rad2 = math.max(p1.w, p1.h) / 2, math.max(p2.w, p2.h) / 2
    local line_width = conf.connect_line_width()
    if line_width <= 2 and p1:ortho_dist(p2) > (rng:random(3, 6) + rad1 + rad2) then
      append(flags, TileMap.FLAG_TUNNEL)
    end
    if p2.id % 5 <= 3 then
      tile = conf.floor2
      append(flags, FLAG_ALTERNATE)
    end
    local fapply = nil
    if rng:random(4) < 2 then
      fapply = p1.line_connect
    else
      fapply = p1.arc_connect
    end
    fapply(p1, {
      map = map,
      area = area,
      target = p2,
      line_width = line_width,
      operator = (tile_operator(tile, {
        matches_none = FLAG_ALTERNATE,
        add = flags
      }))
    })
  end
end
local make_rect_points
make_rect_points = function(x1, y1, x2, y2)
  return {
    {
      x1,
      y2
    },
    {
      x2,
      y2
    },
    {
      x2,
      y1
    },
    {
      x1,
      y1
    }
  }
end
local generate_area
generate_area = function(map, rng, conf, outer)
  local size = conf.size
  local R = RVORegionPlacer.create({
    outer.points
  })
  for i = 1, conf.number_regions do
    local r, n_points, angle = conf.room_radius(), rng:random(3, 10), rng:randomf(0, math.pi)
    r = random_region_add(rng, r * 2, r * 2, n_points, conf.region_delta_func(map, rng, outer), angle, R, outer:bbox(), true)
    if r then
      outer:add(r)
    end
  end
  R:steps(conf.rvo_iterations)
  local _list_0 = R.regions
  for _index_0 = 1, #_list_0 do
    local region = _list_0[_index_0]
    local tile = ((function()
      if rng:random(4) ~= 1 then
        return conf.floor1
      else
        return conf.floor2
      end
    end)())
    region:apply({
      map = map,
      area = outer:bbox(),
      operator = (tile_operator(tile, {
        add = FLAG_ROOM
      }))
    })
  end
  local edges = region_minimum_spanning_tree(R.regions)
  local add_edge_if_unique
  add_edge_if_unique = function(p1, p2)
    for _index_0 = 1, #edges do
      local _des_0 = edges[_index_0]
      local op1, op2
      op1, op2 = _des_0[1], _des_0[2]
      if op1 == p1 and op2 == p2 or op2 == p1 and op1 == p2 then
        return 
      end
    end
    return append(edges, {
      p1,
      p2
    })
  end
  for i = 1, #R.regions do
    for j = i + 1, #R.regions do
      if rng:random(0, 3) == 1 then
        local p1, p2 = R.regions[i], R.regions[j]
        local dist = math.sqrt((p2.x - p1.x) ^ 2 + (p2.y - p1.y) ^ 2)
        if dist < rng:random(5, 15) then
          add_edge_if_unique(p1, p2)
        end
      end
    end
  end
  return connect_edges(map, rng, conf, outer:bbox(), edges)
end
local generate_subareas
generate_subareas = function(map, rng, regions)
  local conf = OVERWORLD_CONF(rng)
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    generate_area(map, rng, region.conf, region)
  end
  local edges = subregion_minimum_spanning_tree(regions, function()
    return rng:random(12) + rng:random(12)
  end)
  connect_edges(map, rng, conf, nil, edges)
  TileMap.erode_diagonal_pairs({
    map = map,
    rng = rng,
    selector = {
      matches_all = TileMap.FLAG_SOLID
    }
  })
  TileMap.perimeter_apply({
    map = map,
    candidate_selector = {
      matches_all = TileMap.FLAG_SOLID
    },
    inner_selector = {
      matches_none = TileMap.FLAG_SOLID
    },
    operator = {
      add = TileMap.FLAG_PERIMETER
    }
  })
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    TileMap.perimeter_apply({
      map = map,
      area = region:bbox(),
      candidate_selector = {
        matches_all = TileMap.FLAG_SOLID
      },
      inner_selector = {
        matches_all = FLAG_ALTERNATE,
        matches_none = TileMap.FLAG_SOLID
      },
      operator = tile_operator(region.conf.wall2)
    })
  end
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    make_rooms_with_tunnels(map, rng, region.conf, region:bbox())
  end
  TileMap.perimeter_apply({
    map = map,
    candidate_selector = {
      matches_none = {
        TileMap.FLAG_SOLID
      }
    },
    inner_selector = {
      matches_all = {
        TileMap.FLAG_PERIMETER,
        TileMap.FLAG_SOLID
      }
    },
    operator = {
      add = FLAG_INNER_PERIMETER
    }
  })
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local _list_0 = region.subregions
    for _index_1 = 1, #_list_0 do
      local subregion = _list_0[_index_1]
      subregion:apply({
        map = map,
        operator = {
          remove = TileMap.FLAG_TUNNEL
        }
      })
    end
  end
  TileMap.rectangle_apply({
    map = map,
    fill_operator = {
      matches_all = FLAG_OVERWORLD,
      remove = TileMap.FLAG_TUNNEL
    }
  })
  TileMap.perimeter_apply({
    map = map,
    candidate_selector = {
      matches_all = {
        TileMap.FLAG_TUNNEL
      },
      matches_none = {
        FLAG_ROOM,
        TileMap.FLAG_SOLID
      }
    },
    inner_selector = {
      matches_all = {
        FLAG_ROOM
      },
      matches_none = {
        FLAG_DOOR_CANDIDATE,
        TileMap.FLAG_SOLID
      }
    },
    operator = {
      add = FLAG_DOOR_CANDIDATE
    }
  })
  local filter_door_candidates
  filter_door_candidates = function(x1, y1, x2, y2)
    return TileMap.rectangle_apply({
      map = map,
      fill_operator = {
        remove = FLAG_DOOR_CANDIDATE
      },
      area = {
        x1,
        y1,
        x2,
        y2
      }
    })
  end
  local filter_random_third
  filter_random_third = function(x1, y1, x2, y2)
    local w, h = (x2 - x1), (y2 - y1)
    if rng:random(0, 2) == 0 then
      filter_door_candidates(x2 + w / 3, y1 - 1, x2 + 1, y2 + 1)
    end
    if rng:random(0, 2) == 0 then
      filter_door_candidates(x1 - 1, y1 - 1, x1 + w / 3, y2 + 1)
    end
    if rng:random(0, 2) == 0 then
      filter_door_candidates(x1 - 1, y1 + h / 3, x2 + 1, y2 + 1)
    end
    if rng:random(0, 2) == 0 then
      return filter_door_candidates(x1 - 1, y1 - 1, x2 + 1, y2 - h / 3)
    end
  end
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    local _list_0 = region.subregions
    for _index_1 = 1, #_list_0 do
      local _des_0 = _list_0[_index_1]
      local x, y, w, h
      x, y, w, h = _des_0.x, _des_0.y, _des_0.w, _des_0.h
      filter_random_third(x, y, x + w, y + h)
    end
  end
  local _list_0 = map.rectangle_rooms
  for _index_0 = 1, #_list_0 do
    local _des_0 = _list_0[_index_0]
    local x1, y1, x2, y2
    x1, y1, x2, y2 = _des_0[1], _des_0[2], _des_0[3], _des_0[4]
    filter_random_third(x1 + 1, y1 + 1, x2 - 1, y2 - 1)
  end
end
local generate_overworld
generate_overworld = function(rng)
  local conf = OVERWORLD_CONF(rng)
  local PW, PH
  PW, PH = LEVEL_PADDING[1], LEVEL_PADDING[2]
  local mw, mh = nil, nil
  if rng:random(0, 2) == 1 then
    mw, mh = OVERWORLD_DIM_LESS, OVERWORLD_DIM_MORE
  else
    mw, mh = OVERWORLD_DIM_MORE, OVERWORLD_DIM_LESS
  end
  local outer = Region.create(1 + PW, 1 + PH, mw - PW, mh - PH)
  local rect = {
    {
      1 + PW,
      1 + PH
    },
    {
      mw - PW,
      1 + PH
    },
    {
      mw - PW,
      mh - PH
    },
    {
      1 + PW,
      mh - PH
    }
  }
  local rect2 = {
    {
      1 + PW,
      mh - PH
    },
    {
      mw - PW,
      mh - PH
    },
    {
      mw - PW,
      1 + PH
    },
    {
      1 + PW,
      1 + PH
    }
  }
  local major_regions = RVORegionPlacer.create({
    rect2
  })
  local map = TileMap.map_create({
    size = {
      mw,
      mh
    },
    content = conf.wall1.id,
    flags = conf.wall1.add_flags,
    map_label = conf.map_label,
    door_locations = { },
    rectangle_rooms = { }
  })
  local _list_0 = {
    DUNGEON_CONF(rng),
    OVERWORLD_CONF(rng),
    OVERWORLD_CONF(rng),
    OVERWORLD_CONF(rng),
    OVERWORLD_CONF(rng),
    DUNGEON_CONF(rng)
  }
  for _index_0 = 1, #_list_0 do
    local subconf = _list_0[_index_0]
    local w, h
    do
      local _obj_0 = subconf.size
      w, h = _obj_0[1], _obj_0[2]
    end
    local r = random_region_add(rng, w, h, 20, spread_region_delta_func(map, rng, outer), 0, major_regions, outer:bbox())
    if r ~= nil then
      r.max_speed = 10
      r.conf = subconf
    end
  end
  local _list_1 = major_regions.regions
  for _index_0 = 1, #_list_1 do
    local r = _list_1[_index_0]
    r._points = false
    r:apply({
      map = map,
      operator = (tile_operator(r.conf.wall1))
    })
  end
  generate_subareas(map, rng, major_regions.regions)
  map.regions = major_regions.regions
  return map
end
local generate_game_map
generate_game_map = function(G, map)
  local map_state
  map_state = require("core").map_state
  local map_place_monsters
  do
    local _obj_0 = require("@generate_objects")
    map_place_object, map_place_monsters = _obj_0.map_place_object, _obj_0.map_place_monsters
  end
  local Feature
  Feature = require('@map_object_types').Feature
  local M = map_state.create_map_state(G, 1, G.rng, map.map_label, map)
  local gen_feature
  gen_feature = function(sprite, solid, seethrough)
    if seethrough == nil then
      seethrough = true
    end
    return function(px, py)
      return Feature.create(M, {
        x = px * 32 + 16,
        y = py * 32 + 16,
        sprite = sprite,
        solid = solid,
        seethrough = seethrough
      })
    end
  end
  local _list_0 = map.regions
  for _index_0 = 1, #_list_0 do
    local region = _list_0[_index_0]
    local area = region:bbox()
    local conf = region.conf
    for i = 1, conf.n_statues do
      map_place_object(M, gen_feature('statues', true, false), area, {
        matches_none = {
          FLAG_INNER_PERIMETER,
          TileMap.FLAG_HAS_OBJECT,
          TileMap.FLAG_SOLID
        }
      })
    end
    local _list_1 = TileMap.rectangle_match({
      map = map,
      selector = {
        matches_none = {
          TileMap.FLAG_HAS_OBJECT,
          TileMap.FLAG_SOLID
        },
        matches_all = {
          FLAG_DOOR_CANDIDATE
        }
      }
    })
    for _index_1 = 1, #_list_1 do
      local _des_0 = _list_1[_index_1]
      local x, y
      x, y = _des_0[1], _des_0[2]
      gen_feature('door_closed', true, false)(x, y)
    end
    for i = 1, conf.n_shops do
      map_place_object(M, gen_feature('shops', false), area, {
        matches_none = {
          TileMap.FLAG_HAS_OBJECT,
          TileMap.FLAG_SOLID
        }
      })
    end
    for i = 1, conf.n_stairs_down do
      map_place_object(M, gen_feature('stairs_down', false), area, {
        matches_none = {
          TileMap.FLAG_HAS_OBJECT,
          TileMap.FLAG_SOLID
        }
      })
    end
    for i = 1, conf.n_stairs_up do
      map_place_object(M, gen_feature('stairs_up', false), area, {
        matches_none = {
          TileMap.FLAG_HAS_OBJECT,
          TileMap.FLAG_SOLID
        }
      })
    end
    map_place_monsters(M, conf.monster_weights(), area)
  end
  return M
end
return {
  generate_overworld = generate_overworld,
  generate_game_map = generate_game_map
}
