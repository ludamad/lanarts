local MapRegion, combine_map_regions, map_regions_bbox
do
  local _obj_0 = require("maps.MapRegion")
  MapRegion, combine_map_regions, map_regions_bbox = _obj_0.MapRegion, _obj_0.combine_map_regions, _obj_0.map_regions_bbox
end
local Engine = require("engine.EngineBase")
local create_dungeon
create_dungeon = function(rng)
  local size_scale = 1
  local enemy_n_scale = 1
  local adjusted_size
  adjusted_size = function(size)
    local w, h
    w, h = size[1], size[2]
    w = w * ((rng:randomf(-0.1, 0.1) + rng:randomf(0.9, 1.1)) * size_scale)
    h = h * ((rng:randomf(-0.1, 0.1) + rng:randomf(0.9, 1.1)) * size_scale)
    return {
      random_round(w, rng),
      random_round(h, rng)
    }
  end
  local scale_node
  scale_node = function(node)
    if node.size then
      node.size = adjusted_size(node.size)
    end
    if node.regions then
      local _list_0 = node.regions
      for _index_0 = 1, #_list_0 do
        local subnode = _list_0[_index_0]
        scale_node(subnode)
      end
    end
  end
  scale_node(root_node)
  local LAIR = {
    {
      "Firelord's Kingdom"
    }
  }
end
Engine.first_map_create = function()
  local create_scenario
  create_scenario = require("scenarios.Scenario1").create_scenario
  local MapCompilerContext
  MapCompilerContext = require("maps.MapCompilerContext").MapCompilerContext
  local cc = MapCompilerContext.create()
  local rng = require("mtwist").create(random(0, 2 ^ 30))
  cc:register("Firelord's Kingdom", create_scenario(rng))
  return cc:get({
    label = "Firelord's Kingdom",
    spawn_players = true
  })
end
return require("engine.StartLanarts")
