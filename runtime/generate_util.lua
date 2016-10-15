local RVOWorld, TileMap, data
do
  local _obj_0 = require("core")
  RVOWorld, TileMap, data = _obj_0.RVOWorld, _obj_0.TileMap, _obj_0.data
end
local LEVEL_PADDING = {
  5,
  5
}
local MAX_TRIES = 1000
local ellipse_points
ellipse_points = function(x, y, w, h, n_points, start_angle)
  if n_points == nil then
    n_points = 16
  end
  if start_angle == nil then
    start_angle = 0
  end
  local points = { }
  local angle, step = start_angle, (1 / n_points * 2 * math.pi)
  local cx, cy = x + w / 2, y + h / 2
  for i = 1, n_points do
    append(points, {
      (math.sin(angle) + 1) / 2 * w + x,
      (math.cos(angle) + 1) / 2 * h + y
    })
    angle = angle + step
  end
  return points
end
local Region = newtype({
  init = function(self, x, y, w, h, n_points, angle)
    if n_points == nil then
      n_points = 16
    end
    if angle == nil then
      angle = 0
    end
    self.x, self.y, self.w, self.h, self.n_points, self.angle = x, y, w, h, n_points, angle
    self._points = false
    self.subregions = { }
  end,
  add = function(self, subregion)
    return append(self.subregions, subregion)
  end,
  get = {
    points = function(self)
      self._points = self._points or ellipse_points(self.x, self.y, self.w, self.h, self.n_points, self.angle)
      return self._points
    end
  },
  apply = function(self, args)
    args.points = self.points
    return TileMap.polygon_apply(args)
  end,
  bbox = function(self)
    return {
      self.x,
      self.y,
      self.x + self.w,
      self.y + self.h
    }
  end,
  ortho_dist = function(self, o)
    local cx, cy = self:center()
    local ocx, ocy = o:center()
    local dx, dy = cx - ocx, cy - ocy
    return math.max(math.abs(dx), math.abs(dy))
  end,
  square_distance = function(self, o)
    local cx, cy = self:center()
    local ocx, ocy = o:center()
    local dx, dy = cx - ocx, cy - ocy
    return dx * dx + dy * dy
  end,
  ellipse_intersect = function(self, x, y, w, h)
    local cx, cy = self:center()
    local cxo, cyo = x + w / 2, y + h / 2
    local dx, dy = cxo - cx, cyo - cy
    local ang = math.atan2(dy, dx)
    local r1 = math.sqrt((self.w / 2 * math.cos(ang)) ^ 2 + (self.h / 2 * math.sin(ang)) ^ 2)
    local r2 = math.sqrt((w / 2 * math.cos(-ang)) ^ 2 + (h / 2 * math.sin(-ang)) ^ 2)
    return (math.sqrt(dx * dx + dy * dy) < (r1 + r2))
  end,
  rect_intersect = function(self, x, y, w, h)
    if self.x > x + w or x > self.x + self.w then
      return false
    end
    if self.y > y + h or y > self.y + self.h then
      return false
    end
    return true
  end,
  center = function(self)
    return math.floor(self.x + self.w / 2), math.floor(self.y + self.h / 2)
  end,
  line_connect = function(self, args)
    args.from_xy = {
      self:center()
    }
    args.to_xy = {
      args.target:center()
    }
    return TileMap.line_apply(args)
  end,
  line_match = function(self, args)
    args.from_xy = {
      self:center()
    }
    args.to_xy = {
      args.target:center()
    }
    return TileMap.line_match(args)
  end,
  _arc_adjust_params = function(self, args)
    local cx, cy = self:center()
    local ocx, ocy = args.target:center()
    local w, h = math.abs(cx - ocx) - 1, math.abs(cy - ocy) - 1
    if w < 2 or h < 2 or w > 15 or h > 15 then
      return false
    end
    args.width, args.height = w * 2, h * 2
    args.x, args.y = math.floor((cx + ocx) / 2), math.floor((cy + ocy) / 2)
    local a1 = math.atan2((args.y - cy) / h, (args.x - cx) / w)
    local a2 = math.atan2((args.y - ocy) / h, (args.x - ocx) / w)
    args.angle1, args.angle2 = a1 + math.pi / 2, (a2 - a1)
  end,
  arc_connect = function(self, args)
    if not self:_arc_adjust_params(args) then
      return self:line_connect(args)
    end
    return TileMap.arc_apply(args)
  end,
  arc_match = function(self, args)
    if not self:_arc_adjust_params(args) then
      return self:line_match(args)
    end
    return TileMap.arc_match(args)
  end
})
local _edge_list_append_if_unique
_edge_list_append_if_unique = function(edges, p1, p2)
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
local subregion_minimum_spanning_tree
subregion_minimum_spanning_tree = function(R, acceptable_distf)
  if acceptable_distf == nil then
    acceptable_distf = nil
  end
  local C
  do
    local _tbl_0 = { }
    for _index_0 = 1, #R do
      local p = R[_index_0]
      local _key_0, _val_0 = false
      _tbl_0[_key_0] = _val_0
    end
    C = _tbl_0
  end
  C[1] = true
  local edge_list = { }
  while true do
    local min_sqr_dist = math.huge
    local min_i, min_j = nil, nil
    local sub_i, sub_j = nil, nil
    for i = 1, #R do
      if C[i] then
        for j = 1, #R do
          if not C[j] then
            local SI, SJ = R[i].subregions, R[j].subregions
            for _index_0 = 1, #SI do
              local si = SI[_index_0]
              for _index_1 = 1, #SJ do
                local sj = SJ[_index_1]
                local sqr_dist = si:square_distance(sj)
                if acceptable_distf and sqr_dist < acceptable_distf(si, sj) ^ 2 then
                  _edge_list_append_if_unique(edge_list, si, sj)
                elseif sqr_dist < min_sqr_dist then
                  min_sqr_dist = sqr_dist
                  min_i, min_j = i, j
                  sub_i, sub_j = sj, si
                end
              end
            end
          end
        end
      end
    end
    if min_i == nil then
      break
    end
    C[min_j] = true
    append(edge_list, {
      sub_i,
      sub_j
    })
  end
  return edge_list
