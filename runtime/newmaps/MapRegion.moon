B2GenerateUtils = require "newmaps.B2GenerateUtils"

-- Define a map region in terms of polygons
-- Should be within (0, 0) to (map.w, map.h)
MapRegion = newtype {
    init: (polygons) =>
        @polygons = assert polygons
    apply: (args) =>
        {:map, :area, :operator} = args
        for polygon in *@polygons
            SourceMap.polygon_apply {
                :map, :area, :operator
                points: polygon
            }
        return nil
}

rng = require("mtwist").create(os.time())
make_polygon = (x, y, w, h, points) ->
    return GenerateUtils.skewed_ellipse_points(rng, {x,y}, {w, h}, points)

sample_shape = () ->
    make_map_regions = (n) ->
        return for i=4,3 + n 
            polygon = make_polygon(i * 10, i * 10, i * math.random() + 3)
            MapRegion.create {polygon}

    return B2GenerateUtils.spread_map_regions {
        :rng
        regions: make_map_regions(4)
        fixed_polygons: {make_polygon(0, 0, 8, 8)}
        n_iterations: 50
        mode: 'towards_fixed_shapes'
        clump_once_near: true
    }

__visualize = () ->
    SourceMap = require "core.SourceMap"
    shape = sample_shape()
    bbox = shape\bbox()
    w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
    -- Make the top y=0 and left x=0
    shape\translate(-bbox[1], -bbox[2])
    assert w > 0 and h > 0, "w, h should be > 0, #{w}, #{h}"
    tw, th = 8,8
    require("core.Display").initialize("Demo", {800, 600} , false)
    require("core.Display").set_world_region({0, 0, w * tw, h * th})
    map = SourceMap.map_create {
        :rng
        label: "Test"
        size: {w, h}
        flags: SourceMap.FLAG_SOLID
    }

    timer = timer_create()
    for i=1,#shape.polygons
        polygon = shape.polygons[i]
        SourceMap.polygon_apply {
            :map
            area: {2,2, w-2, h-2}
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: SourceMap.FLAG_SEETHROUGH
                content: 1
            }
            points: scale_polygon(polygon, 1.1, 1.1)
        }
    print "Time spent rasterizing polygons: ", timer\get_milliseconds()

    require("core.GameState").game_loop () ->
        row = {}
        for y=1,h
            SourceMap.get_row_content(map, row, 1, w, y)
            for x=1,w
                color = COLORS[(row[x] % #COLORS) + 1]
                if color == COL_BLACK and row[x] ~= 0
                    color = COL_GOLD
                require("core.Display").draw_rectangle(color, {(x-1)*tw, (y-1)*th, x*tw, y*th})
        if Keys.key_pressed 'R'
            return true
        return nil

__visualize()
