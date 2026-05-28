local GameObject = require("core.GameObject")
local Display = require("core.Display")
local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local ANIMATE_VELOCITIES = {
  {
    -1,
    -1
  },
  {
    0,
    -1
  },
  {
    1,
    -1
  },
  {
    1,
    0
  },
  {
    1,
    1
  },
  {
    0,
    1
  },
  {
    -1,
    1
  },
  {
    -1,
    0
  }
}
local COLOR_CLASSES = {
  Black = {
    "Necromancer"
  },
  White = {
    "Stormcaller"
  },
  Red = {
    "Pyrocaster"
  },
  Green = {
    "Fighter"
  },
  Blue = { }
}
local create_animation
create_animation = function(self, obj, sprite, speed)
  if speed == nil then
    speed = 4
  end
  GameObject.animation_create({
    map = obj.map,
    xy = obj.xy,
    sprite = sprite,
    duration = 25,
    velocity = vector_scale(ANIMATE_VELOCITIES[(self.n_animations % #ANIMATE_VELOCITIES) + 1], speed)
  })
  self.n_animations = self.n_animations + (function()
    if chance(0.1) then
      return 2
    else
      return 1
    end
  end)()
end
local of_color
of_color = function(obj, color)
  local _list_0 = COLOR_CLASSES[color]
  for _index_0 = 1, #_list_0 do
    local klass = _list_0[_index_0]
    if obj:has_effect(klass) then
      return true
    end
  end
  return false
end
local of_colors
of_colors = function(obj, colors)
  for _index_0 = 1, #colors do
    local color = colors[_index_0]
    if of_color(obj, color) then
      return true
    end
  end
  return false
end
local _draw_color_bound_effect
_draw_color_bound_effect = function(self, obj, xy, colors, description)
  local x, y
  x, y = xy[1], xy[2]
  if of_colors(obj, colors) then
    draw_console_effect({
      x,
      y - 8
    }, self.sprite, {
      {
        COL_GREEN,
        tostring(table.concat(colors, " or ")) .. " class bonus"
      }
    })
    return draw_console_text({
      x + 36,
      y + 8
    }, {
      {
        COL_WHITE,
        description
      }
    })
  else
    draw_console_effect({
      x,
      y - 8
    }, self.sprite, {
      {
        COL_RED,
        tostring(table.concat(colors, " or ")) .. " class bonus"
      }
    })
    return draw_console_text({
      x + 36,
      y + 8
    }, {
      {
        COL_GRAY,
        description
      }
    })
  end
end
local draw_color_bound_effect
draw_color_bound_effect = function(self, obj, get_next, colors, description)
  local xy = get_next()
  return _draw_color_bound_effect(self, obj, xy, colors, description)
end
local draw_color_bound_weapon_effect
draw_color_bound_weapon_effect = function(self, obj, get_next, colors, description)
  local xy = get_next()
  obj.weapon_sprite:draw({
    origin = Display.LEFT_CENTER
  }, {
    xy[1],
    xy[2] + 4
  })
  return _draw_color_bound_effect(self, obj, xy, colors, description)
end
local draw_simple_effect
draw_simple_effect = function(self, xy, name, description)
  local x, y
  x, y = xy[1], xy[2]
  draw_console_effect({
    x,
    y - 8
  }, self.sprite, {
    {
      COL_GOLD,
      name
    }
  })
  return draw_console_text({
    x + 36,
    y + 8
  }, {
    {
      COL_WHITE,
      description
    }
  })
end
return {
  create_animation = create_animation,
  of_color = of_color,
  of_colors = of_colors,
  draw_color_bound_effect = draw_color_bound_effect,
  draw_color_bound_weapon_effect = draw_color_bound_weapon_effect,
  draw_simple_effect = draw_simple_effect
}
