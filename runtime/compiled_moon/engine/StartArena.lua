local Engine = require("engine.EngineBase")
Engine.first_map_create = function()
  local MapCompilerContext
  MapCompilerContext = require("maps.MapCompilerContext").MapCompilerContext
  local Scenario1 = require("scenarios.Scenario1")
  local MapUtils = require("maps.MapUtils")
  local cc = MapCompilerContext.create()
  local rng = require("mtwist").create(random(0, 2 ^ 30))
  cc:register("Player Pit", Scenario1.create_scenario(rng))
  return cc:get({
    label = "Player Pit",
    spawn_players = true
  })
end
return require("engine.StartLanarts")
