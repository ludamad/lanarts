{:MapCompilerContext} = require "maps.MapCompilerContext"
MapUtils = require "maps.MapUtils"
Places = require "maps.Places"

first_map_create = () ->
    cc = MapCompilerContext.create()

    LAIR = {
        --{"Dragon's Cave", Places.DragonLairFoyer}
        {"Dragon's Lair", Places.DragonLair}
    }

    for {label, C} in *LAIR
        cc\register(label, C)

    --for i=1,3
    --    p1 = cc\add_pending_portal LAIR[1][1], (feature, compiler) ->
    --        MapUtils.random_portal(compiler.map, nil, "spr_gates.volcano_portal")
    --    p2 = cc\add_pending_portal LAIR[2][1], (feature, compiler) ->
    --        MapUtils.random_portal(compiler.map, nil, "spr_gates.volcano_exit")
    --    p1\connect(p2)
    --    p2\connect(p1)

    -- Compile the first map
    return cc\get(LAIR[1][1], true) -- 'true' to spawn players

return {:first_map_create}