end
local region_minimum_spanning_tree
region_minimum_spanning_tree = function(R)
  local C
  do
    local _tbl_0 = { }
    for _index_0 = 1, #R do
      local p = R[_index_0]
      local _key_0, _val_0 = false
      _tbl_0[_key_0] = _val_0
    end
    C = _tbl_0
  end
  C[1] = true
  local edge_list = { }
  while true do
    local min_sqr_dist = math.huge
    local min_i, min_j = nil, nil
    for i = 1, #R do
      if C[i] then
        for j = 1, #R do
          if not C[j] then
            local sqr_dist = R[i]:square_distance(R[j])
            if sqr_dist < min_sqr_dist then
              min_sqr_dist = sqr_dist
              min_i, min_j = i, j
            end
          end
        end
      end
    end
    if min_i == nil then
      break
    end
    C[min_j] = true
    append(edge_list, {
      R[min_i],
      R[min_j]
    })
  end
  return edge_list
end
local RVORegionPlacer = newtype({
  init = function(self, boundary)
    if boundary == nil then
      boundary = nil
    end
    self.rvo = RVOWorld.create(boundary)
    self.regions = { }
  end,
  add = function(self, region, velocity_func)
    append(self.regions, region)
    local x, y, w, h
    x, y, w, h = region.x, region.y, region.w, region.h
    region.max_speed = rawget(region, "max_speed") or 1
    local r = math.max(w, h)
    region.id = self.rvo:add_instance(x + w / 2, y + h / 2, math.ceil(r), region.max_speed)
    region.velocity_func = velocity_func
  end,
  step = function(self)
    local _list_0 = self.regions
    for _index_0 = 1, #_list_0 do
      local region = _list_0[_index_0]
      local cx, cy = region:center()
      local vx, vy = region:velocity_func()
      local r = math.max(region.w, region.h)
      self.rvo:update_instance(region.id, cx, cy, math.ceil(r), region.max_speed, vx, vy)
    end
    self.rvo:step()
    local _list_1 = self.regions
    for _index_0 = 1, #_list_1 do
      local region = _list_1[_index_0]
      local vx, vy = self.rvo:get_velocity(region.id)
      region.x, region.y = region.x + vx, region.y + vy
    end
  end,
  steps = function(self, n)
    for i = 1, n do
      self:step()
    end
    return self:finish()
  end,
  finish = function(self)
    local _list_0 = self.regions
    for _index_0 = 1, #_list_0 do
      local region = _list_0[_index_0]
      region.x, region.y = math.round(region.x), math.round(region.y)
    end
  end
})
local random_rect_in_rect
random_rect_in_rect = function(rng, w, h, xo, yo, wo, ho)
  return rng:random(xo, xo + wo - w), rng:random(yo, yo + ho - h), w, h
end
local random_ellipse_in_ellipse
random_ellipse_in_ellipse = function(rng, w, h, xo, yo, wo, ho)
  local dist = rng:randomf(0, 1)
  local ang = rng:randomf(0, 2 * math.pi)
  local cxo, cyo = xo + wo / 2, yo + ho / 2
  local x = math.cos(ang) * dist * (wo / 2 - w / 2) + cxo - w / 2
  local y = math.sin(ang) * dist * (ho / 2 - h / 2) + cyo - h / 2
  assert(x >= xo - 0.1)
  assert(y >= yo - 0.1)
  assert(x + w <= (xo + wo) + 0.1)
  assert(y + h <= (yo + ho) + 0.1)
  return x, y, w, h
