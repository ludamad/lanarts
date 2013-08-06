local utils = import "core.utils"
local MapGen = import "core.map_generation"
local Maps = import "core.maps"
local GameObject = import "core.GameObject"
local World = import "core.GameWorld"

local TileSets = import "@tiles.tilesets"

local map_utils = import ".map_utils"
local dungeons = import ".dungeons"
local PortalSet = import ".PortalSet"
local MapSequence = import ".MapSequence"

local item_utils = import ".item_utils"
local item_groups = import ".item_groups"

local old_maps = import ".old_maps"

local M = {} -- Submodule

-- End of cruft

-- For using stair_kinds structure
local function stair_kinds_index(x, y)
    return y*6+x -- stair_kinds.png has rows of 6
end

-- Overworld to template map sequence (from overworld to deeper in the temple)
local TEMPLE_DEPTH = 2
local O2T = MapSequence.create {preallocate = TEMPLE_DEPTH + 1}
-- Overworld to template map sequence (from deeper in the temple to the overworld)
local T2O = MapSequence.create {preallocate = TEMPLE_DEPTH + 1}
--local temple_to_overworld = MapSequence.create {preallocate = 2}

local function temple_level_base_apply(map, --[[Optional]] area)
    local tileset = TileSets.temple
    MapGen.random_placement_apply { map = map, area = area,
        child_operator = dungeons.room_carve_operator(tileset.wall, tileset.floor),
        size_range = {12,15}, amount_of_placements_range = {10,15},
        create_subgroup = false
    }
    dungeons.simple_tunnels(map, {2,2}, {1,1}, tileset.wall, tileset.floor)
    dungeons.simple_tunnels(map, {1,1}, {0,1}, tileset.wall, tileset.floor_alt)
end

local function temple_level_create(floor, tileset)
    local map
   -- if floor == 1 then
        map = map_utils.map_create({40, 40}, tileset.wall)
        temple_level_base_apply(map)
   -- end

    local map_area = bbox_create({0,0}, map.size)

    local seq_idx = floor+1 -- Add one to account for overworld
    -- Resolve connections
    for i=1,3 do
        local portal, portal_num = nil, i
        if floor == 1 then
            portal = map_utils.random_portal(map, map_area, "stair_kinds", nil, stair_kinds_index(0,0))
            portal_num = 1
        else 
            portal = map_utils.random_portal(map, map_area, "stair_kinds", nil, stair_kinds_index(5,7))
        end
        local portal_num = i
        -- There is only one cave entrance
        if floor == 1 then portal_num = 1 end
        O2T:backward_portal_resolve(seq_idx, portal, portal_num)
         if floor < TEMPLE_DEPTH then
            local portal = map_utils.random_portal(map, map_area, "stair_kinds", nil, stair_kinds_index(4,7))
            O2T:forward_portal_add(seq_idx, portal, i, function() return temple_level_create(floor+1, tileset) end)
        end
    end

    for i=1,2 do map_utils.random_enemy(map, "Giant Bat") end
    for i=1,2 do map_utils.random_enemy(map, "Giant Rat") end

    return O2T:slot_resolve(seq_idx, map_utils.game_map_create(map))
end

local FLAG_PLAYERSPAWN = MapGen.FLAG_CUSTOM1

local function find_player_positions(map, --[[Optional]] flags) 
    local positions = {}
    local map_area = bbox_create({0,0},map.size)
    for i=1,World.player_amount do
        local sqr = map_utils.random_square(map, map_area, --[[Selector]] { matches_all = flags, matches_none = {MapGen.FLAG_SOLID, MapGen.FLAG_HAS_OBJECT} })
        if not sqr then error("Could not find player spawn position for player " .. i .. "!") end
        positions[i] = {(sqr[1]+.5) * Maps.TILE_SIZE, (sqr[2]+.5) * Maps.TILE_SIZE}
    end
    return positions
end

local function generate_store(map, xy)
    local items = {}
    for i=1,random(5,10) do
        table.insert(items, item_utils.item_generate(chance(.5) and item_groups.basic_items or item_groups.enchanted_items, true))
    end
    table.insert(map.instances, GameObject.store_create {
        xy = {(xy[1]+.5)*Maps.TILE_SIZE, (xy[2]+.5)*Maps.TILE_SIZE},
        items = items,
        do_init = false,
        sprite = "store",
        frame = random(0,4)
    })
