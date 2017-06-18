{:MapCompilerContext} = require "maps.MapCompilerContext"
Places = require "maps.Places"

first_map_create = () ->
    cc = MapCompilerContext.create()
    cc\register("Dragon's Lair", Places.DragonLair)
    -- Compile the first map
    return cc\get("Dragon's Lair", true) -- 'true' to spawn players

return {:first_map_create}
