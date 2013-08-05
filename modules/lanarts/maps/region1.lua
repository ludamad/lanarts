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

local M = {} -- Submodule

-- End of cruft

-- Overworld to template map sequence (from overworld to deeper in the temple)
local TEMPLE_DEPTH = 2
local O2T = MapSequence.create {preallocate = TEMPLE_DEPTH + 1}
--local temple_to_overworld = MapSequence.create {preallocate = 2}

local function temple_level_base_apply(map, --[[Optional]] area)
    local tileset = TileSets.temple
    MapGen.random_placement_apply { map = map, area = area,
        child_operator = dungeons.room_carve_operator(tileset.wall, tileset.floor),
        size_range = {12,15}, amount_of_placements_range = {10,15},
        create_subgroup = false
    }
    dungeons.simple_tunnels(map, 2, {1,1}, tileset.wall, tileset.floor)
    dungeons.simple_tunnels(map, 1, {0,1}, tileset.wall, tileset.floor_alt)
end

local function temple_level_create(floor)
    print("Floor ", floor, " was created!")
    local tileset = TileSets.temple
    local map
   -- if floor == 1 then
        map = map_utils.map_create({40, 40}, tileset.wall)
        temple_level_base_apply(map)
   -- end

    local map_area = bbox_create({0,0}, map.size)

    local seq_idx = floor+1 -- Add one to account for overworld
    -- Resolve connections
    for i=1,3 do
        local portal = map_utils.random_portal(map, map_area, "stairs_up")
        local portal_num = i
        -- There is only one cave entrance
        if floor == 1 then portal_num = 1 end
        O2T:backward_portal_resolve(seq_idx, portal, portal_num)
        if floor < TEMPLE_DEPTH then
            local portal = map_utils.random_portal(map, map_area, "stairs_down")
            O2T:forward_portal_add(seq_idx, portal, i, function() return temple_level_create(floor+1) end)
        end
    end

    for i=1,2 do map_utils.random_enemy(map, "Giant Bat") end
    for i=1,2 do map_utils.random_enemy(map, "Giant Rat") end

    return O2T:slot_resolve(seq_idx, map_utils.game_map_create(map))
end

local FLAG_PLAYERSPAWN = MapGen.FLAG_CUSTOM1

local function find_player_positions(map) 
    local positions = {}
    local map_area = bbox_create({0,0},map.size)
    for i=1,World.player_amount do
        local sqr = map_utils.random_square(map, map_area, --[[Selector]] { matches_all = {FLAG_PLAYERSPAWN}, matches_none = {MapGen.FLAG_SOLID, MapGen.FLAG_HAS_OBJECT} })
        if not sqr then error("Could not find player spawn position for player " .. i .. "!") end
        positions[i] = {(sqr[1]+.5) * Maps.TILE_SIZE, (sqr[2]+.5) * Maps.TILE_SIZE}
    end
    return positions
end

local function generate_store(map, xy)
    local items = {}
    for i=1,random(5,10) do
        table.insert(items, item_utils.item_generate(item_groups.basic_items, true))
    end
    table.insert(map.instances, GameObject.store_create {
        xy = {(xy[1]+.5)*Maps.TILE_SIZE, (xy[2]+.5)*Maps.TILE_SIZE},
        items = items,
        do_init = false,
        sprite = "store"
    })
end

function M.overworld_create()	
	local tileset = TileSets.grass
	local portal_number = 1
	local map = map_utils.area_template_to_map(path_resolve "region1.txt", { 
	       ['x'] = { add = {MapGen.FLAG_SEETHROUGH, MapGen.FLAG_SOLID}, content = tileset.wall }, 
	       ['.'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor },
           ['1'] = { add = {MapGen.FLAG_SEETHROUGH, FLAG_PLAYERSPAWN}, content = tileset.floor } ,
           ['<'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor, 
               on_placement = function(map, xy)
                    map_utils.spawn_decoration(map, "anvil", xy)
               end},
           ['T'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor,
               on_placement = function(map, xy)
                    local portal = map_utils.spawn_portal(map, xy, "stairs_down")
                    O2T:forward_portal_add(1, portal, portal_number, function() return temple_level_create(1) end)
                    portal_number = portal_number + 1
               end},
           ['S'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor, on_placement = generate_store } 
	})

	for i=1,2 do map_utils.random_enemy(map, "Giant Bat") end
	for i=1,2 do map_utils.random_enemy(map, "Giant Rat") end

    local map_id = O2T:slot_resolve(1, map_utils.game_map_create(map))
    World.players_spawn(map_id, find_player_positions(map))
    return map_id
end

return M