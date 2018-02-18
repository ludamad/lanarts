import MapRegion, combine_map_regions, map_regions_bbox from require "maps.MapRegion"

Engine = require("engine.EngineBase")

Engine.first_map_create = () ->
    import Scenario from require "scenarios.Scenario1"
    {:MapCompilerContext} = require "maps.MapCompilerContext"
    cc = MapCompilerContext.create()
    cc\register(Scenario.name, Scenario.place)
    return cc\get {label: Scenario.name, spawn_players: true}

return require("engine.StartLanarts")
