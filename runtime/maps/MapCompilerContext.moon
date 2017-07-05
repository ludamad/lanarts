-- Context for compilations, essentially act to organize and configure map compilation    

Map = require "core.Map"
World = require "core.World"

-- Represents a pending feature 
PendingFeature = newtype {
    init: (@label, @placement_function, @max_tries = 100) => 
    _place: (args) =>
        compiler = args.compiler
        for i=1,@max_tries
            compiled_feature = @placement_function(compiler)
            if compiled_feature ~= nil 
                @feature = compiled_feature
                return true
        return false
    compile: (args) =>
        return @_place(args)
}

make_on_player_interact = (cc, target_portal) -> (_, user) ->
    -- We call C\get(label) first, to potentially trigger map
    -- generation 
    -- This will ensure .feature is set on @other_portal
    destination_map = cc\get(target_portal.label)
    assert target_portal.feature, "Connected portal should have compiled!"
    -- Transfer the user object between floors.
    Map.transfer(user, destination_map, target_portal.feature.xy)
PendingPortal = newtype {
    parent: PendingFeature
    connect: (@other_portal) => nil
    compile: (args) =>
        if not PendingFeature.compile(@, args)
            return false
        -- Rely on PendingFeature.compile() setting @feature:
        assert @feature, "Portal should have compiled"
        @feature.on_player_interact = make_on_player_interact(args.context, @other_portal)
        return true
}

MapCompilerContext = newtype {
    init: () =>
        -- Maps not yet generated
        -- Associates map labels with their generator (Lua) class
        @pending_maps = {}
        @maps = {}
        -- Features pushed onto a map of a certain label 
        @pending_features = {}
    add_pending_feature: (label, feature) =>
        append @pending_features[label], feature
        return feature
    add_pending_portal: (label, placement_function) =>
        feature = PendingPortal.create(label, placement_function)
        return @add_pending_feature(label, feature)
    -- Register map compilers with unique labels
    register: (label, C) =>
        assert (not @pending_maps[label]), "#{label} already registered!"
        assert C
        @pending_maps[label] = C
        @pending_features[label] = {}
    -- Gets a core.Map game map object for a label.
    -- Generation arguments are provided NOTE they only affect the first call to this function.
    -- Subsequent calls will pass so long as they have 'label', but it is good form to always
    -- call with generation arguments.
    get: (args) => -- Generates if pending
        label = assert args.label, "Must provide label to identify map to retrieve/generate!"
        -- other args:
        -- 'spawn_players' whether to spawn players in this room
        -- 
        if not @pending_maps[label]
            return assert @maps[label], "Map '#{label}' not registered!"
        log_info "Generating map '#{label}'!"
        C = @pending_maps[label]
        if type(C) == 'function' -- Simple callback
            args.label = label
            args.pending_feature = @pending_feature[label]
            @maps[label] = C(@, args)
        else -- New-style area class
            compiler = C.create {
                label: label
                rng: require('mtwist').create(random(0,2^30))
            }
            args.context = @
            args.compiler = compiler
            compiler\generate(args)
            for pending_feature in *@pending_features[label]
                pending_feature\compile(@, compiler)
            @maps[label] = compiler\compile(args)
            if args.spawn_players 
                World.players_spawn @maps[label], compiler\get_player_spawn_points()
                if os.getenv "LANARTS_XP"
                    for p in *World.players
                        p.instance\gain_xp(tonumber(os.getenv "LANARTS_XP"))
        @pending_maps[label] = nil
        @pending_features[label] = nil
        return @maps[label]
}

return {:MapCompilerContext, :make_on_player_interact}
