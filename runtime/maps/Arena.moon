----
-- Generates the game maps, starting with high-level details (places that will be in the game)
-- and then generating actual tiles.
----

import Vault
    from require "maps.MapElements"

-- Guides level generation.
LevelGenContext = newtype {
    init: () =>
    item: () =>

}

make_polygon_points = (rng, w, h, n_points) ->
    return GenerateUtils.skewed_ellipse_points(rng, {0,0}, {30, 30}, rng\random(4,12))

make_arena_template = (C, rng) -> 
    -- Create topology
    area = Area {
        name: 'root'
        subareas: for i=1,3
            Polygon {
                name: 'main'
                points: make_polygon_points(rng, 10, 10, 4)
            }
        connection_scheme: 'direct'
    }

    -- TODO figure out everything possible with this mess of opportunity

    -- Create level filler
    on_generate = () ->
        area["$root.main"]

    return area, on_generate

return M
