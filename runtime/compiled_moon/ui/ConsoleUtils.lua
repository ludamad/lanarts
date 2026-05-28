local Map = require("core.Map")
local Display = require("core.Display")
local draw_console_text
draw_console_text = function(xy, texts)
  local x, y
  x, y = xy[1], xy[2]
  for _index_0 = 1, #texts do
    local _des_0 = texts[_index_0]
    local color, text
    color, text = _des_0[1], _des_0[2]
    x = x + font_cached_load(settings.font, 10):draw({
      color = color,
      origin = Display.LEFT_CENTER
    }, {
      x,
      y
    }, text)
  end
  return nil
end
local draw_console_effect
draw_console_effect = function(xy, sprite, texts)
  local x, y
  x, y = xy[1], xy[2]
  sprite:draw({
    origin = Display.LEFT_CENTER
  }, {
    x,
    y + 4
  })
  return draw_console_text({
    x + Map.TILE_SIZE + 4,
    y
  }, texts)
end
local draw_weapon_console_effect
draw_weapon_console_effect = function(player, sprite, text, xy, color)
  if color == nil then
    color = COL_PALE_YELLOW
  end
  player.weapon_sprite:draw({
    origin = Display.LEFT_CENTER
  }, {
    xy[1],
    xy[2] + 4
  })
  return draw_console_effect(sprite, text, xy, color)
end
return {
  draw_console_text = draw_console_text,
  draw_console_effect = draw_console_effect,
  draw_weapon_console_effect = draw_weapon_console_effect
}
