local MapGen = import "core.map_generation"
local GameMap = import "core.GameMap"
local GameObject = import "core.GameObject"
local World = import "core.GameWorld"

local TileSets = import "@tiles.tilesets"

local MapUtils = import ".map_utils"
local ShapeUtils = import ".shape_utils"
local Layouts  = import ".layouts"
local dungeons = import ".dungeons"
local PortalSet = import ".PortalSet"
local MapSequence = import ".MapSequence"

local ItemUtils = import ".item_utils"
local ItemGroups = import ".item_groups"

local OldMaps = import ".old_maps"

local M = {} -- Submodule

-- End of cruft

-- For using stair_kinds structure
local function stair_kinds_index(x, y)
    return y*6+x -- stair_kinds.png has rows of 6
end

-- Helps with connecting the overworld to the dungeons
local function connect_map(args)
    local map = args.map
    local seq_idx = args.sequence_index
    local MapSeq = args.map_sequence
    local map_area = bbox_create({0,0}, map.size)
    for i=1,MapSeq:number_of_backward_portals(seq_idx) do
        local iterations = args.backward_portal_multiple or 1
        for iter=1,iterations do
            local portal
            if seq_idx == 2 then
                portal = MapUtils.random_portal(map, map_area, args.sprite_out or args.sprite_up, nil, args.sprite_out_index or args.sprite_up_index)
            else
                portal = MapUtils.random_portal(map, map_area, args.sprite_up, nil, args.sprite_up_index)
            end
            MapSeq:backward_portal_resolve(seq_idx, portal, i)
        end
    end
    for i=1,args.forward_portals do
        local portal = MapUtils.random_portal(map, map_area, args.sprite_down, nil, args.sprite_down_index)
        MapSeq:forward_portal_add(seq_idx, portal, i, args.next_floor_callback)
    end
end

-- Overworld to template map sequence (from overworld to deeper in the temple)
local TEMPLE_DEPTH = 2

local function temple_level_base_apply(map, tileset, area)
    MapGen.random_placement_apply { map = map, area = area,
        child_operator = dungeons.room_carve_operator(tileset.wall, tileset.floor),
        size_range = {12,15}, amount_of_placements_range = {10,15},
        create_subgroup = false
    }
    dungeons.simple_tunnels(map, {2,2}, {1,1}, tileset.wall, tileset.floor, area)
    dungeons.simple_tunnels(map, {1,1}, {0,1}, tileset.wall, tileset.floor_tunnel, area)
end

local function temple_level_create(label, floor, sequences, tileset, enemy_candidates)
    local map = MapUtils.map_create( label .. ' ' .. floor, {40+floor*10, 40+floor*10}, tileset.wall)
    temple_level_base_apply(map, tileset, bbox_create({4,4}, vector_add(map.size, {-8,-8})))

    local map_area = bbox_create({0,0}, map.size)

    local done_once = false
    local sequence_ids = {}
    for MapSeq in values(sequences) do
        local seq_idx = MapSeq:slot_create()
        table.insert(sequence_ids, seq_idx)
        local no_forward = (floor >= TEMPLE_DEPTH or (done_once and floor == 1))
        connect_map {
            map = map, 
            map_sequence = MapSeq, sequence_index = seq_idx,
            sprite_up =  "stair_kinds", sprite_up_index = stair_kinds_index(5, 7),
            sprite_out =  "stair_kinds", sprite_out_index = done_once and stair_kinds_index(0, 9) or 0,
            sprite_down = "stair_kinds", sprite_down_index = stair_kinds_index(4, 7),
            forward_portals = no_forward and 0 or 1,
            next_floor_callback = function() 
                return temple_level_create(label, floor +1, sequences, tileset, enemy_candidates)
            end
        }
        done_once = true
    end

    for i=1,10 do MapUtils.random_enemy(map, random_choice(enemy_candidates)) end
    if floor == TEMPLE_DEPTH then
        for i=1,20 do
            ItemUtils.item_object_generate(map, ItemGroups.basic_items)
        end
    end

    local map_id = MapUtils.game_map_create(map, true)
    for i=1,#sequences do
        sequences[i]:slot_resolve(sequence_ids[i], map_id)
    end
    return map_id
end

local FLAG_PLAYERSPAWN = MapGen.FLAG_CUSTOM1

