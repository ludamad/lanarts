Engine = require("engine.EngineBase")

Engine.first_map_create = () ->
    {:MapCompilerContext} = require "maps.MapCompilerContext"
    Scenario1 = require("scenarios.Scenario1")
    MapUtils = require "maps.MapUtils"
    -- Places = require "maps.Places"
    cc = MapCompilerContext.create()
    rng = require("mtwist").create(random(0, 2^30))
    cc\register("Player Pit", Scenario1.create_scenario(rng))
    return cc\get {label: "Player Pit", spawn_players: true}

return require("engine.StartLanarts")
