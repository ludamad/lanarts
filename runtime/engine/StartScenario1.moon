import MapRegion, combine_map_regions, map_regions_bbox from require "maps.MapRegion"

Engine = require("engine.EngineBase")


create_dungeon = (rng) ->
    size_scale = 1
    enemy_n_scale = 1

    adjusted_size = (size) ->
        {w, h} = size
        w *= (rng\randomf(-0.1,0.1) + rng\randomf(0.9, 1.1)) * size_scale
        h *= (rng\randomf(-0.1,0.1) + rng\randomf(0.9, 1.1)) * size_scale
        return {random_round(w, rng), random_round(h, rng)}

    scale_node = (node) ->
        if node.size
            node.size = adjusted_size(node.size)
        if node.regions
            for subnode in *node.regions
                scale_node subnode

    scale_node(root_node)
    LAIR = {
        {"Firelord's Kingdom", }
    }
    return 

Engine.first_map_create = () ->
    import Scenario from require "scenarios.Scenario1"
    {:MapCompilerContext} = require "maps.MapCompilerContext"
    cc = MapCompilerContext.create()
    rng = require("mtwist").create(random(0, 2^30))
    cc\register(Scenario.name, Scenario.place_func(rng))
    return cc\get {label: Scenario.name, spawn_players: true}

return require("engine.StartLanarts")