local function find_player_positions(map, --[[Optional]] flags) 
    local positions = {}
    local map_area = bbox_create({0,0},map.size)
    for i=1,World.player_amount do
        local sqr = MapUtils.random_square(map, map_area, 
            --[[Selector]] { matches_all = flags, matches_none = {MapGen.FLAG_SOLID, MapGen.FLAG_HAS_OBJECT} },
            --[[Operator]] nil, 
            --[[Max attempts]] 10000
        )
        if not sqr then error("Could not find player spawn position for player " .. i .. "!") end
        positions[i] = {(sqr[1]+.5) * GameMap.TILE_SIZE, (sqr[2]+.5) * GameMap.TILE_SIZE}
    end
    return positions
end

local function generate_store(map, xy)
    local items = {}
    for i=1,random(5,10) do
        table.insert(items, ItemUtils.item_generate(chance(.5) and ItemGroups.basic_items or ItemGroups.enchanted_items, true))
    end
    MapUtils.spawn_store(map, items, xy)
end

local function old_map_generate(MapSeq, tileset, offset, max_floor, floor)
    floor = floor or 1
    local map = OldMaps.create_map("Dungeon "..floor, floor + offset, tileset)
    local last_floor = (floor >= max_floor)
    local seq_idx = MapSeq:slot_create()
    connect_map {
        map = map, 
        backward_portal_multiple = (floor == 1) and 3 or 1,
        map_sequence = MapSeq, sequence_index = seq_idx,
        sprite_up =  "stair_kinds", sprite_up_index = stair_kinds_index(5, 9),
        sprite_out =  "stair_kinds", sprite_out_index = 0,
        sprite_down = "stair_kinds", sprite_down_index = stair_kinds_index(4, 9),
        forward_portals = last_floor and 0 or 3,
        next_floor_callback = function() 
            return old_map_generate(MapSeq, tileset, offset, max_floor, floor + 1)
        end
    }
    return MapSeq:slot_resolve(seq_idx, MapUtils.game_map_create(map, true))
end

local function old_dungeon_placement_function(MapSeq, tileset, levels)
    return function(map, xy)
        local portal = MapUtils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(1, 12))
        MapSeq:forward_portal_add(1, portal, 1, function() 
            return old_map_generate(MapSeq, tileset, levels[1]-1, levels[2]-levels[1]+1) 
        end)
    end
end

local NOT_SOLID_QUERY = {matches_none = MapGen.FLAG_SOLID}
local function try_copy_unsolid(map, xy1, xy2) 
    if map:square_query(xy2, NOT_SOLID_QUERY) then
        map:set(xy1, map:get(xy2))
        return true
    end
    return false
end

local function copy_close_unsolid_tile(map, xy)
    for dx=-1,1,2 do
        if try_copy_unsolid(map, xy, {xy[1]+dx, xy[2]}) then
            return
        end
    end
    for dy=-1,1,2 do
        if try_copy_unsolid(map, xy, {xy[1], xy[2]+dy}) then
            return
        end
    end
    for dy=-1,1,2 do
        for dx=-1,1,2 do
            if dx ~= 0 and dy ~= 0 and try_copy_unsolid(map, xy, {xy[1]+dx, xy[2]+dy}) then
                return
            end
        end
    end
end

M._warning_skull = image_load "modules/lanarts/features/sprites/warning.png"
M._anvil = image_load "modules/lanarts/features/sprites/anvil.png"

local function random_pos(bbox)
    return {random(bbox[1], bbox[3]), random(bbox[2], bbox[4])}
end

local function random_mag(xy, angle, magrange)
    local mag = randomf(magrange[1], magrange[2])
    return vector_add(xy, {mag * math.sin(angle), mag * math.cos(angle)})
end

local function find_clear_patch(map, area)
    while true do
        local xy = MapUtils.random_square(map, area)
        local no_solid = MapGen.rectangle_query { 
            map=map, area = {xy[1]-1,xy[2]-1,xy[1]+1,xy[2]+1},
            fill_selector = {matches_none=MapGen.FLAG_SOLID}
        }
        if no_solid then
            return xy
        end
    end
end

