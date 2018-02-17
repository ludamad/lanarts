Engine = require("engine.EngineBase")

Engine.first_map_create = () ->
    {:MapCompilerContext} = require "maps.MapCompilerContext"
    MapUtils = require "maps.MapUtils"
    Places = require "maps.Places"
    cc = MapCompilerContext.create()
    cc\register("Player Pit", Places.Arena)
    return cc\get {label: "Player Pit", spawn_players: true}

return require("engine.StartLanarts")
