local SourceMap = require "core.SourceMap"
local Map = require "core.Map"
local Display = require "core.Display"
local GameObject = require "core.GameObject"
local World = require "core.World"

local TileSets = require "tiles.Tilesets"

local MapUtils = require "maps.MapUtils"
local dungeons = require "maps.Dungeons"
local PortalSet = require "maps.PortalSet"
local MapSequence = require "maps.MapSequence"

local ItemUtils = require "maps.ItemUtils"
local ItemGroups = require "maps.ItemGroups"

local OldMaps = require "maps.OldMaps"

local M = {} -- Submodule

-- End of cruft

-- For using stair_kinds structure
local function stair_kinds_index(x, y)
    return y*6+x -- stair_kinds.png has rows of 6
end
M.stair_kinds_index = stair_kinds_index

-- Helps with connecting the overworld to the dungeons
local function connect_map(args)
    local map = args.map
    local seq_idx = args.sequence_index
    local MapSeq = args.map_sequence
    local map_area = bbox_create({0,0}, map.size)
    local back_portals = {}
    local forward_portals = {}

    for i=1,MapSeq:number_of_backward_portals(seq_idx) do
        local portal
        if seq_idx == 2 then
            portal = (args.spawn_portal or MapUtils.random_portal)(map, map_area, args.sprite_out or args.sprite_up, nil, args.sprite_out_index or args.sprite_up_index)
        else
            portal = (args.spawn_portal or MapUtils.random_portal)(map, map_area, args.sprite_up, nil, args.sprite_up_index)
        end
        if not portal then
            return nil
        end
        append(back_portals, portal)
    end

    for i=1,args.forward_portals do
        local portal = MapUtils.random_portal(map, map_area, args.sprite_down, nil, args.sprite_down_index)
        if not portal then
            return nil
        end
        append(forward_portals, portal)
    end

    for i=1,MapSeq:number_of_backward_portals(seq_idx) do
        local iterations = args.backward_portal_multiple or 1
        for iter=1,iterations do
            MapSeq:backward_portal_resolve(seq_idx, back_portals[i], i)
        end
    end
    local t = {}
    for i=1,args.forward_portals do
        append(t, MapSeq:forward_portal_add(seq_idx, forward_portals[i], i, args.next_floor_callback))
    end
    if World.player_amount > 1 then
        for c in values(t) do c() end
    end
    return true
end

-- Overworld to template map sequence (from overworld to deeper in the temple)
local TEMPLE_DEPTH = 2