local function place_content(map)
    local OldMapSeq1 = MapSequence.create {preallocate = 1}
    local OldMapSeq2 = MapSequence.create {preallocate = 1}
    local temple_sequences = {}
    local dirthole_sequences = {}

    local area = bbox_create({0,0}, map.size)

    -- Generate temple & mines
    for i=1,2 do
        local xy = find_clear_patch(map, area)
        local portal = MapUtils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(1, 11))
        local seq_len = #temple_sequences
        temple_sequences[seq_len + 1] = MapSequence.create {preallocate = 1}
        temple_sequences[seq_len + 1]:forward_portal_add(1, portal, 1, 
            function() 
                return temple_level_create("Temple", 1, temple_sequences, TileSets.temple, {"Skeleton", "Dark Centaur"})
        end)

        MapGen.rectangle_apply {
            map=map, area = {xy[1]-1,xy[2]-1,xy[1]+2,xy[2]+2}, 
            fill_operator = {matches_none = MapGen.FLAG_SOLID, content=TileSets.temple.floor}
        }

        xy = MapUtils.random_square(map, area)
        portal = MapUtils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(0, 2))
        seq_len = #dirthole_sequences
        dirthole_sequences[seq_len + 1] = MapSequence.create {preallocate = 1}
        dirthole_sequences[seq_len + 1]:forward_portal_add(1, portal, 1, 
            function()
                return temple_level_create("Mines", 1, dirthole_sequences, TileSets.pebble, {"Golem", "Zombie"})
        end)

        MapGen.rectangle_apply {
            map=map, area = {xy[1]-1,xy[2]-1,xy[1]+2,xy[2]+2}, 
            fill_operator = {matches_none = MapGen.FLAG_SOLID, content=TileSets.pebble.floor}
        }
    end

    local xy = MapUtils.random_square(map, area)
    MapGen.rectangle_apply {
        map=map, area = {xy[1]-1,xy[2]-1,xy[1]+2,xy[2]+2}, 
        fill_operator = {matches_none = MapGen.FLAG_SOLID, content=TileSets.pebble.floor}
    }
    old_dungeon_placement_function(OldMapSeq1, TileSets.pebble, {1,5})(map, xy)
    xy = MapUtils.random_square(map, area)
    old_dungeon_placement_function(OldMapSeq2, TileSets.snake, {6,10})(map, xy)
    for dxy in values{{1,0}, {-1,0}, {0,1}, {0,-1}} do
        local nxy = vector_add(xy, dxy)
        if map:square_query(nxy, {matches_none = MapGen.FLAG_SOLID}) then
            MapUtils.spawn_decoration(map, M._warning_skull, nxy)
        end
    end
    MapGen.rectangle_apply {
        map=map, area = {xy[1]-1,xy[2]-1,xy[1]+2,xy[2]+2}, 
        fill_operator = {matches_none = MapGen.FLAG_SOLID, content=TileSets.snake.floor}
    }

    for i=1,10 do
        local xy = find_clear_patch(map, area)
        map:square_apply(xy, {add = MapGen.FLAG_SOLID})
        MapUtils.spawn_decoration(map, M._anvil, xy)
    end
    for i=1,15 do
        if chance(.4) then 
            generate_store(map, find_clear_patch(map, area))
        end
    end
    for i=1,30 do
        if chance(.4) then 
            local item = ItemUtils.item_generate(ItemGroups.basic_items)
            MapUtils.spawn_item(map, item.type, item.amount, MapUtils.random_square(map, area))
        end
        if chance(.4) then 
            MapUtils.spawn_item(map, "Gold", random(2,10), MapUtils.random_square(map, area))
        end
    end
    for i=1,20 do
        local enemy = OldMaps.enemy_generate(OldMaps.medium_animals)
        MapUtils.spawn_enemy(map, enemy, find_clear_patch(map, area))
    end
--    MapUtils.spawn_enemy(map, "Red Dragon", MapUtils.random_square(map, area))

    local map_id = MapUtils.game_map_create(map)
    OldMapSeq1:slot_resolve(1, map_id)
    OldMapSeq2:slot_resolve(1, map_id)
    for MapSeq in values(temple_sequences) do
        MapSeq:slot_resolve(1, map_id)
    end
    for MapSeq in values(dirthole_sequences) do
        MapSeq:slot_resolve(1, map_id)
    end

    return map_id
end

--local function generate_inline_room(args)
--    MapGen.rectangle_query {map = args.map}
--end

local function sort_positions_by_angle(positions, centre)
    table.sort(positions, function(a,b)
        local angle1 = math.atan2(unpack(vector_subtract(a,centre)))
        local angle2 = math.atan2(unpack(vector_subtract(b,centre)))
        return angle1 < angle2
    end)
end

local function sort_positions_by_relative_distance(ps)
    for i=1,#ps do
        local idx, min_dist
        for j=i+1,#ps do
            local dxy = vector_subtract(ps[i], ps[j])
            local dist = dxy[1]*dxy[1] + dxy[2]*dxy[2]
            if not min_dist or dist < min_dist then
                idx,min_dist = j,dist
            end
        end
        if idx then
            -- Swap them
            ps[i+1],ps[idx] = ps[idx],ps[i+1]
        end
    end
