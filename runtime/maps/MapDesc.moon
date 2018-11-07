MapUtils = require "maps.MapUtils"
NewMaps = require "maps.NewMaps"
SourceMap = require "core.SourceMap"
Tilesets = require "tiles.Tilesets"
{:MapRegion, :combine_map_regions, :from_bbox} = require "maps.MapRegion"

PADDING = 10
MAX_MAP_TRIES = 100

MapDesc = newtype {
    init: (@map_args) =>
        @parent = false
        @children = @map_args.children
        @map_args.children = nil
        @region_set = false
    linker: () =>
        return require("maps.MapLink").MapLinker.create(@)
    generate: (back_links={}, forward_links={}) =>
        map = NewMaps.try_n_times MAX_MAP_TRIES, () -> @compile(back_links, forward_links)
        game_map = NewMaps.generate_game_map(map)
        for post_poned in *map.post_game_map
            post_poned(game_map)
        return game_map
    compile: (@back_links={}, @forward_links={}) =>
        map_args = table.merge {
            rng: NewMaps.new_rng()
            size: {100, 100}
            default_content: Tilesets.orc.wall
            default_flags: {SourceMap.FLAG_SOLID} --, SourceMap.FLAG_SEETHROUGH}
            map_label: "Dungeon"
        }, @map_args
        map_args.size = vector_add(map_args.size, {PADDING*2, PADDING*2})
        map = NewMaps.source_map_create map_args

        room_selector = {
            matches_all: SourceMap.FLAG_SOLID
            matches_none: SourceMap.FLAG_PERIMETER
        }
        @region_set = {
            :map
            regions: {
                from_bbox(
                    PADDING
                    PADDING
                    map.size[1] - PADDING
                    map.size[2] - PADDING)\with_selector(room_selector)
            }
        }
        for child in *@children
            child.parent = @
            child.desc = @
            if not child\compile()
                return nil
        if not NewMaps.check_connection(map)
            event_log("Failed connectivity check")
            return nil
        return map
}

MapNode = newtype {
    init: (args) =>
        @children = args.children or {}
        @place = args.place
    new_group: () =>
        @map.next_group += 1
        return @map.next_group
    compile: () =>
        @region_set = @parent.region_set
        @group = @new_group()
        if not @place()
            return false
        for child in *@children
            child.parent = @
            child.desc = @desc
            if not child\compile()
                return false
        return true
    chance: (prob) => @region_set.map.rng\chance(prob)
    apply: (args) =>
        args.map = @map
        for region in *@region_set.regions
            region\apply(args)
    get: {
        map: () => @region_set.map
        rng: () => @region_set.map.rng
    }
}

return {:MapNode, :MapDesc}
