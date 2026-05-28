local b2 = require('b2')
local GenerateUtils = require("maps.GenerateUtils")
local Keys = require("core.Keyboard")
local DebugUtils = require("maps.DebugUtils")
local B2Utils = require("maps.B2Utils")
local B2World = newtype({
  init = function(self)
    self.world = b2.World(b2.Vec2(0.0, 0.0))
    self.dynamic_bodies = { }
    self.static_bodies = { }
    self.all_bodies = { }
  end,
  add_body = function(self, shape, density)
    if density == nil then
      density = 1.0
    end
    local body = B2Utils.create_body(self.world, shape, density)
    if density == 0.0 then
      append(self.static_bodies, body)
    else
      append(self.dynamic_bodies, body)
    end
    append(self.all_bodies, body)
    return body
  end,
  unique_within = function(self, o1, o2, p1, p2)
    local _list_0 = self.all_bodies
    for _index_0 = 1, #_list_0 do
      local _continue_0 = false
      repeat
        local o = _list_0[_index_0]
        if o == o1 or o == o2 then
          _continue_0 = true
          break
        end
        if B2Utils.ray_cast(o.fixtures, p1, p2) then
          return false
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return true
  end,
  remove_body = function(self, body)
    return self.world:DestroyBody(body)
  end,
  has_overlaps = function(self)
    local _list_0 = self.all_bodies
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local body
      body = _des_0.body
      if body:OverlapsOtherBody() then
        return true
      end
    end
    return false
  end,
  visualize = function(self, title)
    if not DebugUtils.is_debug_visualization() then
      return
    end
    self.world:SetDebugDraw((function()
      do
        local _with_0 = b2.GLDrawer()
        _with_0:SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
        return _with_0
      end
    end)())
    local w, h = DebugUtils.visualization_size()
    require("core.Display").set_world_region({
      -w / 2,
      -h / 2,
      w / 2,
      h / 2
    })
    return require("core.GameState").game_loop(function()
      local font = font_cached_load("fonts/Gudea-Regular.ttf", 14)
      self.world:DrawDebugData()
      if Keys.key_pressed("N") then
        return true
      end
      require("core.Display").reset_blend_func()
      font:draw({
        color = COL_WHITE,
        origin = require("core.Display").CENTER
      }, {
        100,
        100
      }, title)
      return false
    end)
  end
})
return nilprotect({
  B2World = B2World
})