end
local region_intersects
region_intersects = function(x, y, w, h, R)
  local _list_0 = R.regions
  for _index_0 = 1, #_list_0 do
    local r = _list_0[_index_0]
    if r:ellipse_intersect(x, y, w, h) then
      return true
    end
  end
  return false
end
local random_region_add
random_region_add = function(rng, w, h, n_points, velocity_func, angle, R, bbox, ignore_intersect)
  if ignore_intersect == nil then
    ignore_intersect = false
  end
  for tries = 1, MAX_TRIES do
    local PW, PH
    PW, PH = LEVEL_PADDING[1], LEVEL_PADDING[2]
    local x1, y1, x2, y2
    x1, y1, x2, y2 = bbox[1], bbox[2], bbox[3], bbox[4]
    local x, y = random_ellipse_in_ellipse(rng, w, h, x1, y1, x2 - x1, y2 - y1)
    if ignore_intersect or not region_intersects(x, y, w, h, R) then
      local r = Region.create(x, y, w, h, n_points, angle)
      R:add(r, velocity_func)
      return r
    end
  end
  return nil
end
local Tile = newtype({
  init = function(self, name, solid, seethrough, add, remove)
    if add == nil then
      add = { }
    end
    if remove == nil then
      remove = { }
    end
    self.solid, self.seethrough = solid, seethrough
    self.id = data.get_tilelist_id(name)
    self.add_flags = add
    self.remove_flags = remove
    append(((function()
      if self.solid then
        return self.add_flags
      else
        return self.remove_flags
      end
    end)()), TileMap.FLAG_SOLID)
    return append(((function()
      if self.seethrough then
        return self.add_flags
      else
        return self.remove_flags
      end
    end)()), TileMap.FLAG_SEETHROUGH)
  end
})
local tile_operator
tile_operator = function(tile, data)
  if data == nil then
    data = { }
  end
  assert(not data.content)
  data.content = tile.id
  data.add = data.add or { }
  data.remove = data.remove or { }
  if type(data.add) ~= "table" then
    data.add = {
      data.add
    }
  end
  if type(data.remove) ~= "table" then
    data.remove = {
      data.remove
    }
  end
  local _list_0 = tile.add_flags
  for _index_0 = 1, #_list_0 do
    local flag = _list_0[_index_0]
    append(data.add, flag)
  end
  local _list_1 = tile.remove_flags
  for _index_0 = 1, #_list_1 do
    local flag = _list_1[_index_0]
    append(data.remove, flag)
  end
  return data
end
local spread_region_delta_func
spread_region_delta_func = function(map, rng, outer)
  local center_x, center_y = outer:center()
  return function(self)
    return rng:randomf(-2, 2), rng:randomf(-2, 2)
  end
end
local default_region_delta_func
default_region_delta_func = function(map, rng, outer)
  local center_x, center_y = outer:center()
  local vfunc
  local type = rng:random(0, 2)
  if type == 0 then
    return function(self)
      return math.sign_of(self.x - center_x) * 2, math.sign_of(self.y - center_y) * 2
    end
  elseif type == 1 then
    return function(self)
      return math.sign_of(center_x - self.x) * 2, math.sign_of(center_y - self.y) * 2
    end
  else
    return function(self)
      return 0, 0
    end
  end
end
local ring_region_delta_func
ring_region_delta_func = function(map, rng, outer)
  local angle = rng:randomf(0, 2 * math.pi)
  local rx, ry = outer:center()
  rx, ry = rx - 5, ry - 5
  local ring_n = rng:random(1, 4)
  rx = rx / ring_n
  ry = ry / ring_n
  local to_x, to_y = math.cos(angle) * rx + outer.w / 2, math.sin(angle) * ry + outer.h / 2
  return function(self)
    return math.sign_of(to_x - self.x) * 10, math.sign_of(to_y - self.y) * 10
  end
end
return {
  LEVEL_PADDING = LEVEL_PADDING,
  ellipse_points = ellipse_points,
  Region = Region,
  RVORegionPlacer = RVORegionPlacer,
  subregion_minimum_spanning_tree = subregion_minimum_spanning_tree,
  region_minimum_spanning_tree = region_minimum_spanning_tree,
  spread_region_delta_func = spread_region_delta_func,
  random_rect_in_rect = random_rect_in_rect,
  random_ellipse_in_ellipse = random_ellipse_in_ellipse,
  Tile = Tile,
  tile_operator = tile_operator,
  region_intersects = region_intersects,
  random_region_add = random_region_add,
  default_region_delta_func = default_region_delta_func,
  ring_region_delta_func = ring_region_delta_func
}
