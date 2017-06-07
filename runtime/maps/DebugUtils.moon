Display = require "core.Display"

COLORS = {
    COL_BLACK
    COL_WHITE
    COL_BABY_BLUE, COL_PALE_YELLOW, COL_PALE_RED,
    COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY 
}

VIS_W, VIS_H = nil, nil
_VIS_DEBUG = false
debug_show_source_map = (map, tw=8, th=8) ->
    if not _VIS_DEBUG
        return
    {w, h} = map.size
    Display.set_world_region({0, 0, w * tw, h * th})
    row = {}
    for y=1,h
        SourceMap.get_row_content(map, row, 1, w, y)
        for x=1,w
            color = COLORS[(row[x] % #COLORS) + 1]
            if color == COL_BLACK and row[x] ~= 0
                color = COL_GOLD
            Display.draw_rectangle(color, {(x-1)*tw, (y-1)*th, x*tw, y*th})

is_debug_visualization = () -> 
    return _VIS_DEBUG

disable_visualization = () ->
    _VIS_DEBUG = false

enable_visualization = (w,  h) ->
    _VIS_W, _VIS_H = w, h
    _VIS_DEBUG = true
    Display.initialize("Demo", {_VIS_W, _VIS_H} , false)

return {:debug_show_source_map, :is_debug_visualization, :disable_visualization, :enable_visualization, :COLORS}
