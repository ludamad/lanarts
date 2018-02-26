SourceMap = require "core.SourceMap"

Vaults = require "maps.Vaults"
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE, :FLAG_TEMPORARY
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

get_door_candidates = (compiler, node) ->
    -- Include a bit around the area to be sure we get all door candidates
    area = compiler\as_bbox(node, 2) -- padding
    group_set = compiler\as_group_set(node)
    -- Install FLAG_TEMPORARY for door candidates
    SourceMap.perimeter_apply {
        map: compiler.map, :area
        candidate_selector: {
            matches_all: {SourceMap.FLAG_TUNNEL}
            matches_none: {FLAG_DOOR_CANDIDATE, SourceMap.FLAG_SOLID}
        }
        inner_selector: {
            matches_group: group_set
            matches_none: {SourceMap.FLAG_TUNNEL, SourceMap.FLAG_SOLID}
        }
        operator: {add: FLAG_TEMPORARY}
    }
    -- Only turn squares with FLAG_TEMPORARY that are around a tunnel, into doors
    SourceMap.perimeter_apply {
        map: compiler.map, :area
        candidate_selector: {
            matches_all: FLAG_TEMPORARY
        }
        inner_selector: {
            matches_all: {SourceMap.FLAG_TUNNEL}
            matches_none: {FLAG_TEMPORARY, SourceMap.FLAG_SOLID}
        }
        operator: {add: FLAG_DOOR_CANDIDATE}
    }
    -- Cleanup
    SourceMap.rectangle_apply {
        map: compiler.map, :area
        operator: {remove: FLAG_TEMPORARY}
        create_subgroup: false
    }
    return SourceMap.rectangle_match {
        map: compiler.map, :area
        selector: {matches_all: FLAG_DOOR_CANDIDATE}
    }

return {:get_door_candidates}
