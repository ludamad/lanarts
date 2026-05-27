local M = {}
local Catalog = require "engine.ContentCatalog"

local GameState
local Map
local World

local function load_core_modules()
    GameState = GameState or require "core.GameState"
    Map = Map or require "core.Map"
    World = World or require "core.World"
end

local function split_csv(value)
    local result = {}
    for part in tostring(value or ""):gmatch("[^,]+") do
        part = part:gsub("^%s+", ""):gsub("%s+$", "")
        if part ~= "" then
            result[#result + 1] = part
        end
    end
    return result
end

local function parse_item_spec(value)
    local spec = tostring(value or ""):gsub("^%s+", ""):gsub("%s+$", "")
    if spec == "" then
        return nil
    end
    local item, amount = spec:match("^(.-)%*([0-9]+)$")
    if item then
        item = item:gsub("^%s+", ""):gsub("%s+$", "")
        return {item = item, amount = math.max(1, tonumber(amount) or 1)}
    end
    return {item = spec, amount = 1}
end

local function append_loadout_specs(dest, specs)
    for _, raw_spec in ipairs(specs or {}) do
        local spec = type(raw_spec) == "table" and raw_spec or parse_item_spec(raw_spec)
        if spec and spec.item and spec.item ~= "" then
            dest[#dest + 1] = {
                item = spec.item,
                amount = math.max(1, tonumber(spec.amount) or 1),
            }
        end
    end
end

local function parse_loadout_specs(value)
    local specs = {}
    for _, part in ipairs(split_csv(value)) do
        append_loadout_specs(specs, {part})
    end
    return specs
end

local function encounter_by_id(encounter_id)
    for _, encounter in ipairs(Catalog.encounters or {}) do
        if encounter.id == encounter_id then
            return encounter
        end
    end
    return nil
end

local function encounter_monsters(encounter_id, custom_monsters)
    local monsters = split_csv(custom_monsters)
    if #monsters > 0 then
        return monsters
    end
    local encounter = encounter_by_id(encounter_id)
    if encounter and encounter.monsters then
        for _, monster in ipairs(encounter.monsters) do
            monsters[#monsters + 1] = monster
        end
    end
    return monsters
end

local function intensity_by_id(intensity_id)
    for _, intensity in ipairs(Catalog.intensities or {}) do
        if intensity.id == intensity_id then
            return intensity
        end
    end
    for _, intensity in ipairs(Catalog.intensities or {}) do
        if intensity.id == "standard" then
            return intensity
        end
    end
    return {
        id = "standard",
        label = "Standard",
        tempo = 1,
        damage = 1,
        summons = 1,
    }
end

local function loadout_specs_for_class(preset_name, custom_specs, class_name)
    local specs = {}
    local preset = Catalog.loadout_presets[preset_name or "none"] or Catalog.loadout_presets.none
    append_loadout_specs(specs, preset.default)
    append_loadout_specs(specs, preset[class_name])
    append_loadout_specs(specs, custom_specs)
    return specs
end

local function inventory_capacity(player)
    for slot_idx = 0, 255 do
        if not player:inventory_get(slot_idx) then
            return slot_idx
        end
    end
    return 256
end

local function item_is_equippable(item_name)
    local GameData = require "core.GameData"
    local item_data = GameData.item_definer(item_name)
    local method = item_data and item_data.__method
    return method == "weapon_create"
        or method == "equipment_create"
        or method == "projectile_create"
end

local function apply_loadout(player, specs)
    if not player or not specs or #specs == 0 then
        return
    end
    local capacity = inventory_capacity(player)
    for slot_idx = 0, capacity - 1 do
        player:inventory_set(slot_idx, {equipped = false})
    end
    for idx, spec in ipairs(specs) do
        if idx > capacity then
            error("Loadout has more items than inventory slots.")
        end
        player:inventory_set(idx - 1, {
            type = spec.item,
            amount = spec.amount,
            equipped = item_is_equippable(spec.item),
        })
    end
end

local function nearest_enemy_distance_sqr(source_map, xy)
    local best
    for _, obj in ipairs(source_map.instances or {}) do
        if obj.type == "enemy" and obj.xy then
            local dx = xy[1] - obj.xy[1]
            local dy = xy[2] - obj.xy[2]
            local dist = dx * dx + dy * dy
            if not best or dist < best then
                best = dist
            end
        end
    end
    return best or 0
end

local function pick_start_spawn_square(source_map, selector)
    local MapUtils = require "maps.MapUtils"
    local best_square
    local best_score
    for _ = 1, 96 do
        local square = MapUtils.random_square(source_map, nil, selector, {})
        if square then
            local xy = MapUtils.from_tile_xy(square)
            local score = nearest_enemy_distance_sqr(source_map, xy)
            if not best_score or score > best_score then
                best_square = square
                best_score = score
            end
        end
    end
    return best_square
end

function M.generate_encounter_arena(options)
    options = options or {}
    local monsters = encounter_monsters(options.encounter, options.monsters)
    if #monsters == 0 then
        error("Content lab encounter requested without any monsters.")
    end

    local encounter = encounter_by_id(options.encounter)
    local arena_options = encounter and encounter.arena or {}
    local game_map = require("map_descs.DebugArena"):generate_and_spawn(monsters, arena_options)
    if encounter and encounter.package then
        require("objects.ContentLabHazards").add_director(game_map, encounter.package, intensity_by_id(options.package_intensity))
    end
    return game_map
end

function M.generate_start_map_desc(desc_name)
    load_core_modules()
    local NewMaps = require "maps.NewMaps"
    local MapUtils = require "maps.MapUtils"
    local SourceMap = require "core.SourceMap"
    local Vaults = require "maps.Vaults"

    local desc = require(desc_name)
    local source_map = desc:compile()
    local spawn_selector = {
        matches_none = {
            SourceMap.FLAG_SOLID,
            SourceMap.FLAG_HAS_OBJECT,
            Vaults.FLAG_HAS_VAULT,
        },
    }
    local spawn_points = {}
    for idx = 1, World.player_amount do
        local square = pick_start_spawn_square(source_map, spawn_selector)
        if not square then
            error("Could not find player spawn for --start_map_desc " .. tostring(desc_name) .. ".")
        end
        source_map:square_apply(square, {add = {SourceMap.FLAG_HAS_OBJECT}})
        spawn_points[idx] = MapUtils.from_tile_xy(square)
    end

    local game_map = NewMaps.generate_game_map(source_map)
    for _, post_game_map in ipairs(source_map.post_game_map or {}) do
        post_game_map(game_map)
    end
    World.players_spawn(game_map, spawn_points)
    return game_map
end

function M.apply_player_setup(options)
    load_core_modules()
    options = options or {}
    local entry = World.players[1]
    local player = entry and entry.instance
    if not player then
        return
    end

    local level = math.max(1, math.floor(tonumber(options.level) or 1))
    for _ = 2, level do
        player:gain_level()
    end

    local custom_specs = parse_loadout_specs(options.loadout)
    local loadout_specs = loadout_specs_for_class(options.loadout_preset, custom_specs, options.class_name)
    apply_loadout(player, loadout_specs)

    local estats = player:effective_stats()
    player.stats.hp = estats.max_hp
    player.stats.mp = estats.max_mp
end

function M.loadout_preset_names()
    return Catalog.loadout_preset_names()
end

return M
