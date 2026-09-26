----
-- Generates the game maps, starting with high-level details (places that will be in the game)
-- and then generating actual tiles.
----

{:Tile, :spread_region_delta_func} = require "maps.GenerateUtils"

NewMaps = require "maps.NewMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"

Vaults = require "maps.Vaults"
{:place_vault_in} = require "maps.VaultUtils"
SourceMap = require "core.SourceMap"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

{:MapDesc, :MapNode} = require "maps.MapDesc"

-- Generation constants and data
{   :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_NO_ENEMY_SPAWN
} = Vaults

{:OVERWORLD_TILESET, :place_doors_and_statues, :overworld_items_and_enemies, :overworld_features} = require "map_descs.OverworldCommon"

return MapDesc.create {
    map_label: "Plain Valley"
    size: {120, 120}
    default_content: Tilesets.grass.wall
    default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
    arc_chance: 0.05
    children: {
        MapNode.create {
            place: () =>
                number_regions = 5 + @rng\random(0, 3) + @rng\random(0, 3) + @rng\random(0, 3)
                connect_line_width = () -> 1 + @rng\random(0, 4) + @rng\random(0, 4) + @rng\random(0, 4) + @rng\random(0, 4)
                room_radius = () -> 5 + @rng\random(0,4) + @rng\random(0, 4) + @rng\random(0, 4)
                factor = math.sqrt(number_regions / 5)
                size = {85 * factor, 85 * factor}
                template = nilprotect {
                    default_wall: Tile.create(Tilesets.grass.wall, true, true, {FLAG_OVERWORLD})
                    subtemplates: {nilprotect {
                        is_overworld: true
                        :size
                        :number_regions
                        floor1: OVERWORLD_TILESET.floor1
                        floor2: OVERWORLD_TILESET.floor2
                        wall1: OVERWORLD_TILESET.wall1
                        wall2: OVERWORLD_TILESET.wall2
                        rect_room_num_range: {0,0} -- disable
                        rect_room_size_range: {10,15}
                        rvo_iterations: 100
                        :connect_line_width
                        :room_radius
                        region_delta_func: spread_region_delta_func
                        -- Dungeon objects/features
                        n_statues: 4
                    }}
                    outer_conf: nilprotect {
                        floor1: OVERWORLD_TILESET.floor1
                        floor2: OVERWORLD_TILESET.floor2
                        :connect_line_width
                    }
                    shell: 25
                }
                -- TODO, handle links to overworld...
                if not NewMaps.map_try_create(@map, @map.rng, template)
                    return false
                if not overworld_features(@region_set, "classic")
                    return false
                append @map.post_maps, () ->
                    if not place_doors_and_statues(@region_set)
                        return false
                    if not overworld_items_and_enemies(@region_set)
                        return false
                    return true
                return true
        }
    }
}
