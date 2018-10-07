Display = require "core.Display"
SourceMap = require "core.SourceMap"
Keys = require "core.Keyboard"

COLORS = {
    COL_BLACK
    COL_WHITE
    COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
    COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY
}

_VIS_W, _VIS_H = nil, nil
_VIS_DEBUG = false

debug_visualize_step = (map, restart=nil, tw=10, th=10) ->
    {w, h} = map.size
    Display.set_world_region({0, 0, w * tw, h * th})
    font = font_cached_load "fonts/Gudea-Regular.ttf", 8

    step = () ->
        require("core.GameState").game_loop () ->
            row = {}
            group_row = {}
            for y=1,h
                SourceMap.get_row_content(map, row, 1, w, y)
                SourceMap.get_row_groups(map, group_row, 1, w, y)
                for x=1,w
                    color = COLORS[(row[x] % #COLORS) + 1]
                    if color == COL_BLACK and row[x] ~= 0
                        color = COL_GOLD
                    bbox = {(x-1)*tw, (y-1)*th, x*tw, y*th}
                    Display.draw_rectangle(color, bbox)
                    font\draw({color: COL_WHITE}, {bbox[1], bbox[2]}, tostring(group_row[x]))
            if Keys.key_pressed('N') or Keys.key_pressed('R')
                return true
            return nil
        return restart
    return step

debug_show_source_map = (map, tw=8, th=8) ->
    if not _VIS_DEBUG
        return
    {w, h} = map.size
    Display.set_world_region({0, 0, w * tw, h * th})
    require("core.GameState").game_loop () ->
        row = {}
        for y=1,h
            SourceMap.get_row_content(map, row, 1, w, y)
            for x=1,w
                color = COLORS[(row[x] % #COLORS) + 1]
                if color == COL_BLACK and row[x] ~= 0
                    color = COL_GOLD
                Display.draw_rectangle(color, {(x-1)*tw, (y-1)*th, x*tw, y*th})
        if Keys.key_pressed('N') or Keys.key_pressed('R')
            return true
        return nil

is_debug_visualization = () ->
    return _VIS_DEBUG

disable_visualization = () ->
    _VIS_DEBUG = false

enable_visualization = (w,  h) ->
    _VIS_W, _VIS_H = w, h
    _VIS_DEBUG = true
    Display.initialize("Demo", {_VIS_W, _VIS_H} , false)

visualization_size = () ->
    return _VIS_W, _VIS_H

visualize_map_regions = (args) ->
    require('maps.B2GenerateUtils').visualize_map_regions(args)

return {:debug_show_source_map, :debug_visualize_step, :is_debug_visualization, :disable_visualization, :enable_visualization, :COLORS, :visualization_size, :visualize_map_regions}