end

--Test
local function connect_map(args)
    local map = args.map
    local seq_idx = args.sequence_index
    local MapSeq = args.map_sequence
    local map_area = bbox_create({0,0}, map.size)
    for i=1,MapSeq:number_of_backward_portals(seq_idx) do
--         Always have 3 going 'out'
--        local iterations = 1
--        if seq_idx == 2 then iterations = 3 end
--        for i=1,iterations do
            local portal
            if seq_idx == 2 then
                portal = map_utils.random_portal(map, map_area, args.sprite_out or args.sprite_up, nil, args.sprite_out_index or args.sprite_up_index)
            else
                portal = map_utils.random_portal(map, map_area, args.sprite_up, nil, args.sprite_up_index)
            end
            MapSeq:backward_portal_resolve(seq_idx, portal, i)
--        end
    end
    for i=1,args.forward_portals do
        print("Generating forward portal!")
        local portal = map_utils.random_portal(map, map_area, args.sprite_down, nil, args.sprite_down_index)
        MapSeq:forward_portal_add(seq_idx, portal, i, args.next_floor_callback)
    end
end

local function old_map_generate(MapSeq, tileset, floor)
    local map = old_maps.create_map(floor, tileset)
    local last_floor = (floor >= old_maps.last_floor)
    local seq_idx = MapSeq:slot_create()
    connect_map {
        map = map, 
        map_sequence = MapSeq, sequence_index = seq_idx,
        sprite_up =  "stair_kinds", sprite_up_index = stair_kinds_index(5, 2),
        sprite_out =  "stair_kinds", sprite_out_index = 0,
        sprite_down = "stair_kinds", sprite_down_index = stair_kinds_index(4, 2),
        forward_portals = last_floor and 0 or 3,
        next_floor_callback = function() 
            return old_map_generate(MapSeq, tileset, floor + 1)
        end
    }
    return MapSeq:slot_resolve(seq_idx, map_utils.game_map_create(map))
end

local function old_dungeon_placement_function(MapSeq, tileset)
    local seq_idx = MapSeq:slot_create()
    return function(map, xy)
        local portal = map_utils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(1, 12))
        MapSeq:forward_portal_add(seq_idx, portal, 1, function() 
            return old_map_generate(MapSeq, tileset, 1) 
        end)
    end
end

local function random_tileset() 
    return random_choice{TileSets.pebble, TileSets.temple};
end

function M.overworld_create()   
    local tileset = TileSets.grass
    local portal_number = 1
    local OldMapSeq = MapSequence.create()
    local map = map_utils.area_template_to_map(path_resolve "region1.txt", { 
           ['x'] = { add = {MapGen.FLAG_SEETHROUGH, MapGen.FLAG_SOLID}, content = tileset.wall }, 
           ['z'] = { add = MapGen.FLAG_SOLID, content = tileset.wall_alt }, 
           ['.'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor },
           [','] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.dirt },
           ['1'] = { add = {MapGen.FLAG_SEETHROUGH, FLAG_PLAYERSPAWN}, content = tileset.floor } ,
           ['<'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.dirt, 
               on_placement = function(map, xy)
                    map_utils.spawn_decoration(map, "anvil", xy)
               end},
           ['p'] = { add = MapGen.FLAG_SEETHROUGH, content = TileSets.pebble.floor }, 
           ['t'] = { add = MapGen.FLAG_SEETHROUGH, content = TileSets.temple.floor }, 
           ['T'] = { add = MapGen.FLAG_SEETHROUGH, content = TileSets.temple.floor,
               on_placement = function(map, xy)
                    local portal = map_utils.spawn_portal(map, xy, "stair_kinds", nil, stair_kinds_index(1, 11))
                    O2T:forward_portal_add(1, portal, portal_number, function() return temple_level_create(1, TileSets.temple) end)
                    portal_number = portal_number + 1
               end},           
           ['D'] = { add = MapGen.FLAG_SEETHROUGH, content = TileSets.temple.floor,
               on_placement = old_dungeon_placement_function(OldMapSeq, TileSets.pebble) },
           ['S'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor, on_placement = generate_store } 
    })

    local map_id = O2T:slot_resolve(1, map_utils.game_map_create(map))
    OldMapSeq:slot_resolve(1, map_id)
    World.players_spawn(map_id, find_player_positions(map, FLAG_PLAYERSPAWN))
    return map_id
end
return M