end

local function generate_area(map, area)
    local tileset = TileSets.grass

    local w,h = unpack(map.size)
    local bounds = {1,1,w-1,h-1}
    local line_positions = {}

    local keep_position = random_choice{true, false}
    for i=1,random(20,50) do
        local xy = random_pos(area)
        MapGen.polygon_apply {
            map = map, area = bounds,
            operator = { remove = MapGen.FLAG_SOLID, add = MapGen.FLAG_SEETHROUGH, content = tileset.floor },
            points = ShapeUtils.random_polygon(area, xy, {4, random(5,20)}, random(5,12)) 
        }
        table.insert(line_positions, xy)

        xy = keep_position and xy or random_pos(area)
        MapGen.polygon_apply {
            map = map, area = bounds,
            operator = { matches_all = MapGen.FLAG_SOLID, remove = MapGen.FLAG_SEETHROUGH, content = TileSets.pebble.wall },
            points = ShapeUtils.random_polygon(area, xy, {4, random(5,15)}, random(5,12)) 
        }

        xy = keep_position and xy or random_pos(area)
        MapGen.polygon_apply {
            map = map, area = bounds,
            operator = { matches_none = MapGen.FLAG_SOLID, content = TileSets.snake.floor_alt },
            points = ShapeUtils.random_polygon(area, xy, {4, 5}, 8) 
        }
    end

    local type = random(1,3)
    if type == 2 then
        sort_positions_by_angle(line_positions, {100,100})
    elseif type == 3 then 
        sort_positions_by_relative_distance(line_positions)
    end
    for i=1,#line_positions do
        MapGen.line_apply {
            map = map, area = bounds,
            operator = { 
                remove = MapGen.FLAG_SOLID, 
                add = MapGen.FLAG_SEETHROUGH, content = random_choice{tileset.floor, tileset.floor_alt1, tileset.floor_alt2}
            },
            line_width = random(3,9),
            from_xy = line_positions[i], to_xy = line_positions[(i%#line_positions)+1 ]
        }
    end
--
    -- Generate small lines
    local NUM_RAND_LINES = random(20,50)
    for i=1,NUM_RAND_LINES do
        local xy = MapUtils.random_square(map, area)
        local content = random_choice{tileset.wall, TileSets.pebble.wall, TileSets.snake.wall}
        local add, remove = {MapGen.FLAG_SOLID}, MapGen.FLAG_SEETHROUGH
        if content == tileset.wall then
            table.insert(add, MapGen.FLAG_SEETHROUGH)
            remove = nil
        end

        local angle,mag = randomf(0, 2*math.pi), randomf(4,5)
        local dir = {math.sin(angle), math.cos(angle)}
        local to_xy = vector_add(xy, {mag * dir[1], mag * dir[2]})

        local line_width = random(1,2)
        -- First check for solid
        local step = vector_scale(dir, 3)
        local no_solid = MapGen.line_query {
            map=map,
            selector={matches_none=MapGen.FLAG_SOLID}, 
            line_width = line_width + 2, 
            from_xy = vector_subtract(xy, step), to_xy = vector_add(to_xy, step)
        }
        -- If none, then cut line
        if no_solid then
            MapGen.line_apply {
                map = map, area = bounds,
                operator = {add=add, remove=remove, content=content},
                line_width = line_width,
                from_xy = xy, to_xy = to_xy
            }
        end
    end
end

function M.overworld_create()
    local tileset = TileSets.grass

    local w,h = 600,600
    local map = MapUtils.map_create("Overworld", {w,h}, tileset.wall, {MapGen.FLAG_SOLID, MapGen.FLAG_SEETHROUGH})
    local bsp = MapGen.bsp_split { map = map, minimum_node_size = {w/3, h/3} }
    for area in values{bsp.left.area, bsp.right.area} do
        area = {area[1] + 25, area[2] + 25, area[3] - 25, area[4] - 25}
        generate_area(map, area)
    end

    Layouts.brute_tunnel(map, bsp.left.area, bsp.right.area, {
            operator = {
                matches_all=MapGen.FLAG_SOLID,
                add=MapGen.FLAG_SEETHROUGH, 
                remove=MapGen.FLAG_SOLID, 
                content=random_choice{tileset.floor, tileset.floor_alt1, tileset.floor_alt2}
            }
        }
    )

--   OldMaps.generate_from_enemy_entries(map, OldMaps.medium_animals, 40)

    local map_id = place_content(map)
    World.players_spawn(map_id, find_player_positions(map))
    return map_id
end
return M