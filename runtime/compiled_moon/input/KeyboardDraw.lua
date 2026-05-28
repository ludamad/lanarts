local Display = require('core.Display')
local spell_keys = {
  'y',
  'u',
  'i',
  'o',
  'p'
}
return {
  _draw_button = function(self, button, xy)
    xy = vector_add(xy, {
      16,
      16
    })
    return Fonts.large:draw({
      color = {
        255,
        255,
        50,
        100
      },
      origin = Display.CENTER
    }, xy, button:upper())
  end,
  draw_item_ui_hint = function(self, xy, slot) end,
  draw_spell_ui_hint = function(self, xy, slot)
    for s, b in ipairs(spell_keys) do
      if slot == s then
        self:_draw_button(b, xy)
      end
    end
    return nil
  end,
  draw_action_bar_weapon_hint = function(self, xy)
    local above_xy = {
      xy[1],
      xy[2] - 32
    }
    return self:_draw_button('g', above_xy)
  end,
  draw_action_bar_spell_hint = function(self, xy, slot)
    for s, b in ipairs(spell_keys) do
      if slot == s - 1 then
        local above_xy = {
          xy[1],
          xy[2] - 32
        }
        self:_draw_button(b, above_xy)
      end
    end
    return nil
  end
}
