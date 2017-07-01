GameState = require "core.GameState"
GlobalData = require "core.GlobalData"
World = require "core.World"
GameObject = require "core.GameObject"
PathFinding = require "core.PathFinding"
Map = require "core.Map"
Keyboard = require "core.Keyboard"

HARDCODED_AI_SEED = 12345657
ASTAR_BUFFER = PathFinding.astar_buffer_create()
_sim = (k) ->
    GameState._simulate_key_press(Keyboard[k])

_collisions = (player, type = nil) ->
    collisions = Map.rectangle_collision_check(player.map, {player.x - 8, player.y - 8, player.x+8, player.y+8}, player)
    if not type
        return collisions
    return table.filter collisions, () => GameObject.get_type(@) == type

_objects = (player, type = nil) ->
    objects = Map.objects_list(player.map)
    if not type
        return objects
    return table.filter objects, () => GameObject.get_type(@) == type

portal_planner = (player) -> nilprotect {
    -- State:
    portal_next: {}
    level_connections: {}
    last_touched_portal: false
    -- Helpers:
    _current_label: () => Map.map_label(player.map)
    _get_label_distances: (root_label) =>
        distance_levels = {
            {root_label}
        }
        distance_to = {
            [root_label]: 1 -- Distance level
        }
        dist = 1
        while dist <= #distance_levels
            labels = distance_levels[dist]
            for label in *labels
                for conn in *(@level_connections[label] or {})
                    if not distance_to[conn]
                        distance_levels[dist + 1] or= {}
                        append distance_levels[dist + 1], conn
                        distance_to[conn] = dist + 1
            dist += 1
        return distance_to
    -- Public API:
    step: () =>
        if @last_touched_portal and not @portal_next[@last_touched_portal]
            {:map} = @last_touched_portal
            if player.map ~= map
                @portal_next[@last_touched_portal] = Map.map_label(map)
            @level_connections[@_current_label()] or= {}
            append @level_connections[@_current_label()], Map.map_label(map)
            @level_connections[Map.map_label(map)] or= {}
            append @level_connections[Map.map_label(map)], @_current_label()
        for portal in *_collisions(player, "feature")
            @last_touched_portal = portal
            break
    -- Gives the closest portal that will take us where we want to go.
    closest_portal: (map_label) =>
        if map_label == @_current_label()
            return nil
        distances = @_get_label_distances(map_label)
        current_distance = distances[@_current_label()]
        if current_distance == nil
            return nil
        closest,closest_distance = nil,math.huge
        for portal in *_objects(player, "feature")
            next = @portal_next[portal]
            if not next
                continue
            if distances[next] == nil
                continue
            if distances[next] < current_distance
                obj_dist = vector_distance(portal.xy, player.xy)
                if obj_dist < closest_distance
                    closest = portal
                    closest_distance = obj_dist 
        return closest 
    unused_portals: () =>
        portals = _objects player, "feature"
        return table.filter portals, (p) -> not @portal_next[p]
}

path_planner = (player) -> nilprotect {
    -- State:
    stored_path: {}
    coord: false
    get_next_coord: () =>
        while true
            next = @stored_path[@coord]
            if not next
                return nil -- Path done
            {:xy} = player
            if vector_distance(next, xy) >= 6
                return next -- Next coord far enough away to path towards
            -- Next coord close enough to continue
            @coord += 1
    set_path_towards: (obj) =>
        {:map, :tile_xy} = player
        @stored_path = ASTAR_BUFFER\calculate_path map, tile_xy, obj.tile_xy
        @coord = 2
    next_direction_towards: () =>
        if @coord == #@stored_path + 1
            @coord += 1
            return {0,0}
        coord = @get_next_coord()
        if not coord
            return nil
        {tx, ty} = coord
        {:x, :y} = player
        dx, dy = tx - x, ty - y
        if dx ~= 0 then dx /= math.abs(dx)
        if dy ~= 0 then dy /= math.abs(dy)
        return {dx, dy}
}

ai_state = (player) -> {
    portal_planner: portal_planner(player)
    path_planner: path_planner(player)
    key_items: {"Azurite Key", "Dandelite Key"}
    rng: require("mtwist").create(HARDCODED_AI_SEED)
    step: () =>
        @portal_planner\step()
    next_key_item: () =>
        for item in *@key_items
            if require("core.GlobalData").keys_picked_up[item]
                continue
            if require("core.GlobalData").lanarts_picked_up[item]
                continue
            return item
        return nil
    get_next_direction: () => 
        if @rng\randomf() < 0.1
            return @get_next_wander_direction()
        dir = @path_planner\next_direction_towards()
        --pretty(dir)
        if dir
            return dir
        --next_obj = nil
        --for obj in *_objects(player, "item")
        --    if obj.type == @next_key_item()
        --        next_obj = obj
        --        break
        --if not next_obj
        --    if @next_key_item() == "Azurite Key"
        --        for i=2,1,-1
        --            if next_obj 
        --                break
        --            next_obj = @portal_planner\closest_portal("Snake Pit ".. i)
        --        for i=3,1,-1
        --            if next_obj 
        --                break
        --            next_obj = @portal_planner\closest_portal("Temple ".. i)
        --    elseif @key_items[2] == ""
        --        for i=3,1,-1
        --            if next_obj 
        --                break
        --            next_obj = @portal_planner\closest_portal("Outpost ".. i)
        return @get_next_wander_direction()
        --if next_obj
        --    @path_planner\set_path_towards(next_obj)
        --    return @path_planner\next_direction_towards()
        --return nil
    _closest: (objs) =>
        o = nil
        min_dist_sqr = math.huge
        for obj in *objs
            dx, dy = obj.x - player.x, obj.y - player.y
            dist_sqr = dx*dx+dy*dy
            if dist_sqr < min_dist_sqr
                min_dist_sqr = dist_sqr
                o = obj
        return o
    get_next_wander_direction: () => 
        --assert #_objects(player, "feature") > 0
        objs = @portal_planner\unused_portals()
        if #objs == 0
            objs = _objects(player, "feature") --@rng\random_choice {"item", "feature"})
        if #objs == 0
            return nil
        --next_obj = @rng\random_choice(objs)
        next_obj = @_closest objs
        @path_planner\set_path_towards(next_obj)
        return @path_planner\next_direction_towards()
}

return {:portal_planner, :path_planner, :ai_state}