local function temple_level_base_apply(map, tileset, area)
    SourceMap.random_placement_apply { rng = map.rng, map = map, area = area,
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

    local game_map = MapUtils.game_map_create(map, true)
    for i=1,#sequences do
        sequences[i]:slot_resolve(sequence_ids[i], map_id)
    end
    return game_map
end

local FLAG_PLAYERSPAWN = SourceMap.FLAG_CUSTOM1

local function find_player_positions(map, --[[Optional]] flags) 
    local positions = {}
    local map_area = bbox_create({0,0},map.size)
    for i=1,World.player_amount do
        local sqr = MapUtils.random_square(map, map_area, 
            --[[Selector]] { matches_all = flags, matches_none = {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT} },
            --[[Operator]] nil, 
            --[[Max attempts]] 10000
        )
        if not sqr then error("Could not find player spawn position for player " .. i .. "!") end
        positions[i] = {(sqr[1]+.5) * Map.TILE_SIZE, (sqr[2]+.5) * Map.TILE_SIZE}
    end
    return positions
end

function M.generate_store(map, xy)
    local items = {}
    for i=1,random(5,10) do
        table.insert(items, ItemUtils.item_generate(ItemGroups.store_items, true))
    end
    MapUtils.spawn_store(map, items, xy)
end

local old_map_generate
local function try_old_map_generate(MapSeq, seq_idx, dungeon, floor)
    floor = floor or 1
    local map = nil
    local wrapped_on_generate = dungeon.on_generate
    local last_floor = (floor >= #dungeon.templates)
    dungeon.on_generate = function(map, floor)
        if wrapped_on_generate and not wrapped_on_generate(map, floor) then
            return nil
        end
        if not connect_map {
            map = map,
            spawn_portal = dungeon.spawn_portal, 
            backward_portal_multiple = (floor == 1) and 3 or 1,
            map_sequence = MapSeq, sequence_index = seq_idx,
            sprite_up =  "stair_kinds", sprite_up_index = stair_kinds_index(5, 9),
            sprite_out =  "stair_kinds", sprite_out_index = dungeon.sprite_out or 0,
            sprite_down = "stair_kinds", sprite_down_index = stair_kinds_index(4, 9),
            forward_portals = last_floor and 0 or 3,
            next_floor_callback = function() 
                dungeon.on_generate = wrapped_on_generate
                return old_map_generate(MapSeq, dungeon, floor + 1)
            end
        } then
            return nil
        end
        return true
    end
    for i=1,100 do
        map = OldMaps.create_map(dungeon, floor)
        if map ~= nil then
            break
        end
        print("Dungeon rejected ("..i..")!")
    end
    return map
end

function old_map_generate(MapSeq, dungeon, floor)
    local seq_idx = MapSeq:slot_create()
    for i=1,200 do
        local map = try_old_map_generate(MapSeq, seq_idx, dungeon, floor) 
        if map then
            local game_map = MapUtils.game_map_create(map, true)
            if dungeon.post_generate then
                dungeon.post_generate(game_map)
            end
            MapSeq:slot_resolve(seq_idx, game_map)
            return game_map
        end
        print("FAILED TO GENERATE VALID MAP in old_map_generate " .. i .. " retrying...")
    end
    assert("Tried really hard and couldnt make an important dungeon. Huh? Contact a dev")
end
-- dungeon requires:
-- - label
-- - templates (from OldMaps.lua, eg OldMaps.Dungeon1)
-- - tileset (from TileSets.lua, eg TileSets.snakes)
function M.old_dungeon_placement_function(MapSeq, dungeon)
    return function(map, xy)
        local portal = MapUtils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(1, 12))
        local c = MapSeq:forward_portal_add(1, portal, 1, function() 
            return old_map_generate(MapSeq, dungeon, 1) 
        end)
        if World.player_amount > 1 then
            c()
        end
    end
end

local NOT_SOLID_QUERY = {matches_none = SourceMap.FLAG_SOLID}
local function try_copy_unsolid(map, xy1, xy2) 
    if map:square_query(xy2, NOT_SOLID_QUERY) then
        map:set(xy1, map:get(xy2))
        assert(map:square_query(xy1, NOT_SOLID_QUERY), "Square should not be solid after try_copy_unsolid!")
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

M._warning_skull = Display.image_load "features/sprites/warning.png"
M._anvil = Display.image_load "features/sprites/anvil.png"
local OVERWORLD_VISION_RADIUS = 10
function M.overworld_create()   
    local tileset = TileSets.grass
    local OldMapSeq1 = MapSequence.create {preallocate = 1}
    local OldMapSeq2 = MapSequence.create {preallocate = 1}
    local temple_sequences = {}
    local dirthole_sequences = {}

    -- These squares take the form of a square next to them, after the initial map generation is done
    local undecided_squares = {}

    local portals = {}
    local UNDECIDED_FLAG, UNDECIDED_TILE = SourceMap.FLAG_SOLID, tileset.wall
    local map = MapUtils.area_template_to_map("Overworld", 
    		--[[file path]] path_resolve "region1.txt", 
    		--[[padding]] 4, {
    	   --Player spawn candidate square
           [':'] = { add = {SourceMap.FLAG_SEETHROUGH, FLAG_PLAYERSPAWN}, content = tileset.floor } ,
    	   --Walls
           ['x'] = { add = {SourceMap.FLAG_SEETHROUGH, SourceMap.FLAG_SOLID}, content = tileset.wall }, 
           ['W'] = { add = SourceMap.FLAG_SOLID, content = tileset.wall_alt }, 
           ['P'] = { add = SourceMap.FLAG_SOLID, content = TileSets.pebble.wall }, 
           --Floors
           ['s'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.snake.floor},
           ['.'] = { add = SourceMap.FLAG_SEETHROUGH, content = tileset.floor },
           ['b'] = { add = SourceMap.FLAG_SEETHROUGH, content = tileset.dirt },
           [','] = { add = SourceMap.FLAG_SEETHROUGH, content = tileset.floor_alt1 },
           ['`'] = { add = SourceMap.FLAG_SEETHROUGH, content = tileset.floor_alt2 },
           ['p'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.pebble.floor }, 
           ['h'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.pebble.floor_alt }, 
           ['t'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.temple.floor }, 
           ['T'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE,
               on_placement = function(map, xy)
                    local portal = MapUtils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(1, 11))
                    local seq_len = #temple_sequences
                    temple_sequences[seq_len + 1] = MapSequence.create {preallocate = 1}
                    local c = temple_sequences[seq_len + 1]:forward_portal_add(1, portal, 1, 
                        function() 
                            return temple_level_create("Temple", 1, temple_sequences, TileSets.temple, {"Skeleton", "Dark Centaur"})
                    end)
                    table.insert(portals, c)
                    table.insert(undecided_squares, xy)
               end},

           --Entrance to Dungeon 1: easier monsters
           ['D'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.pebble.floor,
               on_placement = M.old_dungeon_placement_function(OldMapSeq1, TileSets.pebble, {1,5}) },

           --Entrance to Dungeon 2: harder monsters
           ['X'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.snake.floor,
               on_placement = M.old_dungeon_placement_function(OldMapSeq2, TileSets.snake, {6,10}) },
           -- Overworld features
           -- Anvil
           ['<'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE, 
               on_placement = function(map, xy)
                    MapUtils.spawn_decoration(map, M._anvil, xy)
                    table.insert(undecided_squares, xy)
               end},
           --Warning skull
           ['w'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE,
               on_placement = function(map, xy)
                    MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
                    table.insert(undecided_squares, xy)
               end},
           --Store candidate square
           ['S'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE, 
           			on_placement = function(map, xy) 
           				if chance(.4) then 
           					M.generate_store(map, xy)
           				end
                        table.insert(undecided_squares, xy)
           			end },
           --Item candidate square
           ['i'] = { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.pebble.floor, 
                    on_placement = function(map, xy)
                        if chance(.4) then 
                            local item = ItemUtils.item_generate(ItemGroups.enchanted_items)
                            MapUtils.spawn_item(map, item.type, item.amount, xy)
                        end
                        table.insert(undecided_squares, xy)
                    end },           
           --Door
           ['d'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE, 
                   on_placement = function (map, xy) 
                        MapUtils.spawn_door(map,xy)
                        table.insert(undecided_squares, xy)
                   end },               
           --Gold candidate square
           ['*'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE, 
                   on_placement = function (map, xy) 
                        if chance(.4) then 
                            MapUtils.spawn_item(map, "Gold", random(2,10), xy)
                        end
                        table.insert(undecided_squares, xy)
                   end },           
           --Enemy candidate square
           ['e'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE, 
                    on_placement = function(map, xy)
                        local enemy = OldMaps.enemy_generate(OldMaps.harder_enemies)
                        MapUtils.spawn_enemy(map, enemy, xy)
                        table.insert(undecided_squares, xy)
                    end },          
           --Red dragon square
           ['E'] = { add = UNDECIDED_FLAG, content = UNDECIDED_TILE, 
                    on_placement = function(map, xy)
                        MapUtils.spawn_enemy(map, "Red Dragon", xy)
                        table.insert(undecided_squares, xy)
                    end },
           --Entrance to 'Mines': a connecting mini-dungeon
           ['G'] =  { add = SourceMap.FLAG_SEETHROUGH, content = TileSets.pebble.floor,
               on_placement = function(map, xy)
                    local portal = MapUtils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(0, 2))
                    local seq_len = #dirthole_sequences
                    dirthole_sequences[seq_len + 1] = MapSequence.create {preallocate = 1}
                    local c = dirthole_sequences[seq_len + 1]:forward_portal_add(1, portal, 1, 
                        function()
                            return temple_level_create("Mines", 1, dirthole_sequences, TileSets.pebble, {"Golem", "Zombie"})
                    end)
                    table.insert(portals, c)
               end}
    })

    -- For all undecided squares, copy over the square from the left
    for xy in values(undecided_squares) do
        copy_close_unsolid_tile(map, xy)
    end

    OldMaps.generate_from_enemy_entries(map, OldMaps.medium_animals, 40)

    local game_map = MapUtils.game_map_create(map)
    OldMapSeq1:slot_resolve(1, game_map)
    OldMapSeq2:slot_resolve(1, game_map)
    for MapSeq in values(temple_sequences) do
        MapSeq:slot_resolve(1, game_map)
    end
    for MapSeq in values(dirthole_sequences) do
        MapSeq:slot_resolve(1, game_map)
    end

    World.players_spawn(game_map, find_player_positions(map, FLAG_PLAYERSPAWN))
    for p in values(portals) do p() end
    Map.set_vision_radius(game_map, OVERWORLD_VISION_RADIUS)
    return game_map
end
return M
