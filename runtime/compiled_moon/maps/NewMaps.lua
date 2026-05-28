local map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator
do
  local _obj_0 = require("maps.GenerateUtils")
  map_place_object, ellipse_points, LEVEL_PADDING, Region, RVORegionPlacer, random_rect_in_rect, random_ellipse_in_ellipse, ring_region_delta_func, default_region_delta_func, spread_region_delta_func, center_region_delta_func, towards_region_delta_func, random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree, Tile, tile_operator = _obj_0.map_place_object, _obj_0.ellipse_points, _obj_0.LEVEL_PADDING, _obj_0.Region, _obj_0.RVORegionPlacer, _obj_0.random_rect_in_rect, _obj_0.random_ellipse_in_ellipse, _obj_0.ring_region_delta_func, _obj_0.default_region_delta_func, _obj_0.spread_region_delta_func, _obj_0.center_region_delta_func, _obj_0.towards_region_delta_func, _obj_0.random_region_add, _obj_0.subregion_minimum_spanning_tree, _obj_0.region_minimum_spanning_tree, _obj_0.Tile, _obj_0.tile_operator
end
local MapUtils = require("maps.MapUtils")
local make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper = MapUtils.make_tunnel_oper, MapUtils.make_rectangle_criteria, MapUtils.make_rectangle_oper
local B2WorldUtils = require("maps.B2WorldUtils")
local Vaults = require("maps.Vaults")
local SourceMap = require("core.SourceMap")
local Map = require("core.Map")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_ROOM
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_ROOM = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_ROOM
local make_rooms_with_tunnels
make_rooms_with_tunnels = function(map, rng, conf, area)
  local oper = SourceMap.random_placement_operator({
    size_range = conf.rect_room_size_range,
    rng = rng,
    area = area,
    amount_of_placements_range = conf.rect_room_num_range,
    create_subgroup = false,
    child_operator = function(map_, subgroup, bounds)
      local queryfn
      queryfn = function()
        local query = make_rectangle_criteria()
        return query(map_, subgroup, bounds)
      end
      local oper
      oper = make_rectangle_oper(conf.floor2.id, conf.wall2.id, conf.wall2.seethrough, queryfn)
      if oper(map_, subgroup, bounds) then
        append(map_.rectangle_rooms, bounds)
        return true
      end
      return false
    end
  })
  oper(map, SourceMap.ROOT_GROUP, area)
  local tunnel_oper = make_tunnel_oper(rng, conf.floor1.id, conf.wall1.id, conf.wall1.seethrough)
  tunnel_oper(map, SourceMap.ROOT_GROUP, area)
  return map
end
local generate_game_map
generate_game_map = function(map, place_object, place_monsters)
  local M = Map.create({
    map = map,
    label = assert(map.map_label),
    instances = map.instances,
    wandering_enabled = map.wandering_enabled
  })
  return M
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
      append(flags, SourceMap.FLAG_TUNNEL)
    end
    if p2.id % 5 <= 3 then
      tile = conf.floor2
      append(flags, FLAG_ALTERNATE)
    end
    local fapply
    if chance(map.arc_chance) then
      fapply = p1.arc_connect
    else
      fapply = p1.line_connect
    end
    fapply(p1, {
      map = map,
      area = area,
      target = p2,
      line_width = line_width,
      operator = (tile_operator(tile, {
        matches_none = FLAG_ALTERNATE,
        matches_all = SourceMap.FLAG_SOLID,
        add = flags
      }))
    })
  end
