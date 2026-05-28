local SourceMap = require("core.SourceMap")
local Vaults = require("maps.Vaults")
local FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_TEMPORARY, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN
FLAG_ALTERNATE, FLAG_INNER_PERIMETER, FLAG_DOOR_CANDIDATE, FLAG_TEMPORARY, FLAG_OVERWORLD, FLAG_ROOM, FLAG_NO_ENEMY_SPAWN, FLAG_NO_ITEM_SPAWN = Vaults.FLAG_ALTERNATE, Vaults.FLAG_INNER_PERIMETER, Vaults.FLAG_DOOR_CANDIDATE, Vaults.FLAG_TEMPORARY, Vaults.FLAG_OVERWORLD, Vaults.FLAG_ROOM, Vaults.FLAG_NO_ENEMY_SPAWN, Vaults.FLAG_NO_ITEM_SPAWN
local get_door_candidates
get_door_candidates = function(compiler, node)
  local area = compiler:as_bbox(node, 2)
  local group_set = compiler:as_group_set(node)
  SourceMap.perimeter_apply({
    map = compiler.map,
    area = area,
    candidate_selector = {
      matches_all = {
        SourceMap.FLAG_TUNNEL
      },
      matches_none = {
        FLAG_DOOR_CANDIDATE,
        SourceMap.FLAG_SOLID
      }
    },
    inner_selector = {
      matches_group = group_set,
      matches_none = {
        SourceMap.FLAG_TUNNEL,
        SourceMap.FLAG_SOLID
      }
    },
    operator = {
      add = FLAG_TEMPORARY
    }
  })
  SourceMap.perimeter_apply({
    map = compiler.map,
    area = area,
    candidate_selector = {
      matches_all = FLAG_TEMPORARY
    },
    inner_selector = {
      matches_all = {
        SourceMap.FLAG_TUNNEL
      },
      matches_none = {
        FLAG_TEMPORARY,
        SourceMap.FLAG_SOLID
      }
    },
    operator = {
      add = FLAG_DOOR_CANDIDATE
    }
  })
  SourceMap.rectangle_apply({
    map = compiler.map,
    area = area,
    operator = {
      remove = FLAG_TEMPORARY
    },
    create_subgroup = false
  })
  return SourceMap.rectangle_match({
    map = compiler.map,
    area = area,
    selector = {
      matches_all = FLAG_DOOR_CANDIDATE
    }
  })
end
return {
  get_door_candidates = get_door_candidates
}
