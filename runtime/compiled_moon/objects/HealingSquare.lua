local GameObject = require("core.GameObject")
local GameState = require("core.GameState")
local Map = require("core.Map")
local EventLog = require("ui.EventLog")
local Display = require("core.Display")
local ObjectUtils = require("objects.ObjectUtils")
local LuaGameObject = require("objects.LuaGameObject")
local GlobalData = require("core.GlobalData")
local DungeonFeatures = require("objects.DungeonFeatures")
local M = nilprotect({ })
M.healingsqr_sprite = tosprite("spr_doors.healingsqr")
M.HealingSquare = LuaGameObject.type_create({
  base = DungeonFeatures.FeatureBase
})
do
  local _with_0 = M.HealingSquare
  _with_0.parent_on_step = function(self) end
  _with_0.on_step = function(self)
    local collisions = Map.rectangle_collision_check(self.map, self.area, self)
    self.stepped_on = false
    for _index_0 = 1, #collisions do
      local _continue_0 = false
      repeat
        local col = collisions[_index_0]
        if col.is_enemy == false then
          local mp, max_mp, hp, max_hp, hpregen, mpregen
          do
            local _obj_0 = col:effective_stats()
            mp, max_mp, hp, max_hp, hpregen, mpregen = _obj_0.mp, _obj_0.max_mp, _obj_0.hp, _obj_0.max_hp, _obj_0.hpregen, _obj_0.mpregen
          end
          if hp >= max_hp and mp >= max_mp then
            _continue_0 = true
            break
          end
          self.stepped_on = true
          if col.is_resting then
            col:heal_hp(hpregen * 8)
            col:heal_mp(mpregen * 8)
            for _ in screens() do
              EventLog.add("You rest twice as quickly!", COL_PALE_BLUE)
            end
          else
            col:heal_hp(hpregen)
            col:heal_mp(mpregen)
            for _ in screens() do
              EventLog.add("You regain health and mana twice as quickly!", COL_PALE_BLUE)
            end
          end
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    if not self.seen and Map.object_visible(self) then
      self.seen = true
    end
  end
  _with_0.bbox = function(self)
    local x, y
    do
      local _obj_0 = Display.screen_coords(self.xy)
      x, y = _obj_0[1], _obj_0[2]
    end
    return {
      x - 16,
      y - 16,
      x + 16,
      y + 16
    }
  end
  _with_0.on_draw = function(self)
    if Display.object_within_view(self) then
      if self.seen or Map.object_visible(self) then
        Display.draw_rectangle(with_alpha(COL_PALE_BLUE, ((function()
          if self.stepped_on then
            return 0.4
          else
            return 0.05
          end
        end)())), self:bbox())
        return ObjectUtils.screen_draw(self.sprite, self.xy, ((function()
          if self.stepped_on then
            return 1
          else
            return 0.5
          end
        end)()), 0.0)
      end
    end
  end
  _with_0.on_map_init = function(self)
    local tile_xy = ObjectUtils.tile_xy(self, true)
    Map.tile_set_seethrough(self.map, tile_xy, true)
    self.area = {
      self.x - 16,
      self.y - 16,
      self.x + 16,
      self.y + 16
    }
  end
  _with_0.init = function(self, args)
    M.HealingSquare.parent_init(self, args)
    self.depth = DungeonFeatures.FEATURE_DEPTH
    self.stepped_on = false
    self.seen = false
    self.sprite = M.healingsqr_sprite
  end
end
return M
