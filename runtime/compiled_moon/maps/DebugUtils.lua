local Display = require("core.Display")
local SourceMap = require("core.SourceMap")
local Keys = require("core.Keyboard")
local COLORS = {
  COL_BLACK,
  COL_WHITE,
  COL_BABY_BLUE,
  COL_PALE_YELLOW,
  COL_PALE_RED,
  COL_PALE_GREEN,
  COL_PALE_BLUE,
  COL_LIGHT_GRAY
}
local _VIS_W, _VIS_H = nil, nil
local _VIS_DEBUG = false
local debug_visualize_step
debug_visualize_step = function(map, restart, tw, th)
  if restart == nil then
    restart = nil
  end
  if tw == nil then
    tw = 10
  end
  if th == nil then
    th = 10
  end
  local w, h
  do
    local _obj_0 = map.size
    w, h = _obj_0[1], _obj_0[2]
  end
  Display.set_world_region({
    0,
    0,
    w * tw,
    h * th
  })
  local font = font_cached_load("fonts/Gudea-Regular.ttf", 8)
  local step
  step = function()
    require("core.GameState").game_loop(function()
      local row = { }
      local group_row = { }
      for y = 1, h do
        SourceMap.get_row_content(map, row, 1, w, y)
        SourceMap.get_row_groups(map, group_row, 1, w, y)
        for x = 1, w do
          local color = COLORS[(row[x] % #COLORS) + 1]
          if color == COL_BLACK and row[x] ~= 0 then
            color = COL_GOLD
          end
          local bbox = {
            (x - 1) * tw,
            (y - 1) * th,
            x * tw,
            y * th
          }
          Display.draw_rectangle(color, bbox)
          font:draw({
            color = COL_WHITE
          }, {
            bbox[1],
            bbox[2]
          }, tostring(group_row[x]))
        end
      end
      if Keys.key_pressed('N') or Keys.key_pressed('R') then
        return true
      end
      return nil
    end)
    return restart
  end
  return step
end
local debug_show_source_map
debug_show_source_map = function(map, tw, th)
  if tw == nil then
    tw = 8
  end
  if th == nil then
    th = 8
  end
  if not _VIS_DEBUG then
    return
  end
  local w, h
  do
    local _obj_0 = map.size
    w, h = _obj_0[1], _obj_0[2]
  end
  Display.set_world_region({
    0,
    0,
    w * tw,
    h * th
  })
  return require("core.GameState").game_loop(function()
    local row = { }
    for y = 1, h do
      SourceMap.get_row_content(map, row, 1, w, y)
      for x = 1, w do
        local color = COLORS[(row[x] % #COLORS) + 1]
        if color == COL_BLACK and row[x] ~= 0 then
          color = COL_GOLD
        end
        Display.draw_rectangle(color, {
          (x - 1) * tw,
          (y - 1) * th,
          x * tw,
          y * th
        })
      end
    end
    if Keys.key_pressed('N') or Keys.key_pressed('R') then
      return true
    end
    return nil
  end)
end
local is_debug_visualization
is_debug_visualization = function()
  return _VIS_DEBUG
end
local disable_visualization
disable_visualization = function()
  _VIS_DEBUG = false
end
local enable_visualization
enable_visualization = function(w, h)
  _VIS_W, _VIS_H = w, h
  _VIS_DEBUG = true
  return Display.initialize("Demo", {
    _VIS_W,
    _VIS_H
  }, false)
end
local visualization_size
visualization_size = function()
  return _VIS_W, _VIS_H
end
local visualize_map_regions
visualize_map_regions = function(args)
  return require('maps.B2GenerateUtils').visualize_map_regions(args)
end
return {
  debug_show_source_map = debug_show_source_map,
  debug_visualize_step = debug_visualize_step,
  is_debug_visualization = is_debug_visualization,
  disable_visualization = disable_visualization,
  enable_visualization = enable_visualization,
  COLORS = COLORS,
  visualization_size = visualization_size,
  visualize_map_regions = visualize_map_regions
}