end
local connect_edges_with_pos_pairs
connect_edges_with_pos_pairs = function(map, rng, conf, area, pos_pairs)
  for _index_0 = 1, #pos_pairs do
    local _des_0 = pos_pairs[_index_0]
    local p1, p2
    p1, p2 = _des_0[1], _des_0[2]
    local tile = conf.floor1
    local flags = { }
    local line_width = conf.connect_line_width()
    if line_width <= 2 then
      append(flags, SourceMap.FLAG_TUNNEL)
    end
    if rng:chance(0.2) then
      tile = conf.floor2
      append(flags, FLAG_ALTERNATE)
    end
    local fapply = nil
    if false then
      fapply = SourceMap.arc_apply
    else
      fapply = SourceMap.line_apply
    end
    assert(p1 and p2)
    fapply({
      map = map,
      area = area,
      from_xy = p1,
      to_xy = p2,
      line_width = line_width,
      operator = (tile_operator(tile, {
        matches_none = FLAG_ALTERNATE,
        matches_all = SourceMap.FLAG_SOLID,
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
generate_area = function(map, rng, conf, outer, padding, starting_edges)
  if starting_edges == nil then
    starting_edges = { }
  end
  local size = conf.size
  local R = RVORegionPlacer.create({
    outer.points
  })
  for i = 1, conf.number_regions do
    local r, n_points, angle = conf.room_radius(), rng:random(3, 10), rng:randomf(0, math.pi)
    local x1, y1, x2, y2
    do
      local _obj_0 = outer:bbox()
      x1, y1, x2, y2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
    end
    r = random_region_add(rng, r * 2, r * 2, n_points, conf.region_delta_func(map, rng, outer), angle, R, {
      x1 + padding,
      y1 + padding,
      x2 - padding,
      y2 - padding
    }, true)
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
  for _index_0 = 1, #starting_edges do
    local _des_0 = starting_edges[_index_0]
    local p1, p2
    p1, p2 = _des_0[1], _des_0[2]
    add_edge_if_unique(p1, p2)
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
  connect_edges(map, rng, conf, outer:bbox(), edges)
  local _list_1 = R.regions
  for _index_0 = 1, #_list_1 do
    local region = _list_1[_index_0]
    if map.rng:chance(.5) then
      SourceMap.perimeter_apply({
        map = map,
        area = region:bbox(),
        candidate_selector = {
          matches_all = SourceMap.FLAG_SOLID
        },
        inner_selector = {
          matches_all = FLAG_ALTERNATE,
          matches_none = SourceMap.FLAG_SOLID
        },
        operator = tile_operator(outer.conf.wall2)
      })
    end
  end
  return true
end
local generate_subareas
generate_subareas = function(template, map, rng, regions, starting_edges)
  if starting_edges == nil then
    starting_edges = { }
  end
  local conf = template.outer_conf
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    if not generate_area(map, rng, region.conf, region, template.shell, starting_edges) then
      return false
    end
  end
  local edges = subregion_minimum_spanning_tree(regions, function()
    return rng:random(12) + rng:random(12)
  end)
  connect_edges(map, rng, conf, nil, edges)
  SourceMap.erode_diagonal_pairs({
    map = map,
    rng = rng,
    selector = {
      matches_all = SourceMap.FLAG_SOLID
    }
  })
  SourceMap.perimeter_apply({
    map = map,
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
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    make_rooms_with_tunnels(map, rng, region.conf, region:bbox())
  end
  return true
end
local generate_door_candidates
generate_door_candidates = function(map, rng, regions)
  SourceMap.perimeter_apply({
    map = map,
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
      add = FLAG_INNER_PERIMETER
    }
  })
  for _index_0 = 1, #regions do
    local region = regions[_index_0]
    if region.conf.is_overworld or region.conf.is_small_overworld then
      local _list_0 = region.subregions
      for _index_1 = 1, #_list_0 do
        local subregion = _list_0[_index_1]
        subregion:apply({
          map = map,
          operator = {
            remove = SourceMap.FLAG_TUNNEL
          }
        })
      end
    end
  end
  SourceMap.perimeter_apply({
    map = map,
    candidate_selector = {
      matches_all = {
        SourceMap.FLAG_TUNNEL
      },
      matches_none = {
        FLAG_ROOM,
        SourceMap.FLAG_SOLID
      }
    },
    inner_selector = {
      matches_all = {
        FLAG_ROOM
      },
      matches_none = {
        FLAG_DOOR_CANDIDATE,
        SourceMap.FLAG_SOLID
      }
    },
    operator = {
      add = FLAG_DOOR_CANDIDATE
    }
  })
  local filter_door_candidates
  filter_door_candidates = function(x1, y1, x2, y2)
    return SourceMap.rectangle_apply({
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
local source_map_create
source_map_create = function(args)
  local rng, size, default_content, default_flags, map_label, arc_chance, wandering_enabled
  rng, size, default_content, default_flags, map_label, arc_chance, wandering_enabled = args.rng, args.size, args.default_content, args.default_flags, args.map_label, args.arc_chance, args.wandering_enabled
  return SourceMap.map_create({
    rng = rng,
    size = size,
    content = default_content,
    flags = default_flags,
    map_label = map_label,
    arc_chance = arc_chance or 0,
    next_group = 0,
    wandering_enabled = (function()
      if wandering_enabled == nil then
        return true
      else
        return wandering_enabled
      end
    end)(),
    door_locations = { },
    instances = { },
    portal_spawns = { },
    post_maps = { },
    post_game_map = { },
    rectangle_rooms = { },
    player_candidate_squares = { }
  })
end
local map_from_template
map_from_template = function(rng, template)
  return source_map_create({
    rng = rng,
    size = {
      template.w,
      template.h
    },
    default_content = template.default_wall.id,
    default_flags = {
      SourceMap.FLAG_SOLID,
      (function()
        if template.seethrough then
          return SourceMap.FLAG_SEETHROUGH
        else
          return 0
        end
      end)()
    },
    map_label = template.map_label,
    arc_chance = template.arc_chance,
    wandering_enabled = template.wandering_enabled
  })
end
local map_try_create
map_try_create = function(map, rng, template)
  event_log("(RNG #%d) before map_try_create", rng:amount_generated())
  local PW, PH
  PW, PH = LEVEL_PADDING[1], LEVEL_PADDING[2]
  local mw, mh
  do
    local _obj_0 = map.size
    mw, mh = _obj_0[1], _obj_0[2]
  end
  local outer = Region.create(1 + PW, 1 + PH, mw - PW, mh - PH)
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
  local _list_0 = assert(template.subtemplates, "Must have subtemplates!")
  for _index_0 = 1, #_list_0 do
    local subconf = _list_0[_index_0]
    local w, h
    do
      local _obj_0 = subconf.size
      w, h = _obj_0[1], _obj_0[2]
    end
    assert(w and h, "Subconf must have size")
    local r = random_region_add(rng, w, h, 20, center_region_delta_func(map, rng, outer), 0, major_regions, outer:bbox())
    if r == nil then
      print("ABORT: region could not be added")
      return nil
    end
    local rx1, ry1, rx2, ry2
    do
      local _obj_0 = r:bbox()
      rx1, ry1, rx2, ry2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
    end
    event_log("(RNG #%d) created region at (%d,%d,%d,%d)", rng:amount_generated(), rx1, ry1, rx2, ry2)
    r.max_speed = 8
    r.conf = subconf
  end
  local starting_edges = { }
  major_regions:steps(1500)
  local tx1, ty1, tx2, ty2 = math.huge, math.huge, -math.huge, -math.huge
  local _list_1 = major_regions.regions
  for _index_0 = 1, #_list_1 do
    local r = _list_1[_index_0]
    local rx1, ry1, rx2, ry2
    do
      local _obj_0 = r:bbox()
      rx1, ry1, rx2, ry2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
    end
    tx1, ty1 = math.min(tx1, rx1), math.min(ty1, ry1)
    tx2, ty2 = math.max(tx2, rx2), math.max(ty2, ry2)
  end
  local tdx, tdy = ((mw - tx2) - tx1) / 2, ((mh - ty2) - ty1) / 2
  local _list_2 = major_regions.regions
  for _index_0 = 1, #_list_2 do
    local r = _list_2[_index_0]
    r.x, r.y = r.x + tdx, r.y + tdy
    local rx1, ry1, rx2, ry2
    do
      local _obj_0 = r:bbox()
      rx1, ry1, rx2, ry2 = _obj_0[1], _obj_0[2], _obj_0[3], _obj_0[4]
    end
    event_log("(RNG #%d) after RVO, have region at (%d,%d,%d,%d)", rng:amount_generated(), rx1, ry1, rx2, ry2)
    r._points = false
    r:apply({
      map = map,
      operator = (tile_operator(r.conf.wall1))
    })
  end
  generate_subareas(template, map, rng, major_regions.regions, starting_edges)
  map.regions = major_regions.regions
  event_log("(RNG #%d) after map_try_create", rng:amount_generated())
  return map
end
local check_connection
check_connection = function(map)
  return SourceMap.area_fully_connected({
    map = map,
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
  })
end
local _map_create_pass
_map_create_pass = function(rng, template_f)
  local template = template_f(rng)
  local map = map_from_template(rng, template)
  if not map_try_create(map, rng, template) then
    return nil
  end
  if not template:on_create_source_map(map) then
    print("ABORT: on_create_source_map")
    return nil
  end
  if not check_connection(map) then
    print("ABORT: connection check failed")
    return nil
  end
  for _, map_gen_func in ipairs(map.post_maps) do
    map_gen_func()
  end
  local game_map = generate_game_map(map)
  template:on_create_game_map(game_map)
  local _list_0 = map.post_game_map
  for _index_0 = 1, #_list_0 do
    local f = _list_0[_index_0]
    f(game_map)
  end
  return game_map
end
local new_rng
new_rng = function()
  local seed = random(0, 2 ^ 30)
  event_log("(RNG 0) Creating mtwist object with seed=%d", seed)
  return require("mtwist").create(seed)
end
local try_n_times
try_n_times = function(n, f)
  for i = 1, n do
    local ret = f(i)
    if ret then
      return ret
    end
    print("** MAP GENERATION ATTEMPT " .. i .. " FAILED, RETRYING **")
  end
  return error("Could not generate a viable map in " .. n .. " tries!")
end
local map_create
map_create = function(template)
  local rng = new_rng()
  event_log("(RNG #%d) Attempting map_create", rng:amount_generated())
  for i = 1, 1000 do
    local map = _map_create_pass(rng, template)
    if map then
      return map
    end
    print("** MAP GENERATION ATTEMPT " .. i .. " FAILED, RETRYING **")
  end
  return error("Could not generate a viable map in 1000 tries!")
end
return {
  map_create = map_create,
  generate_door_candidates = generate_door_candidates,
  connect_edges = connect_edges,
  generate_game_map = generate_game_map,
  map_try_create = map_try_create,
  check_connection = check_connection,
  try_n_times = try_n_times,
  new_rng = new_rng,
  source_map_create = source_map_create
}
