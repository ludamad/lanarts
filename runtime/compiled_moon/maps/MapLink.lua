local World = require("core.World")
local SourceMap = require("core.SourceMap")
local Map = require("core.Map")
local MapUtils = require("maps.MapUtils")
local Display = require("core.Display")
local GameState = require("core.GameState")
local CLOSE_THRESHOLD = 64
local portal_set_label
portal_set_label = function(portal, label, is_down_arrow)
  portal.on_draw = function(self)
    local progress = math.abs(1 - (GameState.frame % 100) / 50)
    local alpha = 0.75
    local _list_0 = Map.rectangle_collision_check(self.map, {
      self.x - CLOSE_THRESHOLD,
      self.y - CLOSE_THRESHOLD,
      self.x + CLOSE_THRESHOLD,
      self.y + CLOSE_THRESHOLD
    }, self)
    for _index_0 = 1, #_list_0 do
      local obj = _list_0[_index_0]
      if obj.is_enemy == false then
        alpha = 0.25
        break
      end
    end
    local color = with_alpha(is_down_arrow and COL_PALE_RED or COL_PALE_GREEN, alpha)
    local arrow_color = with_alpha(COL_WHITE, alpha)
    if Map.object_visible(self) then
      local screen_xy = Display.to_screen_xy({
        self.x,
        self.y - 8 - 8 * progress
      })
      bitmap_draw_wrapped({
        color = color,
        origin = Display.CENTER_BOTTOM
      }, screen_xy, 96, label)
      local sprite = tosprite((function()
        if is_down_arrow then
          return "spr_menu.down"
        else
          return "spr_menu.up"
        end
      end)())
      return sprite:draw({
        color = arrow_color,
        origin = Display.CENTER_BOTTOM
      }, vector_add(screen_xy, {
        0,
        19
      }))
    end
  end
end
local MapLinker = newtype({
  init = function(self, map_desc, generate_hooks)
    if generate_hooks == nil then
      generate_hooks = { }
    end
    self.map_desc, self.generate_hooks = map_desc, generate_hooks
    self.backwards, self.forwards = { }, { }
    self.map = false
    return assert(self:label())
  end,
  get = function(self)
    if not self.map then
      self.map = self.map_desc:generate(self.backwards, self.forwards, self.generate_hooks)
    end
    return self.map
  end,
  label = function(self)
    return self.map_desc.map_label
  end,
  link_portal = function(self, portal, from_sprite)
    local bportal = {
      nil
    }
    portal.on_player_interact = function(portal, user)
      return Map.transfer(user, self:get(), bportal[1].xy)
    end
    portal_set_label(portal, self:label(), true)
    return append(self.backwards, function(map, xy)
      bportal[1] = MapUtils.spawn_portal(map, xy, from_sprite)
      bportal[1].on_player_interact = function(bportal_, user)
        return Map.transfer(user, portal.map, portal.xy)
      end
      return portal_set_label(bportal[1], Map.map_label(portal.map), false)
    end)
  end,
  link_linker = function(self, other_linker, to_sprite, from_sprite)
    local fportal = {
      nil
    }
    local bportal = {
      nil
    }
    append(other_linker.backwards, function(map, xy)
      fportal[1] = MapUtils.spawn_portal(map, xy, from_sprite)
      fportal[1].on_player_interact = function(fportal_, user)
        return Map.transfer(user, self:get(), bportal[1].xy)
      end
      return portal_set_label(fportal[1], self:label(), false)
    end)
    return append(self.forwards, function(map, xy)
      bportal[1] = MapUtils.spawn_portal(map, xy, to_sprite)
      bportal[1].on_player_interact = function(bportal_, user)
        return Map.transfer(user, other_linker:get(), fportal[1].xy)
      end
      return portal_set_label(bportal[1], other_linker:label(), true)
    end)
  end
})
return nilprotect({
  MapLinker = MapLinker
})
