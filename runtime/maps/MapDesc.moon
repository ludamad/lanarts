NewMaps = require "maps.NewMaps"
SourceMap = require "core.SourceMap"
Tilesets = require "tiles.Tilesets"
{:from_bbox} = require "maps.MapRegion"

DEFAULT_PADDING = 10
MAX_MAP_TRIES = 100

MapDesc = newtype {
    init: (@map_args) =>
        @parent = false
        @children = @map_args.children
        for child in *@children
            if child.tag == ""
                child.tag = "root"
        @map_label = @map_args.map_label or "Dungeon"
        @padding = @map_args.padding or DEFAULT_PADDING
        @map_args.children = nil
        @map_args.padding = nil
        @map_args.map_label = nil
        @region_set = false
    linker: (generate_hooks={}) =>
        return require("maps.MapLink").MapLinker.create(@, generate_hooks)
    generate: (back_links={}, forward_links={}, generate_hooks={}) =>
        map = @compile(back_links, forward_links, generate_hooks)
        game_map = NewMaps.generate_game_map(map)
        for post_poned in *map.post_game_map
            post_poned(game_map)
        return game_map

    compile: (back_links={}, forward_links={}, generate_hooks={}) =>
        return NewMaps.try_n_times MAX_MAP_TRIES, () ->
            return @_compile(back_links, forward_links, generate_hooks)
    _compile: (@back_links={}, @forward_links={}, @generate_hooks={}) =>
        map_args = table.merge {
            rng: NewMaps.new_rng()
            size: {100, 100}
            default_content: Tilesets.orc.wall
            default_flags: {SourceMap.FLAG_SOLID} --, SourceMap.FLAG_SEETHROUGH}
            map_label: @map_label
            post_map: {}
        }, @map_args
        map_args.size = vector_add(map_args.size, {@padding*2, @padding*2})
        map = NewMaps.source_map_create map_args
        map.post_map = {}

        room_selector = {
            matches_all: SourceMap.FLAG_SOLID
            matches_none: SourceMap.FLAG_PERIMETER
        }
        @region_set = {
            :map
            regions: {
                from_bbox(@padding, @padding, map.size[1] - @padding, map.size[2] - @padding)\with_selector(room_selector)
            }
        }
        for child in *@children
            child.parent = @
            child.desc = @
            if not child\compile()
                return nil
        for post_map in *map.post_map
            if not post_map(map)
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
        @tag = args.tag or ""
    new_group: () =>
        @map.next_group += 1
        return @map.next_group
    compile: () =>
        @region_set = @parent.region_set
        @group = @new_group()
        if not @place()
            return false
        hook = @desc.generate_hooks[@tag]
        if hook and not hook(@)
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

return nilprotect {:MapNode, :MapDesc}
