local MapCompilerContext
MapCompilerContext = require("maps.MapCompilerContext").MapCompilerContext
local MapUtils = require("maps.MapUtils")
local Places = require("maps.Places")
local first_map_create
first_map_create = function()
  local cc = MapCompilerContext.create()
  local LAIR = {
    {
      "Dragon's Lair",
      Places.ChickenCoop
    }
  }
  for _index_0 = 1, #LAIR do
    local _des_0 = LAIR[_index_0]
    local label, C
    label, C = _des_0[1], _des_0[2]
    cc:register(label, C)
  end
  return cc:get({
    label = LAIR[1][1],
    spawn_players = true
  })
end
return {
  first_map_create = first_map_create
}
