local argparse = require "argparse"
local json = require "json"
local EngineInternal = require "core.EngineInternal"
local ResourceLoading = require "engine.ResourceLoading"
local StartEngine = require "engine.StartEngine"
local EngineHooks = require "engine.EngineBase"

local DEFAULT_CLASSES = "Fighter,Templar,Rogue,Necromancer,Pyrocaster,Stormcaller"
local ContentApi
local ContentLaunch
local ContentLabHazards
local GameState
local Map
local World

local function load_core_modules()
    GameState = GameState or require "core.GameState"
    Map = Map or require "core.Map"
    World = World or require "core.World"
end

local function load_content_launch()
    ContentLaunch = ContentLaunch or require "engine.ContentLaunch"
    return ContentLaunch
end

local function load_content_api()
    ContentApi = ContentApi or require "engine.ContentApi"
    return ContentApi
end

local function load_content_lab_hazards()
    ContentLabHazards = ContentLabHazards or require "objects.ContentLabHazards"
    return ContentLabHazards
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

local function content_lab_arena_requested(options)
    return (options.encounter and options.encounter ~= "")
        or (options.monsters and options.monsters ~= "")
end

local function parse_levels(value)
    local result = {}
    for _, part in ipairs(split_csv(value)) do
        local level = tonumber(part)
        if level and level >= 1 then
            result[#result + 1] = math.floor(level)
        end
    end
    if #result == 0 then
        result[#result + 1] = 1
    end
    return result
end

local function group_key(class_name, start_level, loadout, bot_profile)
    return table.concat({
        tostring(class_name or ""),
        tostring(start_level or 1),
        tostring(loadout or ""),
        tostring(bot_profile or ""),
    }, "\n")
end

local function parse_bool(value)
    value = tostring(value or ""):lower()
    return value == "1" or value == "true" or value == "yes" or value == "on"
end

local function normalize_seed_mode(value)
    value = tostring(value or "paired"):lower()
    if value == "paired" or value == "unique" then
        return value
    end
    error("Invalid --seed_mode '" .. tostring(value) .. "'. Expected paired or unique.")
end

local function csv_escape(value)
    if value == nil then
        return ""
    end
    value = tostring(value)
    if value:find('[,"\n]') then
        return '"' .. value:gsub('"', '""') .. '"'
    end
    return value
end

local function sorted_extra_keys(rows, preferred)
    local seen = {}
    local extras = {}
    for _, key in ipairs(preferred) do
        seen[key] = true
    end
    for _, row in ipairs(rows) do
        for key, _ in pairs(row) do
            if not seen[key] then
                seen[key] = true
                extras[#extras + 1] = key
            end
        end
    end
    table.sort(extras)
    local keys = {}
    for _, key in ipairs(preferred) do
        keys[#keys + 1] = key
    end
    for _, key in ipairs(extras) do
        keys[#keys + 1] = key
    end
    return keys
end

local function write_csv(path, rows, preferred_keys)
    if not path or path == "" then
        return
    end
    local keys = sorted_extra_keys(rows, preferred_keys or {})
    local lines = {table.concat(keys, ",")}
    for _, row in ipairs(rows) do
        local values = {}
        for _, key in ipairs(keys) do
            values[#values + 1] = csv_escape(row[key])
        end
        lines[#lines + 1] = table.concat(values, ",")
    end
    file_dump_string(path, table.concat(lines, "\n"))
end

local function encode_counts(counts)
    local parts = {}
    for key, value in pairs(counts or {}) do
        parts[#parts + 1] = tostring(key) .. ":" .. tostring(value)
    end
    table.sort(parts)
    return table.concat(parts, "|")
end

local function decode_counts(value)
    local counts = {}
    for part in tostring(value or ""):gmatch("[^|]+") do
        local key, raw_count = part:match("^(.*):([^:]*)$")
        if key and key ~= "" then
            counts[key] = (counts[key] or 0) + (tonumber(raw_count) or 0)
        end
    end
    return counts
end

local function count_values(counts)
    local total = 0
    for _, value in pairs(counts or {}) do
        total = total + value
    end
    return total
end

local function format_rate(value)
    return string.format("%.3f", value or 0)
end

local function safe_div(num, denom)
    if not denom or denom == 0 then
        return 0
    end
    return (num or 0) / denom
end

local function regen_adjusted_damage(class_name, damage_total, regen_recovered)
    damage_total = damage_total or 0
    if class_name ~= "Necromancer" then
        return damage_total
    end
    return math.max(0, damage_total - (regen_recovered or 0))
end

local function regen_offset_pct(class_name, damage_total, regen_recovered)
    if class_name ~= "Necromancer" then
        return 0
    end
    return math.min(1, safe_div(regen_recovered or 0, damage_total or 0))
end

local function enemy_xpaward(name)
    local enemy_defs = rawget(_G, "enemies")
    local entry = enemy_defs and enemy_defs[name]
    return tonumber(entry and entry.xpaward) or 0
end

local function sorted_group_keys(groups)
    local keys = {}
    for key, _ in pairs(groups) do
        keys[#keys + 1] = key
    end
    table.sort(keys)
    return keys
end

local LOADOUT_PRESETS = {
    none = {},
    gragh = {
        default = {
            "Health Potion*2",
            "Mana Potion",
        },
        Fighter = {
            "Iron Broadsword",
            "Chainmail",
            "Leather Gloves",
            "Iron Helmet",
            "Leather Boots",
            "Belt of Protection",
            "Ring of Vitality",
        },
        Necromancer = {
            "Apprentice Staff",
            "Robe",
            "Wizard's Ring",
            "Amulet of Great Pain",
        },
        Pyrocaster = {
            "Apprentice Staff",
            "Red Robe",
            "Wizard's Hat",
            "Ring of Spells",
            "Amulet of Greater Fire",
        },
        Stormcaller = {
            "Wizard's Staff",
            "White Robe",
            "Wizard's Hat",
            "Ring of Spells",
            "Amulet of Light",
        },
    },
    late = {
        default = {
            "Health Potion*3",
            "Mana Potion*2",
        },
        Fighter = {
            "Gragh's Club",
            "Red Dragonplate",
            "Steel Gloves",
            "Iron Helmet",
            "Platelegs",
            "Jack Boots",
            "Belt of Slaying",
            "Ring of Vampirism",
            "Ring of Vitality",
            "Amulet of Healing",
        },
        Templar = {
            "Mace of Light",
            "Crystalline Plate",
            "Steel Gloves",
            "Iron Helmet",
            "Platelegs",
            "Jack Boots",
            "Belt of Protection",
            "Ring of Stone",
            "Ring of Vitality",
            "Amulet of Fortification",
        },
        Rogue = {
            "Iron Scimitar",
            "Leather Armour",
            "Leather Gloves",
            "Leather Boots",
            "Dank Belt",
            "Ring of Vampirism",
            "Hydra Ring",
            "Amulet of Fear",
        },
        Necromancer = {
            "Runed Robe",
            "Wizard's Ring",
            "Hydra Ring",
            "Moonstone Ring",
            "Wishful Belt",
            "Amulet of Great Pain",
        },
        Pyrocaster = {
            "Gallanthor's Crutch",
            "Red Robe",
            "Runed Gloves",
            "Wizard's Hat",
            "Magic Skirt",
            "Wishful Belt",
            "Magician's Ring",
            "Ring of Spells",
            "Amulet of Greater Fire",
        },
        Stormcaller = {
            "Wizard's Staff",
            "Robe of Vitality",
            "Runed Gloves",
            "Wizard's Hat",
            "Magic Skirt",
            "Wishful Belt",
            "Magician's Ring",
            "Ring of Spells",
            "Amulet of Light",
        },
    },
}

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

local function parse_loadout_specs(value)
    local specs = {}
    for _, part in ipairs(split_csv(value)) do
        local spec = parse_item_spec(part)
        if spec then
            specs[#specs + 1] = spec
        end
    end
    return specs
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

local function normalize_loadout_preset(value)
    local preset = tostring(value or "none")
    if preset == "" then
        preset = "none"
    end
    if not LOADOUT_PRESETS[preset] then
        local names = {}
        for name, _ in pairs(LOADOUT_PRESETS) do
            names[#names + 1] = name
        end
        table.sort(names)
        error("Invalid --loadout_preset '" .. preset .. "'. Expected one of: " .. table.concat(names, ", ") .. ".")
    end
    return preset
end

local function loadout_name(preset, custom_specs, explicit_label)
    if explicit_label and explicit_label ~= "" then
        return explicit_label
    end
    local parts = {}
    if preset and preset ~= "none" then
        parts[#parts + 1] = preset
    end
    if custom_specs and #custom_specs > 0 then
        parts[#parts + 1] = "custom"
    end
    return table.concat(parts, "+")
end

local function loadout_specs_for_class(preset_name, custom_specs, class_name)
    local specs = {}
    local preset = LOADOUT_PRESETS[preset_name or "none"] or {}
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

local function spell_names(player)
    if not player or player.destroyed then
        return ""
    end
    local source = player.input_source
    if source and source.known_spell_names then
        return table.concat(source:known_spell_names(), "|")
    end
    return ""
end

local function hostile_alive_enemy(player, enemy)
    if not enemy or enemy.destroyed or not enemy.stats then
        return false
    end
    if enemy.is_enemy == false then
        return false
    end
    if player and enemy.team == player.team then
        return false
    end
    return true
end

local function visible_enemy_count(player)
    load_core_modules()
    local count = 0
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if hostile_alive_enemy(player, enemy) and Map.object_visible(enemy, enemy.xy, player) then
            count = count + 1
        end
    end
    return count
end

local function effective_max_hp(obj)
    local ok, estats = pcall(function()
        return obj:effective_stats()
    end)
    local max_hp = ok and estats and tonumber(estats.max_hp) or 0
    if max_hp <= 0 and obj.stats then
        max_hp = tonumber(obj.stats.max_hp) or 0
    end
    if max_hp <= 0 and obj.stats then
        max_hp = tonumber(obj.stats.hp) or 0
    end
    return math.max(0, max_hp)
end

local function alive_enemy_state(player, enemies)
    local counts = {}
    local hp = {}
    local max_hp = {}
    for _, enemy in ipairs(enemies or {}) do
        if hostile_alive_enemy(player, enemy) then
            local name = enemy.name or "unknown"
            counts[name] = (counts[name] or 0) + 1
            hp[name] = (hp[name] or 0) + math.max(0, tonumber(enemy.stats.hp) or 0)
            max_hp[name] = (max_hp[name] or 0) + effective_max_hp(enemy)
        end
    end
    return counts, hp, max_hp
end

local function hostile_alive_enemy_count(player)
    load_core_modules()
    local count = 0
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if hostile_alive_enemy(player, enemy) then
            count = count + 1
        end
    end
    return count
end

local function hostile_alive_enemy_named_count(player, name)
    load_core_modules()
    if not name or name == "" then
        return 0
    end
    local count = 0
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if hostile_alive_enemy(player, enemy) and enemy.name == name then
            count = count + 1
        end
    end
    return count
end

local function attach_objective_damage_tracker(player, objective_enemy, telemetry)
    load_core_modules()
    if not player or not objective_enemy or objective_enemy == "" then
        return
    end
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if hostile_alive_enemy(player, enemy) and enemy.name == objective_enemy
            and not enemy:has_effect("BalanceDamageTracker") then
            enemy:add_effect("BalanceDamageTracker", {
                time_left = 10000000,
                telemetry = telemetry,
                damage_by_source_key = "objective_damage_by_source",
                events_by_source_key = "objective_damage_events_by_source",
                damage_total_key = "objective_damage_total",
                event_total_key = "objective_damage_event_total",
            })
        end
    end
end

local function attach_damage_dealt_trackers(player, telemetry)
    load_core_modules()
    if not player then
        return
    end
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if hostile_alive_enemy(player, enemy) and not enemy:has_effect("BalanceDamageDealtTracker") then
            enemy:add_effect("BalanceDamageDealtTracker", {
                time_left = 10000000,
                telemetry = telemetry,
                attacker_team = player.team,
            })
        end
    end
end

local function player_snapshot()
    load_core_modules()
    local entry = World.players[1]
    local player = entry and entry.instance
    if not player then
        return nil
    end
    local estats = player:effective_stats()
    local map_label = player.map and Map.map_label(player.map) or ""
    local enemies = Map.enemies_list(player)
    local alive_enemy_counts, alive_enemy_hp, alive_enemy_max_hp = alive_enemy_state(player, enemies)
    return {
        player = player,
        map_label = map_label,
        enemies_alive = hostile_alive_enemy_count(player),
        visible_enemies = visible_enemy_count(player),
        alive_enemy_counts = encode_counts(alive_enemy_counts),
        alive_enemy_hp = encode_counts(alive_enemy_hp),
        alive_enemy_max_hp = encode_counts(alive_enemy_max_hp),
        hp = player.stats.hp,
        max_hp = estats.max_hp,
        mp = player.stats.mp,
        max_mp = estats.max_mp,
        hp_pct = string.format("%.3f", player.stats.hp / math.max(1, estats.max_hp)),
        mp_pct = string.format("%.3f", player.stats.mp / math.max(1, estats.max_mp)),
        level = player.stats.level,
        xp = player.stats.xp,
        kills = player.kills,
        deaths = player.deaths,
        deepest_floor = player.deepest_floor,
        current_floor = player.current_floor,
        spells_known = spell_names(player),
    }
end

local function content_lab_package_state(player)
    if not player or not player.map then
        return nil
    end
    if player.map.content_lab_package_result then
        return player.map.content_lab_package_result
    end
    if not player.map.content_lab_director then
        return nil
    end
    return load_content_lab_hazards().package_state(player.map)
end

local function package_state_fields(player)
    local state = content_lab_package_state(player)
    if not state then
        return {
            package_outcome = "",
            package_frames = "",
            package_escort_count = "",
            package_escort_hp = "",
            package_escort_max_hp = "",
            package_escort_hp_pct = "",
            package_primary_name = "",
            package_primary_count = "",
            package_primary_hp = "",
            package_primary_max_hp = "",
            package_primary_hp_pct = "",
            package_aggro_target_role = "",
            package_aggro_target_name = "",
            package_aggro_frames = "",
            package_aggro_player_frames = "",
            package_aggro_player_rate = "",
            package_aggro_ally_frames = "",
            package_aggro_ally_rate = "",
            package_aggro_other_frames = "",
            package_aggro_other_rate = "",
            package_aggro_none_frames = "",
            package_aggro_none_rate = "",
            package_aggro_inferred_frames = "",
            package_aggro_inferred_rate = "",
            package_aggro_switches = "",
            package_next_threat_id = "",
            package_next_threat_label = "",
            package_next_threat_state = "",
            package_next_threat_frames = "",
            package_pressure_active_count = "",
            package_pressure_warning_count = "",
            package_pressure_total_count = "",
            package_pressure_max_count = "",
            package_pressure_overlap_frames = "",
            package_pressure_overlap_rate = "",
            package_pressure_sync_frames = "",
            package_pressure_sync_rate = "",
            package_pressure_sync_bursts = "",
            package_pressure_sync_peak = "",
            package_pressure_unique_components = "",
            package_pressure_signature_switches = "",
            package_pressure_variety_frames = "",
            package_pressure_dominant_component = "",
            package_pressure_dominant_component_frames = "",
            package_pressure_dominance_rate = "",
            package_pressure_labels = "",
            package_micro_score = "",
            package_micro_streak = "",
            package_micro_best_streak = "",
            package_micro_hits = "",
            package_micro_dodges = "",
            package_micro_events = "",
            package_micro_thread_cues = "",
            package_micro_streak_bursts = "",
            package_micro_counter_windows = "",
            package_micro_counter_damage_total = "",
            package_micro_counter_stagger_frames = "",
            package_micro_punish_total = "",
            package_micro_punish_events = "",
            package_micro_punish_break_total = "",
            package_micro_punish_break_events = "",
            package_summon_warning_count = "",
            package_summon_warning_remaining = "",
            package_orb_active_count = "",
            package_orb_active_remaining = "",
            package_pulse_warning_count = "",
            package_pulse_warning_remaining = "",
            package_pulse_active_count = "",
            package_pulse_active_remaining = "",
            package_sweep_warning_count = "",
            package_sweep_warning_remaining = "",
            package_sweep_active_count = "",
            package_sweep_active_remaining = "",
            package_rescue_saves = "",
            package_rescue_opportunities = "",
            package_rescue_save_rate = "",
            package_rescue_clutch_saves = "",
            package_rescue_clutch_rate = "",
            package_rescue_perfect_saves = "",
            package_rescue_perfect_rate = "",
            package_rescue_early_saves = "",
            package_rescue_early_rate = "",
            package_rescue_timing_score = "",
            package_rescue_window_cues = "",
            package_rescue_perfect_window_cues = "",
            package_rescue_late_window_frames = "",
            package_rescue_late_uncovered_frames = "",
            package_rescue_late_cover_rate = "",
            package_rescue_perfect_window_frames = "",
            package_rescue_perfect_uncovered_frames = "",
            package_rescue_perfect_cover_rate = "",
            package_rescue_perfect_window = "",
            package_rescue_best_perfect_margin = "",
            package_rescue_margin_total = "",
            package_rescue_clutch_margin_total = "",
            package_rescue_perfect_margin_total = "",
            package_rescue_avg_clutch_margin = "",
            package_rescue_avg_perfect_margin = "",
            package_rescue_pulls = "",
            package_rescue_pull_rate = "",
            package_rescue_clutch_pulls = "",
            package_rescue_perfect_pulls = "",
            package_rescue_timed_pull_rate = "",
            package_rescue_pull_discipline_rate = "",
            package_rescue_perfect_pull_rate = "",
            package_rescue_pull_margin_total = "",
            package_rescue_timed_pull_margin_total = "",
            package_rescue_perfect_pull_margin_total = "",
            package_rescue_avg_pull_margin = "",
            package_rescue_avg_timed_pull_margin = "",
            package_rescue_pull_snap_bursts = "",
            package_rescue_perfect_pull_snap_bursts = "",
            package_rescue_pull_total_frames = "",
            package_rescue_pull_uptime_rate = "",
            package_rescue_taunt_safe_frames = "",
            package_rescue_taunt_strain_frames = "",
            package_rescue_taunt_strain_rate = "",
            package_rescue_taunt_strain_breaks = "",
            package_rescue_taunt_strain_pulses = "",
            package_rescue_counter_windows = "",
            package_rescue_counter_damage_total = "",
            package_rescue_counter_stagger_frames = "",
            package_rescue_misses = "",
            package_rescue_fatal_misses = "",
            package_rescue_survived_misses = "",
            package_rescue_best_margin = "",
            package_rescue_best_pull_margin = "",
            package_rescue_last_margin = "",
            package_rescue_last_pull_frames = "",
            package_rescue_last_pull_margin = "",
            package_rescue_next_frames = "",
            package_rescue_active_count = "",
            package_rescue_active_remaining = "",
            package_rescue_active_blocked = "",
            package_rescue_active_late = "",
            package_rescue_active_perfect = "",
            package_rescue_pull_active_frames = "",
            package_rescue_pull_target = "",
        }
    end
    local escort_hp = tonumber(state.escort_hp) or 0
    local escort_max_hp = tonumber(state.escort_max_hp) or 0
    local primary_hp = tonumber(state.primary_hp) or 0
    local primary_max_hp = tonumber(state.primary_max_hp) or 0
    return {
        package_outcome = state.outcome or "",
        package_frames = state.frames or 0,
        package_escort_count = state.escort_count or 0,
        package_escort_hp = format_rate(escort_hp),
        package_escort_max_hp = format_rate(escort_max_hp),
        package_escort_hp_pct = escort_max_hp > 0 and format_rate(escort_hp / escort_max_hp) or "",
        package_primary_name = state.primary_name or "",
        package_primary_count = state.primary_count or 0,
        package_primary_hp = format_rate(primary_hp),
        package_primary_max_hp = format_rate(primary_max_hp),
        package_primary_hp_pct = primary_max_hp > 0 and format_rate(primary_hp / primary_max_hp) or "",
        package_aggro_target_role = state.aggro_target_role or "",
        package_aggro_target_name = state.aggro_target_name or "",
        package_aggro_frames = state.aggro_frames or 0,
        package_aggro_player_frames = state.aggro_player_frames or 0,
        package_aggro_player_rate = format_rate(state.aggro_player_rate or 0),
        package_aggro_ally_frames = state.aggro_ally_frames or 0,
        package_aggro_ally_rate = format_rate(state.aggro_ally_rate or 0),
        package_aggro_other_frames = state.aggro_other_frames or 0,
        package_aggro_other_rate = format_rate(state.aggro_other_rate or 0),
        package_aggro_none_frames = state.aggro_none_frames or 0,
        package_aggro_none_rate = format_rate(state.aggro_none_rate or 0),
        package_aggro_inferred_frames = state.aggro_inferred_frames or 0,
        package_aggro_inferred_rate = format_rate(state.aggro_inferred_rate or 0),
        package_aggro_switches = state.aggro_switches or 0,
        package_next_threat_id = state.next_threat_id or "",
        package_next_threat_label = state.next_threat_label or "",
        package_next_threat_state = state.next_threat_state or "",
        package_next_threat_frames = state.next_threat_frames or "",
        package_pressure_active_count = state.pressure_active_count or 0,
        package_pressure_warning_count = state.pressure_warning_count or 0,
        package_pressure_total_count = state.pressure_total_count or 0,
        package_pressure_max_count = state.pressure_max_count or 0,
        package_pressure_overlap_frames = state.pressure_overlap_frames or 0,
        package_pressure_overlap_rate = format_rate(state.pressure_overlap_rate or 0),
        package_pressure_sync_frames = state.pressure_sync_frames or 0,
        package_pressure_sync_rate = format_rate(state.pressure_sync_rate or 0),
        package_pressure_sync_bursts = state.pressure_sync_bursts or 0,
        package_pressure_sync_peak = state.pressure_sync_peak or 0,
        package_pressure_unique_components = state.pressure_unique_components or 0,
        package_pressure_signature_switches = state.pressure_signature_switches or 0,
        package_pressure_variety_frames = state.pressure_variety_frames or 0,
        package_pressure_dominant_component = state.pressure_dominant_component or "",
        package_pressure_dominant_component_frames = state.pressure_dominant_component_frames or 0,
        package_pressure_dominance_rate = format_rate(state.pressure_dominance_rate or 0),
        package_pressure_labels = state.pressure_labels or "",
        package_micro_score = state.micro_score or 0,
        package_micro_streak = state.micro_streak or 0,
        package_micro_best_streak = state.micro_best_streak or 0,
        package_micro_hits = state.micro_hits or 0,
        package_micro_dodges = state.micro_dodges or 0,
        package_micro_events = state.micro_events or 0,
        package_micro_thread_cues = state.micro_thread_cues or 0,
        package_micro_streak_bursts = state.micro_streak_bursts or 0,
        package_micro_counter_windows = state.micro_counter_windows or 0,
        package_micro_counter_damage_total = format_rate(state.micro_counter_damage_total or 0),
        package_micro_counter_stagger_frames = state.micro_counter_stagger_frames or 0,
        package_micro_punish_total = format_rate(state.micro_punish_total or 0),
        package_micro_punish_events = state.micro_punish_events or 0,
        package_micro_punish_break_total = format_rate(state.micro_punish_break_total or 0),
        package_micro_punish_break_events = state.micro_punish_break_events or 0,
        package_summon_warning_count = state.summon_warning_count or 0,
        package_summon_warning_remaining = state.summon_warning_remaining or "",
        package_orb_active_count = state.orb_active_count or 0,
        package_orb_active_remaining = state.orb_active_remaining or "",
        package_pulse_warning_count = state.pulse_warning_count or 0,
        package_pulse_warning_remaining = state.pulse_warning_remaining or "",
        package_pulse_active_count = state.pulse_active_count or 0,
        package_pulse_active_remaining = state.pulse_active_remaining or "",
        package_sweep_warning_count = state.sweep_warning_count or 0,
        package_sweep_warning_remaining = state.sweep_warning_remaining or "",
        package_sweep_active_count = state.sweep_active_count or 0,
        package_sweep_active_remaining = state.sweep_active_remaining or "",
        package_rescue_saves = state.rescue_saves or 0,
        package_rescue_opportunities = state.rescue_opportunities or ((state.rescue_saves or 0) + (state.rescue_misses or 0)),
        package_rescue_save_rate = format_rate(state.rescue_save_rate or 0),
        package_rescue_clutch_saves = state.rescue_clutch_saves or 0,
        package_rescue_clutch_rate = format_rate(state.rescue_clutch_rate or 0),
        package_rescue_perfect_saves = state.rescue_perfect_saves or 0,
        package_rescue_perfect_rate = format_rate(state.rescue_perfect_rate or 0),
        package_rescue_early_saves = state.rescue_early_saves or 0,
        package_rescue_early_rate = format_rate(state.rescue_early_rate or 0),
        package_rescue_timing_score = format_rate(state.rescue_timing_score or 0),
        package_rescue_window_cues = state.rescue_window_cues or 0,
        package_rescue_perfect_window_cues = state.rescue_perfect_window_cues or 0,
        package_rescue_late_window_frames = state.rescue_late_window_frames or 0,
        package_rescue_late_uncovered_frames = state.rescue_late_uncovered_frames or 0,
        package_rescue_late_cover_rate = format_rate(state.rescue_late_cover_rate or 0),
        package_rescue_perfect_window_frames = state.rescue_perfect_window_frames or 0,
        package_rescue_perfect_uncovered_frames = state.rescue_perfect_uncovered_frames or 0,
        package_rescue_perfect_cover_rate = format_rate(state.rescue_perfect_cover_rate or 0),
        package_rescue_perfect_window = state.rescue_perfect_window or "",
        package_rescue_best_perfect_margin = state.rescue_best_perfect_margin or "",
        package_rescue_margin_total = format_rate(state.rescue_margin_total or 0),
        package_rescue_clutch_margin_total = format_rate(state.rescue_clutch_margin_total or 0),
        package_rescue_perfect_margin_total = format_rate(state.rescue_perfect_margin_total or 0),
        package_rescue_avg_clutch_margin = format_rate(state.rescue_avg_clutch_margin or 0),
        package_rescue_avg_perfect_margin = format_rate(state.rescue_avg_perfect_margin or 0),
        package_rescue_pulls = state.rescue_pulls or 0,
        package_rescue_pull_rate = format_rate(state.rescue_pull_rate or 0),
        package_rescue_clutch_pulls = state.rescue_clutch_pulls or 0,
        package_rescue_perfect_pulls = state.rescue_perfect_pulls or 0,
        package_rescue_timed_pull_rate = format_rate(state.rescue_timed_pull_rate or 0),
        package_rescue_pull_discipline_rate = format_rate(state.rescue_pull_discipline_rate or 0),
        package_rescue_perfect_pull_rate = format_rate(state.rescue_perfect_pull_rate or 0),
        package_rescue_pull_margin_total = format_rate(state.rescue_pull_margin_total or 0),
        package_rescue_timed_pull_margin_total = format_rate(state.rescue_timed_pull_margin_total or 0),
        package_rescue_perfect_pull_margin_total = format_rate(state.rescue_perfect_pull_margin_total or 0),
        package_rescue_avg_pull_margin = format_rate(state.rescue_avg_pull_margin or 0),
        package_rescue_avg_timed_pull_margin = format_rate(state.rescue_avg_timed_pull_margin or 0),
        package_rescue_pull_snap_bursts = state.rescue_pull_snap_bursts or 0,
        package_rescue_perfect_pull_snap_bursts = state.rescue_perfect_pull_snap_bursts or 0,
        package_rescue_pull_total_frames = state.rescue_pull_total_frames or 0,
        package_rescue_pull_uptime_rate = format_rate(state.rescue_pull_uptime_rate or 0),
        package_rescue_taunt_safe_frames = state.rescue_taunt_safe_frames or 0,
        package_rescue_taunt_strain_frames = state.rescue_taunt_strain_frames or 0,
        package_rescue_taunt_strain_rate = format_rate(state.rescue_taunt_strain_rate or 0),
        package_rescue_taunt_strain_breaks = state.rescue_taunt_strain_breaks or 0,
        package_rescue_taunt_strain_pulses = state.rescue_taunt_strain_pulses or 0,
        package_rescue_counter_windows = state.rescue_counter_windows or 0,
        package_rescue_counter_damage_total = format_rate(state.rescue_counter_damage_total or 0),
        package_rescue_counter_stagger_frames = state.rescue_counter_stagger_frames or 0,
        package_rescue_misses = state.rescue_misses or 0,
        package_rescue_fatal_misses = state.rescue_fatal_misses or 0,
        package_rescue_survived_misses = state.rescue_survived_misses or 0,
        package_rescue_best_margin = state.rescue_best_margin or "",
        package_rescue_best_pull_margin = state.rescue_best_pull_margin or "",
        package_rescue_last_margin = state.rescue_last_margin or "",
        package_rescue_last_pull_frames = state.rescue_last_pull_frames or "",
        package_rescue_last_pull_margin = state.rescue_last_pull_margin or "",
        package_rescue_next_frames = state.rescue_next_frames or "",
        package_rescue_active_count = state.rescue_active_count or 0,
        package_rescue_active_remaining = state.rescue_active_remaining or "",
        package_rescue_active_blocked = state.rescue_active_blocked and "true" or "false",
        package_rescue_active_late = state.rescue_active_late and "true" or "false",
        package_rescue_active_perfect = state.rescue_active_perfect and "true" or "false",
        package_rescue_pull_active_frames = state.rescue_pull_active_frames or 0,
        package_rescue_pull_target = state.rescue_pull_target or "",
    }
end

local function copy_package_state_fields(row, player)
    for key, value in pairs(package_state_fields(player)) do
        row[key] = value
    end
end

local DICEBINDER_METRICS = {
    "dice_rolls_total",
    "dice_roll_1_count",
    "dice_roll_2_count",
    "dice_roll_3_count",
    "dice_roll_4_count",
    "dice_roll_5_count",
    "dice_roll_6_count",
    "dice_dash_refreshes",
    "dice_dash_effective_refreshes",
    "dice_power_refreshes",
    "dice_power_effective_refreshes",
    "dice_jackpot_rolls",
    "dice_roll_animations_total",
    "dice_roll_animation_frames",
    "dice_roll_buildup_fx",
    "dice_roll_face_swaps",
    "dice_roll_reveal_cues",
    "dice_roll_lock_cues",
    "dice_roll_surge_cues",
    "dice_roll_finale_fx",
    "dice_roll_suspense_pips",
    "dice_roll_guard_cues",
    "dice_cooldown_trim_frames",
    "dice_dash_trim_effective",
    "dice_power_trim_effective",
    "dice_bag_refills",
    "dice_anti_repeat_rerolls",
    "dice_high_rolls",
    "dice_utility_rolls",
    "dice_longest_high_streak",
    "dice_longest_utility_streak",
    "dice_high_streak_breaks",
    "dice_low_streak_breaks",
    "dice_repeat_rolls",
    "dice_roll_variety_switches",
    "dice_buff_active_frames",
    "dice_fleet_frames",
    "dice_guard_frames",
    "dice_edge_frames",
    "dice_focus_frames",
    "dice_dash_frames",
    "dice_jackpot_frames",
}

local function dicebinder_state_fields(player)
    local fields = {
        dice_last_roll = "",
        dice_next_roll_frames = "",
        dice_pending_roll = "",
        dice_rolling_frames = "",
    }
    for _, key in ipairs(DICEBINDER_METRICS) do
        fields[key] = 0
    end
    if not player or player.class_name ~= "Dicebinder" or not player:has_effect("Dicebinder") then
        return fields
    end
    local effect = player:get_effect("Dicebinder") or {}
    fields.dice_last_roll = effect.last_roll or 0
    fields.dice_next_roll_frames = effect.next_roll or 0
    fields.dice_pending_roll = effect.pending_roll or ""
    fields.dice_rolling_frames = effect.rolling_time or 0
    fields.dice_rolls_total = effect.rolls_total or 0
    for roll = 1, 6 do
        local key = "dice_roll_" .. tostring(roll) .. "_count"
        fields[key] = effect["roll_" .. tostring(roll) .. "_count"] or 0
    end
    fields.dice_dash_refreshes = effect.dash_refreshes or 0
    fields.dice_dash_effective_refreshes = effect.dash_effective_refreshes or 0
    fields.dice_power_refreshes = effect.power_refreshes or 0
    fields.dice_power_effective_refreshes = effect.power_effective_refreshes or 0
    fields.dice_jackpot_rolls = effect.jackpot_rolls or 0
    fields.dice_roll_animations_total = effect.roll_animations_total or 0
    fields.dice_roll_animation_frames = effect.roll_animation_frames or 0
    fields.dice_roll_buildup_fx = effect.roll_buildup_fx or 0
    fields.dice_roll_face_swaps = effect.roll_face_swaps or 0
    fields.dice_roll_reveal_cues = effect.roll_reveal_cues or 0
    fields.dice_roll_lock_cues = effect.roll_lock_cues or 0
    fields.dice_roll_surge_cues = effect.roll_surge_cues or 0
    fields.dice_roll_finale_fx = effect.roll_finale_fx or 0
    fields.dice_roll_suspense_pips = effect.roll_suspense_pips or 0
    fields.dice_roll_guard_cues = effect.roll_guard_cues or 0
    fields.dice_cooldown_trim_frames = effect.cooldown_trim_frames or 0
    fields.dice_dash_trim_effective = effect.dash_trim_effective or 0
    fields.dice_power_trim_effective = effect.power_trim_effective or 0
    fields.dice_bag_refills = effect.bag_refills or 0
    fields.dice_anti_repeat_rerolls = effect.anti_repeat_rerolls or 0
    fields.dice_high_rolls = effect.high_rolls or 0
    fields.dice_utility_rolls = effect.utility_rolls or 0
    fields.dice_longest_high_streak = effect.longest_high_streak or 0
    fields.dice_longest_utility_streak = effect.longest_utility_streak or 0
    fields.dice_high_streak_breaks = effect.high_streak_breaks or 0
    fields.dice_low_streak_breaks = effect.low_streak_breaks or 0
    fields.dice_repeat_rolls = effect.repeat_rolls or 0
    fields.dice_roll_variety_switches = effect.roll_variety_switches or 0
    fields.dice_buff_active_frames = effect.buff_active_frames or 0
    fields.dice_fleet_frames = effect.fleet_frames or 0
    fields.dice_guard_frames = effect.guard_frames or 0
    fields.dice_edge_frames = effect.edge_frames or 0
    fields.dice_focus_frames = effect.focus_frames or 0
    fields.dice_dash_frames = effect.dash_frames or 0
    fields.dice_jackpot_frames = effect.jackpot_frames or 0
    return fields
end

local function copy_dicebinder_state_fields(row, player)
    for key, value in pairs(dicebinder_state_fields(player)) do
        row[key] = value
    end
end

local function sample_run(run, telemetry, samples, reason)
    local snap = player_snapshot()
    if not snap then
        return
    end
    local spell_orders_total = count_values(telemetry.spell_orders)
    local hostile_kills = count_values(telemetry.monster_kills)
    local necromancer_pain_orders = telemetry.necromancer_pain_orders or 0
    local damage_taken_total = telemetry.damage_taken_total or 0
    local regen_recovered_total = telemetry.necromancer_regen_recovered_total or 0
    local row = {
        run_id = run.run_id,
        seed = tostring(run.seed),
        seed_group = run.seed_group,
        class_run_index = run.class_run_index,
        class = run.class_name,
        start_level = run.start_level or 1,
        loadout = run.loadout_name or "",
        bot_profile = run.bot_profile or telemetry.skill_profile or "",
        objective_enemy = run.objective_enemy or "",
        start_map_desc = run.start_map_desc or "",
        map_profile = run.map_profile or "",
        encounter = run.encounter or "",
        monsters = run.monsters or "",
        package_intensity = run.package_intensity or "",
        package_components = run.package_components or "",
        frame = GameState.frame,
        reason = reason or "interval",
        level = snap.level,
        xp = snap.xp,
        hp = snap.hp,
        max_hp = snap.max_hp,
        hp_pct = snap.hp_pct,
        mp = snap.mp,
        max_mp = snap.max_mp,
        mp_pct = snap.mp_pct,
        kills = hostile_kills,
        player_kills = snap.kills,
        deaths = snap.deaths,
        deepest_floor = snap.deepest_floor,
        current_floor = snap.current_floor,
        map_label = snap.map_label,
        enemies_alive = snap.enemies_alive,
        visible_enemies = snap.visible_enemies,
        alive_enemy_counts = snap.alive_enemy_counts,
        alive_enemy_hp = snap.alive_enemy_hp,
        alive_enemy_max_hp = snap.alive_enemy_max_hp,
        spells_known = snap.spells_known,
        spell_orders_total = spell_orders_total,
        ability_orders_total = spell_orders_total + necromancer_pain_orders,
        weapon_orders = telemetry.weapon_orders or 0,
        item_orders_total = count_values(telemetry.item_orders),
        damage_taken_total = format_rate(damage_taken_total),
        regen_adjusted_damage_taken_total = format_rate(regen_adjusted_damage(run.class_name, damage_taken_total, regen_recovered_total)),
        necromancer_regen_offset_pct = format_rate(regen_offset_pct(run.class_name, damage_taken_total, regen_recovered_total)),
        damage_event_total = telemetry.damage_event_total or 0,
        damage_taken_by_source = encode_counts(telemetry.damage_taken_by_source),
        damage_events_by_source = encode_counts(telemetry.damage_events_by_source),
        damage_dealt_total = format_rate(telemetry.damage_dealt_total or 0),
        damage_dealt_event_total = telemetry.damage_dealt_event_total or 0,
        damage_dealt_by_target = encode_counts(telemetry.damage_dealt_by_target),
        damage_dealt_events_by_target = encode_counts(telemetry.damage_dealt_events_by_target),
        damage_dealt_by_source = encode_counts(telemetry.damage_dealt_by_source),
        damage_dealt_events_by_source = encode_counts(telemetry.damage_dealt_events_by_source),
        objective_damage_total = format_rate(telemetry.objective_damage_total or 0),
        objective_damage_event_total = telemetry.objective_damage_event_total or 0,
        objective_damage_by_source = encode_counts(telemetry.objective_damage_by_source),
        objective_damage_events_by_source = encode_counts(telemetry.objective_damage_events_by_source),
        hunt_enemy_frames = telemetry.hunt_enemy_frames or 0,
        ranged_keepaway_frames = telemetry.ranged_keepaway_frames or 0,
        ranged_close_enemy_frames = telemetry.ranged_close_enemy_frames or 0,
        ranged_close_gragh_frames = telemetry.ranged_close_gragh_frames or 0,
        rescue_intercept_frames = telemetry.rescue_intercept_frames or 0,
        rescue_hold_frames = telemetry.rescue_hold_frames or 0,
        rescue_late_reaction_frames = telemetry.rescue_late_reaction_frames or 0,
        objective_hunt_enemy_frames = telemetry.objective_hunt_enemy_frames or 0,
        necromancer_close_frames = telemetry.necromancer_close_frames or 0,
        necromancer_pain_orders = necromancer_pain_orders,
        necromancer_pain_during_regen_orders = telemetry.necromancer_pain_during_regen_orders or 0,
        necromancer_regen_combat_frames = telemetry.necromancer_regen_combat_frames or 0,
        necromancer_retreat_frames = telemetry.necromancer_retreat_frames or 0,
        pyrocaster_ring_combat_frames = telemetry.pyrocaster_ring_combat_frames or 0,
        pyrocaster_ring_close_frames = telemetry.pyrocaster_ring_close_frames or 0,
        pyrocaster_ring_reposition_frames = telemetry.pyrocaster_ring_reposition_frames or 0,
    }
    copy_package_state_fields(row, snap.player)
    copy_dicebinder_state_fields(row, snap.player)
    samples[#samples + 1] = row
end

local function summarize_run(run, telemetry, outcome)
    local snap = player_snapshot() or {}
    local spell_orders_total = count_values(telemetry.spell_orders)
    local hostile_kills = count_values(telemetry.monster_kills)
    local necromancer_pain_orders = telemetry.necromancer_pain_orders or 0
    local damage_taken_total = telemetry.damage_taken_total or 0
    local regen_recovered_total = telemetry.necromancer_regen_recovered_total or 0
    local row = {
        run_id = run.run_id,
        seed = tostring(run.seed),
        seed_group = run.seed_group,
        class_run_index = run.class_run_index,
        class = run.class_name,
        start_level = run.start_level or 1,
        loadout = run.loadout_name or "",
        bot_profile = run.bot_profile or telemetry.skill_profile or "",
        objective_enemy = run.objective_enemy or "",
        start_map_desc = run.start_map_desc or "",
        map_profile = run.map_profile or "",
        encounter = run.encounter or "",
        monsters = run.monsters or "",
        package_intensity = run.package_intensity or "",
        package_components = run.package_components or "",
        outcome = outcome,
        frames = GameState.frame,
        level = snap.level or 0,
        xp = snap.xp or 0,
        hp = snap.hp or 0,
        max_hp = snap.max_hp or 0,
        hp_pct = snap.hp_pct or "0.000",
        mp = snap.mp or 0,
        max_mp = snap.max_mp or 0,
        mp_pct = snap.mp_pct or "0.000",
        kills = hostile_kills,
        player_kills = snap.kills or 0,
        deaths = snap.deaths or 0,
        min_hp = telemetry.min_hp or snap.hp or 0,
        min_hp_pct = format_rate(telemetry.min_hp_pct or tonumber(snap.hp_pct) or 0),
        deepest_floor = snap.deepest_floor or 0,
        current_floor = snap.current_floor or 0,
        map_label = snap.map_label or "",
        enemies_alive = snap.enemies_alive or 0,
        visible_enemies = snap.visible_enemies or 0,
        alive_enemy_counts = snap.alive_enemy_counts or "",
        alive_enemy_hp = snap.alive_enemy_hp or "",
        alive_enemy_max_hp = snap.alive_enemy_max_hp or "",
        spells_known = snap.spells_known or "",
        spell_orders_total = spell_orders_total,
        ability_orders_total = spell_orders_total + necromancer_pain_orders,
        weapon_orders = telemetry.weapon_orders or 0,
        item_orders_total = count_values(telemetry.item_orders),
        damage_taken_total = format_rate(damage_taken_total),
        regen_adjusted_damage_taken_total = format_rate(regen_adjusted_damage(run.class_name, damage_taken_total, regen_recovered_total)),
        necromancer_regen_offset_pct = format_rate(regen_offset_pct(run.class_name, damage_taken_total, regen_recovered_total)),
        damage_event_total = telemetry.damage_event_total or 0,
        damage_taken_by_source = encode_counts(telemetry.damage_taken_by_source),
        damage_events_by_source = encode_counts(telemetry.damage_events_by_source),
        damage_dealt_total = format_rate(telemetry.damage_dealt_total or 0),
        damage_dealt_event_total = telemetry.damage_dealt_event_total or 0,
        damage_dealt_by_target = encode_counts(telemetry.damage_dealt_by_target),
        damage_dealt_events_by_target = encode_counts(telemetry.damage_dealt_events_by_target),
        damage_dealt_by_source = encode_counts(telemetry.damage_dealt_by_source),
        damage_dealt_events_by_source = encode_counts(telemetry.damage_dealt_events_by_source),
        objective_damage_total = format_rate(telemetry.objective_damage_total or 0),
        objective_damage_event_total = telemetry.objective_damage_event_total or 0,
        objective_damage_by_source = encode_counts(telemetry.objective_damage_by_source),
        objective_damage_events_by_source = encode_counts(telemetry.objective_damage_events_by_source),
        reaction_hold_frames = telemetry.reaction_hold_frames or 0,
        movement_noise_frames = telemetry.movement_noise_frames or 0,
        spell_skip_frames = telemetry.spell_skip_frames or 0,
        item_skip_frames = telemetry.item_skip_frames or 0,
        weapon_skip_frames = telemetry.weapon_skip_frames or 0,
        controlled_frames = telemetry.controlled_frames or 0,
        hunt_enemy_frames = telemetry.hunt_enemy_frames or 0,
        ranged_keepaway_frames = telemetry.ranged_keepaway_frames or 0,
        ranged_close_enemy_frames = telemetry.ranged_close_enemy_frames or 0,
        ranged_close_gragh_frames = telemetry.ranged_close_gragh_frames or 0,
        rescue_intercept_frames = telemetry.rescue_intercept_frames or 0,
        rescue_hold_frames = telemetry.rescue_hold_frames or 0,
        rescue_late_reaction_frames = telemetry.rescue_late_reaction_frames or 0,
        objective_hunt_enemy_frames = telemetry.objective_hunt_enemy_frames or 0,
        combat_frames = telemetry.combat_frames or 0,
        explore_frames = telemetry.explore_frames or 0,
        rest_frames = telemetry.rest_frames or 0,
        idle_frames = telemetry.idle_frames or 0,
        random_walk_frames = telemetry.random_walk_frames or 0,
        unstick_frames = telemetry.unstick_frames or 0,
        portal_stop_frames = telemetry.portal_stop_frames or 0,
        necromancer_close_frames = telemetry.necromancer_close_frames or 0,
        necromancer_pain_orders = necromancer_pain_orders,
        necromancer_pain_during_regen_orders = telemetry.necromancer_pain_during_regen_orders or 0,
        necromancer_regen_frames = telemetry.necromancer_regen_frames or 0,
        necromancer_regen_recovered_total = format_rate(regen_recovered_total),
        necromancer_regen_combat_frames = telemetry.necromancer_regen_combat_frames or 0,
        necromancer_retreat_frames = telemetry.necromancer_retreat_frames or 0,
        pyrocaster_ring_combat_frames = telemetry.pyrocaster_ring_combat_frames or 0,
        pyrocaster_ring_close_frames = telemetry.pyrocaster_ring_close_frames or 0,
        pyrocaster_ring_reposition_frames = telemetry.pyrocaster_ring_reposition_frames or 0,
        max_same_square_frames = telemetry.max_same_square_frames or 0,
        max_visible_enemies = telemetry.max_visible_enemies or 0,
        spell_orders = encode_counts(telemetry.spell_orders),
        item_orders = encode_counts(telemetry.item_orders),
        monster_kills = encode_counts(telemetry.monster_kills),
        visible_enemy_frames = encode_counts(telemetry.visible_enemy_frames),
    }
    copy_package_state_fields(row, snap.player)
    copy_dicebinder_state_fields(row, snap.player)
    return row
end

local function flatten_spell_rows(run, telemetry)
    local rows = {}
    for spell, count in pairs(telemetry.spell_orders or {}) do
        rows[#rows + 1] = {
            run_id = run.run_id,
            seed = tostring(run.seed),
            seed_group = run.seed_group,
            class_run_index = run.class_run_index,
            class = run.class_name,
            start_level = run.start_level or 1,
            loadout = run.loadout_name or "",
            bot_profile = run.bot_profile or telemetry.skill_profile or "",
            spell = spell,
            orders = count,
        }
    end
    return rows
end

local function flatten_damage_rows(run, telemetry, damage_by_source_key, events_by_source_key)
    local rows = {}
    local events = telemetry[events_by_source_key or "damage_events_by_source"] or {}
    for source, damage in pairs(telemetry[damage_by_source_key or "damage_taken_by_source"] or {}) do
        rows[#rows + 1] = {
            run_id = run.run_id,
            seed = tostring(run.seed),
            seed_group = run.seed_group,
            class_run_index = run.class_run_index,
            class = run.class_name,
            start_level = run.start_level or 1,
            loadout = run.loadout_name or "",
            bot_profile = run.bot_profile or telemetry.skill_profile or "",
            source = source,
            damage = format_rate(damage),
            events = events[source] or 0,
        }
    end
    return rows
end

local function flatten_damage_dealt_target_rows(run, telemetry)
    local rows = {}
    local events = telemetry.damage_dealt_events_by_target or {}
    for target, damage in pairs(telemetry.damage_dealt_by_target or {}) do
        rows[#rows + 1] = {
            run_id = run.run_id,
            seed = tostring(run.seed),
            seed_group = run.seed_group,
            class_run_index = run.class_run_index,
            class = run.class_name,
            start_level = run.start_level or 1,
            loadout = run.loadout_name or "",
            bot_profile = run.bot_profile or telemetry.skill_profile or "",
            target = target,
            damage = format_rate(damage),
            events = events[target] or 0,
        }
    end
    return rows
end

local SUMMARY_METRICS = {
    "frames", "level", "xp", "hp_pct", "mp_pct", "kills", "player_kills", "deaths",
    "min_hp", "min_hp_pct", "deepest_floor", "current_floor", "spell_orders_total", "ability_orders_total", "weapon_orders",
    "item_orders_total", "damage_taken_total", "regen_adjusted_damage_taken_total",
    "necromancer_regen_offset_pct", "damage_event_total",
    "damage_dealt_total", "damage_dealt_event_total",
    "objective_damage_total", "objective_damage_event_total",
    "reaction_hold_frames", "movement_noise_frames", "spell_skip_frames",
    "item_skip_frames", "weapon_skip_frames",
    "controlled_frames", "hunt_enemy_frames", "combat_frames",
    "explore_frames", "rest_frames", "idle_frames", "random_walk_frames",
    "unstick_frames", "portal_stop_frames", "ranged_keepaway_frames",
    "ranged_close_enemy_frames", "ranged_close_gragh_frames", "rescue_intercept_frames",
    "rescue_hold_frames", "rescue_late_reaction_frames", "objective_hunt_enemy_frames", "necromancer_close_frames",
    "necromancer_pain_orders", "necromancer_pain_during_regen_orders",
    "necromancer_regen_frames", "necromancer_regen_recovered_total",
    "necromancer_regen_combat_frames", "necromancer_retreat_frames",
    "pyrocaster_ring_combat_frames", "pyrocaster_ring_close_frames",
    "pyrocaster_ring_reposition_frames",
    "max_same_square_frames", "max_visible_enemies",
    "dice_rolls_total", "dice_roll_1_count", "dice_roll_2_count",
    "dice_roll_3_count", "dice_roll_4_count", "dice_roll_5_count",
    "dice_roll_6_count", "dice_dash_refreshes", "dice_dash_effective_refreshes",
    "dice_power_refreshes", "dice_power_effective_refreshes", "dice_jackpot_rolls",
    "dice_roll_animations_total", "dice_roll_animation_frames", "dice_roll_buildup_fx",
    "dice_roll_face_swaps", "dice_roll_reveal_cues", "dice_roll_lock_cues",
    "dice_roll_surge_cues", "dice_roll_finale_fx", "dice_roll_suspense_pips",
    "dice_roll_guard_cues",
    "dice_cooldown_trim_frames", "dice_dash_trim_effective", "dice_power_trim_effective",
    "dice_bag_refills", "dice_anti_repeat_rerolls", "dice_high_rolls", "dice_utility_rolls",
    "dice_longest_high_streak", "dice_longest_utility_streak",
    "dice_high_streak_breaks", "dice_low_streak_breaks",
    "dice_repeat_rolls", "dice_roll_variety_switches",
    "dice_buff_active_frames", "dice_fleet_frames", "dice_guard_frames",
    "dice_edge_frames", "dice_focus_frames", "dice_dash_frames", "dice_jackpot_frames",
    "package_frames", "package_escort_count", "package_escort_hp", "package_escort_max_hp",
    "package_escort_hp_pct", "package_rescue_saves", "package_rescue_clutch_saves",
    "package_aggro_frames", "package_aggro_player_frames", "package_aggro_player_rate",
    "package_aggro_ally_frames", "package_aggro_ally_rate", "package_aggro_other_frames",
    "package_aggro_other_rate", "package_aggro_none_frames", "package_aggro_none_rate",
    "package_aggro_inferred_frames", "package_aggro_inferred_rate", "package_aggro_switches",
    "package_rescue_perfect_saves", "package_rescue_pulls", "package_rescue_misses",
    "package_rescue_fatal_misses", "package_rescue_survived_misses",
    "package_rescue_opportunities", "package_rescue_save_rate", "package_rescue_clutch_rate",
    "package_rescue_perfect_rate", "package_rescue_early_saves", "package_rescue_early_rate",
    "package_rescue_timing_score", "package_rescue_window_cues", "package_rescue_perfect_window_cues",
    "package_rescue_late_window_frames", "package_rescue_late_uncovered_frames",
    "package_rescue_late_cover_rate", "package_rescue_perfect_window_frames",
    "package_rescue_perfect_uncovered_frames", "package_rescue_perfect_cover_rate",
    "package_rescue_margin_total", "package_rescue_clutch_margin_total",
    "package_rescue_perfect_margin_total", "package_rescue_avg_clutch_margin",
    "package_rescue_avg_perfect_margin",
    "package_rescue_pull_rate",
    "package_rescue_clutch_pulls", "package_rescue_perfect_pulls",
    "package_rescue_timed_pull_rate", "package_rescue_pull_discipline_rate", "package_rescue_perfect_pull_rate",
    "package_rescue_pull_margin_total", "package_rescue_timed_pull_margin_total",
    "package_rescue_perfect_pull_margin_total", "package_rescue_avg_pull_margin",
    "package_rescue_avg_timed_pull_margin",
    "package_rescue_pull_snap_bursts", "package_rescue_perfect_pull_snap_bursts",
    "package_rescue_pull_total_frames", "package_rescue_pull_uptime_rate",
    "package_rescue_taunt_safe_frames", "package_rescue_taunt_strain_frames",
    "package_rescue_taunt_strain_rate", "package_rescue_taunt_strain_breaks",
    "package_rescue_taunt_strain_pulses",
    "package_rescue_counter_windows", "package_rescue_counter_damage_total",
    "package_rescue_counter_stagger_frames",
    "package_pressure_active_count", "package_pressure_warning_count", "package_pressure_total_count",
    "package_pressure_max_count", "package_pressure_overlap_frames", "package_pressure_overlap_rate",
    "package_pressure_sync_frames", "package_pressure_sync_rate", "package_pressure_sync_bursts",
    "package_pressure_sync_peak", "package_pressure_unique_components", "package_pressure_signature_switches",
    "package_pressure_variety_frames", "package_pressure_dominant_component_frames",
    "package_pressure_dominance_rate",
    "package_micro_score", "package_micro_streak", "package_micro_best_streak",
    "package_micro_hits", "package_micro_dodges", "package_micro_events",
    "package_micro_thread_cues", "package_micro_streak_bursts",
    "package_micro_counter_windows", "package_micro_counter_damage_total",
    "package_micro_counter_stagger_frames",
    "package_micro_punish_total", "package_micro_punish_events",
    "package_micro_punish_break_total", "package_micro_punish_break_events",
    "package_sweep_warning_count", "package_sweep_active_count",
}

local function add_summary_metric(aggregate, row, key)
    local value = tonumber(row[key]) or 0
    aggregate.sums[key] = (aggregate.sums[key] or 0) + value
    if aggregate.mins[key] == nil or value < aggregate.mins[key] then
        aggregate.mins[key] = value
    end
    if aggregate.maxes[key] == nil or value > aggregate.maxes[key] then
        aggregate.maxes[key] = value
    end
end

local function aggregate_summary_rows(summaries)
    local groups = {}
    for _, row in ipairs(summaries) do
        local class_name = row.class or ""
        local start_level = tonumber(row.start_level) or 1
        local loadout = row.loadout or ""
        local bot_profile = row.bot_profile or ""
        local objective_enemy = row.objective_enemy or ""
        local start_map_desc = row.start_map_desc or ""
        local map_profile = row.map_profile or ""
        local encounter = row.encounter or ""
        local monsters = row.monsters or ""
        local package_intensity = row.package_intensity or ""
        local package_components = row.package_components or ""
        local key = group_key(class_name, start_level, loadout, bot_profile)
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = class_name,
                start_level = start_level,
                loadout = loadout,
                bot_profile = bot_profile,
                objective_enemy = objective_enemy,
                start_map_desc = start_map_desc,
                map_profile = map_profile,
                encounter = encounter,
                monsters = monsters,
                package_intensity = package_intensity,
                package_components = package_components,
                runs = 0,
                outcomes = {},
                sums = {},
                mins = {},
                maxes = {},
            }
            groups[key] = aggregate
        end

        aggregate.runs = aggregate.runs + 1
        aggregate.outcomes[row.outcome or "unknown"] = (aggregate.outcomes[row.outcome or "unknown"] or 0) + 1
        for _, key in ipairs(SUMMARY_METRICS) do
            add_summary_metric(aggregate, row, key)
        end
    end

    local rows = {}
    for _, key in ipairs(sorted_group_keys(groups)) do
        local aggregate = groups[key]
        local runs = math.max(1, aggregate.runs)
        local wins = (aggregate.outcomes.won or 0) + (aggregate.outcomes.cleared or 0)
        local row = {
            class = aggregate.class,
            start_level = aggregate.start_level,
            loadout = aggregate.loadout,
            bot_profile = aggregate.bot_profile,
            objective_enemy = aggregate.objective_enemy,
            start_map_desc = aggregate.start_map_desc,
            map_profile = aggregate.map_profile,
            encounter = aggregate.encounter,
            monsters = aggregate.monsters,
            package_intensity = aggregate.package_intensity,
            package_components = aggregate.package_components,
            runs = aggregate.runs,
            wins = wins,
            clears = aggregate.outcomes.cleared or 0,
            ally_downs = aggregate.outcomes.ally_down or 0,
            package_failures = aggregate.outcomes.package_failed or 0,
            deaths = aggregate.outcomes.death or 0,
            step_limits = aggregate.outcomes.step_limit or 0,
            bot_stalls = aggregate.outcomes.bot_stalled or 0,
            game_loop_exits = aggregate.outcomes.game_loop_exit or 0,
            win_rate = format_rate(wins / runs),
            death_rate = format_rate((aggregate.outcomes.death or 0) / runs),
            outcome_counts = encode_counts(aggregate.outcomes),
        }
        for _, key in ipairs(SUMMARY_METRICS) do
            row["avg_" .. key] = format_rate((aggregate.sums[key] or 0) / runs)
            row["min_" .. key] = aggregate.mins[key] or 0
            row["max_" .. key] = aggregate.maxes[key] or 0
        end

        local controlled_frames = math.max(1, aggregate.sums.controlled_frames or 0)
        local combat_frames = aggregate.sums.combat_frames or 0
        local kills = aggregate.sums.kills or 0
        local damage_dealt = aggregate.sums.damage_dealt_total or 0
        row.combat_frame_rate = format_rate(combat_frames / controlled_frames)
        row.explore_frame_rate = format_rate((aggregate.sums.explore_frames or 0) / controlled_frames)
        row.rest_frame_rate = format_rate((aggregate.sums.rest_frames or 0) / controlled_frames)
        row.idle_frame_rate = format_rate((aggregate.sums.idle_frames or 0) / controlled_frames)
        row.portal_stop_frame_rate = format_rate((aggregate.sums.portal_stop_frames or 0) / controlled_frames)
        row.kills_per_1000_controlled_frames = format_rate(kills / controlled_frames * 1000)
        row.kills_per_1000_combat_frames = format_rate(combat_frames > 0 and kills / combat_frames * 1000 or 0)
        row.damage_dealt_per_1000_controlled_frames = format_rate(damage_dealt / controlled_frames * 1000)
        row.damage_dealt_per_1000_combat_frames = format_rate(combat_frames > 0 and damage_dealt / combat_frames * 1000 or 0)
        rows[#rows + 1] = row
    end
    return rows
end

local function class_run_counts(summaries)
    local counts = {}
    for _, row in ipairs(summaries) do
        local key = group_key(row.class, row.start_level, row.loadout, row.bot_profile)
        counts[key] = (counts[key] or 0) + 1
    end
    return counts
end

local function aggregate_spell_rows(spell_rows, summaries)
    local run_counts = class_run_counts(summaries)
    local groups = {}
    for _, row in ipairs(spell_rows) do
        local run_group = group_key(row.class, row.start_level, row.loadout, row.bot_profile)
        local key = run_group .. "\n" .. tostring(row.spell or "")
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = row.class,
                start_level = tonumber(row.start_level) or 1,
                loadout = row.loadout or "",
                bot_profile = row.bot_profile or "",
                run_group = run_group,
                spell = row.spell,
                runs_using = 0,
                total_orders = 0,
                max_orders_in_run = 0,
            }
            groups[key] = aggregate
        end
        local orders = tonumber(row.orders) or 0
        aggregate.runs_using = aggregate.runs_using + 1
        aggregate.total_orders = aggregate.total_orders + orders
        aggregate.max_orders_in_run = math.max(aggregate.max_orders_in_run, orders)
    end

    local rows = {}
    for _, key in ipairs(sorted_group_keys(groups)) do
        local aggregate = groups[key]
        local class_runs = math.max(1, run_counts[aggregate.run_group] or 0)
        rows[#rows + 1] = {
            class = aggregate.class,
            start_level = aggregate.start_level,
            loadout = aggregate.loadout,
            bot_profile = aggregate.bot_profile,
            spell = aggregate.spell,
            class_runs = run_counts[aggregate.run_group] or 0,
            runs_using = aggregate.runs_using,
            use_rate = format_rate(aggregate.runs_using / class_runs),
            total_orders = aggregate.total_orders,
            avg_orders_per_run = format_rate(aggregate.total_orders / class_runs),
            avg_orders_per_using_run = format_rate(aggregate.total_orders / math.max(1, aggregate.runs_using)),
            max_orders_in_run = aggregate.max_orders_in_run,
        }
    end
    return rows
end

local function aggregate_damage_rows(damage_rows, summaries, group_field, output_field)
    group_field = group_field or "source"
    output_field = output_field or group_field
    local run_counts = class_run_counts(summaries)
    local class_damage = {}
    local groups = {}
    for _, row in ipairs(damage_rows) do
        local run_group = group_key(row.class, row.start_level, row.loadout, row.bot_profile)
        class_damage[run_group] = (class_damage[run_group] or 0) + (tonumber(row.damage) or 0)
        local group_value = row[group_field] or ""
        local key = run_group .. "\n" .. tostring(group_value)
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = row.class,
                start_level = tonumber(row.start_level) or 1,
                loadout = row.loadout or "",
                bot_profile = row.bot_profile or "",
                run_group = run_group,
                group_value = group_value,
                runs_hit = 0,
                total_damage = 0,
                total_events = 0,
                max_damage_in_run = 0,
            }
            groups[key] = aggregate
        end
        local damage = tonumber(row.damage) or 0
        aggregate.runs_hit = aggregate.runs_hit + 1
        aggregate.total_damage = aggregate.total_damage + damage
        aggregate.total_events = aggregate.total_events + (tonumber(row.events) or 0)
        aggregate.max_damage_in_run = math.max(aggregate.max_damage_in_run, damage)
    end

    local rows = {}
    for _, key in ipairs(sorted_group_keys(groups)) do
        local aggregate = groups[key]
        local class_runs = math.max(1, run_counts[aggregate.run_group] or 0)
        local total_class_damage = class_damage[aggregate.run_group] or 0
        rows[#rows + 1] = {
            class = aggregate.class,
            start_level = aggregate.start_level,
            loadout = aggregate.loadout,
            bot_profile = aggregate.bot_profile,
            [output_field] = aggregate.group_value,
            class_runs = run_counts[aggregate.run_group] or 0,
            runs_hit = aggregate.runs_hit,
            hit_rate = format_rate(aggregate.runs_hit / class_runs),
            total_damage = format_rate(aggregate.total_damage),
            total_events = aggregate.total_events,
            avg_damage_per_run = format_rate(aggregate.total_damage / class_runs),
            avg_damage_per_hit_run = format_rate(aggregate.total_damage / math.max(1, aggregate.runs_hit)),
            avg_events_per_run = format_rate(aggregate.total_events / class_runs),
            max_damage_in_run = format_rate(aggregate.max_damage_in_run),
            damage_share = format_rate(safe_div(aggregate.total_damage, total_class_damage)),
        }
    end
    return rows
end

local function aggregate_enemy_rows(summaries)
    local class_totals = {}
    local enemy_groups = {}

    for _, row in ipairs(summaries) do
        local class_name = row.class or ""
        local start_level = tonumber(row.start_level) or 1
        local loadout = row.loadout or ""
        local bot_profile = row.bot_profile or ""
        local run_group = group_key(class_name, start_level, loadout, bot_profile)
        local class_total = class_totals[run_group]
        if not class_total then
            class_total = {
                class = class_name,
                start_level = start_level,
                loadout = loadout,
                bot_profile = bot_profile,
                runs = 0,
                kills = 0,
                visible_enemy_frames = 0,
                xp_from_kills = 0,
                controlled_frames = 0,
                combat_frames = 0,
            }
            class_totals[run_group] = class_total
        end

        class_total.runs = class_total.runs + 1
        class_total.controlled_frames = class_total.controlled_frames + (tonumber(row.controlled_frames) or 0)
        class_total.combat_frames = class_total.combat_frames + (tonumber(row.combat_frames) or 0)

        local kills = decode_counts(row.monster_kills)
        local visible_frames = decode_counts(row.visible_enemy_frames)
        local alive_counts = decode_counts(row.alive_enemy_counts)
        local alive_hp = decode_counts(row.alive_enemy_hp)
        local alive_max_hp = decode_counts(row.alive_enemy_max_hp)
        local seen = {}
        for monster, count in pairs(kills) do
            seen[monster] = true
            class_total.kills = class_total.kills + count
            class_total.xp_from_kills = class_total.xp_from_kills + count * enemy_xpaward(monster)
        end
        for monster, frames in pairs(visible_frames) do
            seen[monster] = true
            class_total.visible_enemy_frames = class_total.visible_enemy_frames + frames
        end
        for monster, _ in pairs(alive_counts) do
            seen[monster] = true
        end

        for monster, _ in pairs(seen) do
            local key = run_group .. "\n" .. monster
            local group = enemy_groups[key]
            if not group then
                group = {
                    class = class_name,
                    start_level = start_level,
                    loadout = loadout,
                    bot_profile = bot_profile,
                    monster = monster,
                    run_group = run_group,
                    runs_visible = 0,
                    runs_killing = 0,
                    kills = 0,
                    visible_enemy_frames = 0,
                    runs_alive_at_end = 0,
                    alive_at_end = 0,
                    end_hp = 0,
                    end_max_hp = 0,
                    xpaward = enemy_xpaward(monster),
                    xp_from_kills = 0,
                }
                enemy_groups[key] = group
            end

            local kill_count = kills[monster] or 0
            local frame_count = visible_frames[monster] or 0
            local alive_count = alive_counts[monster] or 0
            if kill_count > 0 then
                group.runs_killing = group.runs_killing + 1
            end
            if frame_count > 0 then
                group.runs_visible = group.runs_visible + 1
            end
            if alive_count > 0 then
                group.runs_alive_at_end = group.runs_alive_at_end + 1
            end
            group.kills = group.kills + kill_count
            group.visible_enemy_frames = group.visible_enemy_frames + frame_count
            group.alive_at_end = group.alive_at_end + alive_count
            group.end_hp = group.end_hp + (alive_hp[monster] or 0)
            group.end_max_hp = group.end_max_hp + (alive_max_hp[monster] or 0)
            group.xp_from_kills = group.xp_from_kills + kill_count * group.xpaward
        end
    end

    local rows = {}
    for _, key in ipairs(sorted_group_keys(enemy_groups)) do
        local group = enemy_groups[key]
        local class_total = class_totals[group.run_group] or {}
        local runs = math.max(1, class_total.runs or 0)
        rows[#rows + 1] = {
            class = group.class,
            start_level = group.start_level,
            loadout = group.loadout,
            bot_profile = group.bot_profile,
            monster = group.monster,
            runs = class_total.runs or 0,
            runs_visible = group.runs_visible,
            runs_killing = group.runs_killing,
            xpaward = group.xpaward,
            kills = group.kills,
            visible_enemy_frames = group.visible_enemy_frames,
            runs_alive_at_end = group.runs_alive_at_end,
            alive_at_end = group.alive_at_end,
            end_hp = format_rate(group.end_hp),
            end_max_hp = format_rate(group.end_max_hp),
            avg_kills_per_run = format_rate(group.kills / runs),
            avg_visible_enemy_frames_per_run = format_rate(group.visible_enemy_frames / runs),
            avg_alive_at_end_per_run = format_rate(group.alive_at_end / runs),
            avg_end_hp_per_run = format_rate(group.end_hp / runs),
            avg_end_hp_pct_when_alive = format_rate(safe_div(group.end_hp, group.end_max_hp)),
            kill_share = format_rate(safe_div(group.kills, class_total.kills)),
            visible_frame_share = format_rate(safe_div(group.visible_enemy_frames, class_total.visible_enemy_frames)),
            xp_from_kills = group.xp_from_kills,
            xp_share = format_rate(safe_div(group.xp_from_kills, class_total.xp_from_kills)),
            kills_per_1000_visible_enemy_frames = format_rate(safe_div(group.kills * 1000, group.visible_enemy_frames)),
            kills_per_1000_controlled_frames = format_rate(safe_div(group.kills * 1000, class_total.controlled_frames)),
            kills_per_1000_combat_frames = format_rate(safe_div(group.kills * 1000, class_total.combat_frames)),
        }
    end
    return rows
end

local REPORT_METRICS = {
    {
        key = "win_rate",
        label = "win rate",
        direction = "higher",
        absolute_delta_threshold = 0.20,
    },
    {
        key = "death_rate",
        label = "death rate",
        direction = "lower",
        absolute_delta_threshold = 0.20,
    },
    {
        key = "avg_deepest_floor",
        label = "deepest floor",
        direction = "higher",
    },
    {
        key = "avg_level",
        label = "level",
        direction = "higher",
    },
    {
        key = "avg_kills",
        label = "kills",
        direction = "higher",
    },
    {
        key = "avg_min_hp_pct",
        label = "minimum HP pct",
        direction = "higher",
        absolute_delta_threshold = 0.15,
    },
    {
        key = "kills_per_1000_combat_frames",
        label = "kills per 1000 combat frames",
        direction = "higher",
    },
    {
        key = "damage_dealt_per_1000_combat_frames",
        label = "damage dealt per 1000 combat frames",
        direction = "higher",
    },
}

local BASELINE_METRICS = {}
for _, metric in ipairs(REPORT_METRICS) do
    BASELINE_METRICS[#BASELINE_METRICS + 1] = metric
end
BASELINE_METRICS[#BASELINE_METRICS + 1] = {
    key = "combat_frame_rate",
    label = "combat contact rate",
}
BASELINE_METRICS[#BASELINE_METRICS + 1] = {
    key = "kills_per_1000_controlled_frames",
    label = "kills per 1000 controlled frames",
}
BASELINE_METRICS[#BASELINE_METRICS + 1] = {
    key = "damage_dealt_per_1000_controlled_frames",
    label = "damage dealt per 1000 controlled frames",
}
BASELINE_METRICS[#BASELINE_METRICS + 1] = {
    key = "avg_frames",
    label = "clear frames",
}

local function report_severity(metric, delta)
    if delta == 0 then
        return nil
    end
    if metric.direction == "higher" then
        return delta > 0 and "overperforming" or "underperforming"
    elseif metric.direction == "lower" then
        return delta > 0 and "underperforming" or "overperforming"
    end
    return delta > 0 and "high" or "low"
end

local function report_message(class_name, metric, value, baseline, delta_pct)
    return string.format(
        "%s %s is %.3f versus %.3f baseline (%+.1f%%).",
        class_name,
        metric.label,
        value,
        baseline,
        delta_pct * 100
    )
end

local function metric_baselines(class_rows, start_level, loadout, bot_profile)
    local baselines = {}
    for _, metric in ipairs(BASELINE_METRICS) do
        local total = 0
        local count = 0
        for _, row in ipairs(class_rows) do
            local same_level = not start_level or (tonumber(row.start_level) or 1) == start_level
            local same_loadout = (row.loadout or "") == (loadout or "")
            local same_profile = (row.bot_profile or "") == (bot_profile or "")
            if same_level and same_loadout and same_profile then
                total = total + (tonumber(row[metric.key]) or 0)
                count = count + 1
            end
        end
        baselines[metric.key] = count > 0 and total / count or 0
    end
    return baselines
end

local function add_report_row(rows, class_name, severity, category, metric, value, baseline, message)
    local delta = value - baseline
    local delta_pct = baseline ~= 0 and delta / math.abs(baseline) or 0
    local row = {
        class = class_name,
        severity = severity,
        category = category,
        metric = metric,
        value = format_rate(value),
        baseline = format_rate(baseline),
        delta = format_rate(delta),
        delta_pct = format_rate(delta_pct),
        message = message,
    }
    rows[#rows + 1] = row
    return row
end

local function report_class_name(row)
    local class_name = row.class or ""
    local start_level = tonumber(row.start_level) or 1
    local loadout = row.loadout or ""
    local bot_profile = row.bot_profile or ""
    local suffix = loadout ~= "" and (" " .. loadout) or ""
    if bot_profile ~= "" then
        suffix = suffix .. " " .. bot_profile
    end
    if start_level > 1 then
        return string.format("%s L%d%s", class_name, start_level, suffix)
    end
    return class_name .. suffix
end

local function has_package_report_data(row)
    return (row.encounter or "") ~= ""
        or (tonumber(row.avg_package_frames) or 0) > 0
        or (tonumber(row.avg_package_pressure_max_count) or 0) > 0
        or (tonumber(row.avg_package_rescue_opportunities) or 0) > 0
end

local drill_scorecards_by_key_cache

local function drill_scorecard_key(class_name, level, encounter, package_components, package_intensity, bot_profile)
    return table.concat({
        tostring(class_name or ""),
        tostring(level or ""),
        tostring(encounter or ""),
        tostring(package_components or ""),
        tostring(package_intensity or ""),
        tostring(bot_profile or ""),
    }, "\n")
end

local function drill_scorecards_by_key()
    if drill_scorecards_by_key_cache then
        return drill_scorecards_by_key_cache
    end
    local result = {}
    local ok, raw = pcall(function()
        return load_content_api().micro_drills_json("{}")
    end)
    if not ok then
        drill_scorecards_by_key_cache = result
        return result
    end
    local parsed_ok, payload = json.parse(raw or "")
    if not parsed_ok then
        drill_scorecards_by_key_cache = result
        return result
    end
    for _, drill in ipairs(payload.drills or {}) do
        local config = drill.config or {}
        local key = drill_scorecard_key(
            config.className,
            config.level,
            config.encounter,
            config.packageComponents,
            config.intensity,
            config.botProfile
        )
        result[key] = drill
    end
    drill_scorecards_by_key_cache = result
    return result
end

local function scorecard_passes(value, comparator, threshold)
    if comparator == ">=" then
        return value >= threshold
    elseif comparator == ">" then
        return value > threshold
    elseif comparator == "<=" then
        return value <= threshold
    elseif comparator == "<" then
        return value < threshold
    elseif comparator == "==" then
        return value == threshold
    end
    return true
end

local function scorecard_failure_severity(id, runs, min_runs)
    if runs < min_runs then
        return "info"
    end
    if id == "clear-floor" or id == "rescue-save" or id == "ally-margin" then
        return "critical"
    end
    return "warning"
end

local function add_drill_scorecard_report_rows(rows, row, class_name, runs, min_runs)
    if (row.encounter or "") == "" then
        return
    end
    local key = drill_scorecard_key(
        row.class,
        tonumber(row.start_level) or row.start_level,
        row.encounter,
        row.package_components,
        row.package_intensity,
        row.bot_profile
    )
    local drill = drill_scorecards_by_key()[key]
    if not drill or not drill.scorecard then
        return
    end

    local passed_count = 0
    local total_count = 0
    for _, score in ipairs(drill.scorecard.rows or {}) do
        local metric = score.metric or ""
        local value = tonumber(row[metric]) or 0
        local threshold = tonumber(score.threshold) or 0
        local passed = scorecard_passes(value, score.comparator or "", threshold)
        total_count = total_count + 1
        if passed then
            passed_count = passed_count + 1
        end
        local severity = passed and "info" or scorecard_failure_severity(score.id, runs, min_runs)
        local report = add_report_row(
            rows,
            class_name,
            severity,
            "drill_scorecard",
            metric,
            value,
            threshold,
            string.format(
                "%s %s %s: %s is %.3f, target %s (%s).",
                class_name,
                passed and "passes" or "misses",
                drill.label or "drill scorecard",
                score.label or metric,
                value,
                score.target or tostring(threshold),
                score.reason or "scorecard target"
            )
        )
        report.drill_id = drill.id or ""
        report.drill_label = drill.label or ""
        report.scorecard_id = score.id or ""
        report.target = score.target or ""
        report.comparator = score.comparator or ""
        report.threshold = format_rate(threshold)
        report.passed = passed and "true" or "false"
        report.reason = score.reason or ""
    end

    local summary = add_report_row(
        rows,
        class_name,
        total_count > 0 and passed_count == total_count and "info" or scorecard_failure_severity("summary", runs, min_runs),
        "drill_scorecard",
        "scorecard_pass_rate",
        total_count > 0 and passed_count / total_count or 0,
        1,
        string.format(
            "%s passed %d/%d %s scorecard targets.",
            class_name,
            passed_count,
            total_count,
            drill.label or "drill"
        )
    )
    summary.drill_id = drill.id or ""
    summary.drill_label = drill.label or ""
    summary.scorecard_id = "summary"
    summary.target = "all targets"
    summary.comparator = "=="
    summary.threshold = "1"
    summary.passed = total_count > 0 and passed_count == total_count and "true" or "false"
end

local function add_package_report_rows(rows, row, class_name, runs, min_runs)
    if not has_package_report_data(row) then
        return
    end

    local encounter = (row.encounter or "") ~= "" and row.encounter or "package"
    local win_rate = tonumber(row.win_rate) or 0
    local death_rate = tonumber(row.death_rate) or 0
    local min_hp_pct = tonumber(row.avg_min_hp_pct) or 0
    local pressure_peak = tonumber(row.avg_package_pressure_max_count) or 0
    local pressure_overlap = tonumber(row.avg_package_pressure_overlap_rate) or 0
    local pressure_sync_rate = tonumber(row.avg_package_pressure_sync_rate) or 0
    local pressure_sync_bursts = tonumber(row.avg_package_pressure_sync_bursts) or 0
    local pressure_sync_peak = tonumber(row.avg_package_pressure_sync_peak) or 0
    local pressure_unique_components = tonumber(row.avg_package_pressure_unique_components) or 0
    local pressure_signature_switches = tonumber(row.avg_package_pressure_signature_switches) or 0
    local pressure_variety_frames = tonumber(row.avg_package_pressure_variety_frames) or 0
    local pressure_dominance_rate = tonumber(row.avg_package_pressure_dominance_rate) or 0
    local package_frames = tonumber(row.avg_package_frames) or 0
    local micro_score = tonumber(row.avg_package_micro_score) or 0
    local micro_hits = tonumber(row.avg_package_micro_hits) or 0
    local micro_dodges = tonumber(row.avg_package_micro_dodges) or 0
    local micro_thread_cues = tonumber(row.avg_package_micro_thread_cues) or 0
    local micro_streak_bursts = tonumber(row.avg_package_micro_streak_bursts) or 0
    local micro_best_streak = tonumber(row.avg_package_micro_best_streak) or 0
    local micro_counter_windows = tonumber(row.avg_package_micro_counter_windows) or 0
    local micro_counter_damage = tonumber(row.avg_package_micro_counter_damage_total) or 0
    local micro_counter_stagger = tonumber(row.avg_package_micro_counter_stagger_frames) or 0
    local micro_punish_total = tonumber(row.avg_package_micro_punish_total) or 0
    local micro_punish_break_total = tonumber(row.avg_package_micro_punish_break_total) or 0
    local aggro_player_rate = tonumber(row.avg_package_aggro_player_rate) or 0
    local aggro_ally_rate = tonumber(row.avg_package_aggro_ally_rate) or 0
    local aggro_other_rate = tonumber(row.avg_package_aggro_other_rate) or 0
    local aggro_none_rate = tonumber(row.avg_package_aggro_none_rate) or 0
    local aggro_inferred_rate = tonumber(row.avg_package_aggro_inferred_rate) or 0
    local aggro_switches = tonumber(row.avg_package_aggro_switches) or 0

    add_report_row(
        rows,
        class_name,
        "info",
        "package_pressure",
        "avg_package_pressure_overlap_rate",
        pressure_overlap,
        0,
        string.format(
            "%s package pressure in %s averaged %.1f peak stack, %.1f%% overlap, %.1f%% sync-beat frames, %.1f sync bursts, and %.1f sync peak over %.0f package frames.",
            class_name,
            encounter,
            pressure_peak,
            pressure_overlap * 100,
            pressure_sync_rate * 100,
            pressure_sync_bursts,
            pressure_sync_peak,
            package_frames
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_sync",
        "avg_package_pressure_sync_bursts",
        pressure_sync_bursts,
        0,
        string.format(
            "%s sync beats in %s fired %.1f times/run on %.1f%% of package frames, highlighting 3+ simultaneous package demands.",
            class_name,
            encounter,
            pressure_sync_bursts,
            pressure_sync_rate * 100
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_variety",
        "avg_package_pressure_unique_components",
        pressure_unique_components,
        0,
        string.format(
            "%s beat variety in %s averaged %.1f unique mechanic types, %.1f signature switches, %.0f pressure frames, and %.1f%% dominant-mechanic share.",
            class_name,
            encounter,
            pressure_unique_components,
            pressure_signature_switches,
            pressure_variety_frames,
            pressure_dominance_rate * 100
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_micro",
        "avg_package_micro_score",
        micro_score,
        0,
        string.format(
            "%s package execution in %s averaged %.1f micro score, %.1f threaded avoids, %.1f thread cues, %.1f openings, %.1f opening damage, %.1f opening stagger, %.1f streak bursts, %.1f mechanic hits, %.1f punish, %.1f break, and %.1f best streak.",
            class_name,
            encounter,
            micro_score,
            micro_dodges,
            micro_thread_cues,
            micro_counter_windows,
            micro_counter_damage,
            micro_counter_stagger,
            micro_streak_bursts,
            micro_hits,
            micro_punish_total,
            micro_punish_break_total,
            micro_best_streak
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_aggro",
        "avg_package_aggro_player_rate",
        aggro_player_rate,
        0,
        string.format(
            "%s package aggro in %s averaged player %.1f%%, ally %.1f%%, other %.1f%%, none %.1f%%, inferred %.1f%%, and %.1f target switches/run.",
            class_name,
            encounter,
            aggro_player_rate * 100,
            aggro_ally_rate * 100,
            aggro_other_rate * 100,
            aggro_none_rate * 100,
            aggro_inferred_rate * 100,
            aggro_switches
        )
    )

    if win_rate >= 0.50 and aggro_ally_rate >= 0.35 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_aggro",
            "avg_package_aggro_ally_rate",
            aggro_ally_rate,
            0.35,
            string.format(
                "%s cleared %s while the primary targeted allies %.1f%% of package frames; check for forced-target cheese.",
                class_name,
                encounter,
                aggro_ally_rate * 100
            )
        )
    end

    if runs >= min_runs and win_rate >= 0.95 and death_rate <= 0.05 and min_hp_pct >= 0.45 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_difficulty",
            "win_rate",
            win_rate,
            0.95,
            string.format(
                "%s cleared %s %.1f%% of the time with %.1f%% average minimum HP; the selected automation profile may be over-clearing this package.",
                class_name,
                encounter,
                win_rate * 100,
                min_hp_pct * 100
            )
        )
    end

    if win_rate >= 0.75 and micro_score < 0 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_micro",
            "avg_package_micro_score",
            micro_score,
            0,
            string.format(
                "%s cleared %s %.1f%% of the time while averaging %.1f micro score; survivability may be masking failed package mechanics.",
                class_name,
                encounter,
                win_rate * 100,
                micro_score
            )
        )
    end

    if pressure_peak >= 2 and pressure_overlap < 0.05 then
        add_report_row(
            rows,
            class_name,
            "info",
            "package_pressure",
            "avg_package_pressure_overlap_rate",
            pressure_overlap,
            0.05,
            string.format(
                "%s saw stacked package components in %s, but overlap averaged only %.1f%%; verify the fight is not just serial single-beat pressure.",
                class_name,
                encounter,
                pressure_overlap * 100
            )
        )
    end

    if pressure_peak >= 3 and pressure_sync_bursts <= 0 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_sync",
            "avg_package_pressure_sync_bursts",
            pressure_sync_bursts,
            1,
            string.format(
                "%s reached %.1f peak pressure in %s but emitted no sync beats; stacked rhythm may not be readable enough in play.",
                class_name,
                pressure_peak,
                encounter
            )
        )
    end

    if pressure_peak >= 3 and pressure_unique_components < 3 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_variety",
            "avg_package_pressure_unique_components",
            pressure_unique_components,
            3,
            string.format(
                "%s reached %.1f peak pressure in %s but only exposed %.1f mechanic types; the package may be busy without feeling varied.",
                class_name,
                pressure_peak,
                encounter,
                pressure_unique_components
            )
        )
    end

    if pressure_peak >= 3 and pressure_dominance_rate >= 0.88 then
        add_report_row(
            rows,
            class_name,
            "info",
            "package_variety",
            "avg_package_pressure_dominance_rate",
            pressure_dominance_rate,
            0.88,
            string.format(
                "%s pressure in %s was %.1f%% dominated by one mechanic; check whether the full package reads as one repeated action.",
                class_name,
                encounter,
                pressure_dominance_rate * 100
            )
        )
    end

    local rescue_opportunities = tonumber(row.avg_package_rescue_opportunities) or 0
    if rescue_opportunities <= 0 then
        return
    end

    local save_rate = tonumber(row.avg_package_rescue_save_rate) or 0
    local clutch_rate = tonumber(row.avg_package_rescue_clutch_rate) or 0
    local perfect_rate = tonumber(row.avg_package_rescue_perfect_rate) or 0
    local early_rate = tonumber(row.avg_package_rescue_early_rate) or 0
    local timing_score = tonumber(row.avg_package_rescue_timing_score) or 0
    local window_cues = tonumber(row.avg_package_rescue_window_cues) or 0
    local perfect_window_cues = tonumber(row.avg_package_rescue_perfect_window_cues) or 0
    local late_window_frames = tonumber(row.avg_package_rescue_late_window_frames) or 0
    local late_uncovered_frames = tonumber(row.avg_package_rescue_late_uncovered_frames) or 0
    local late_cover_rate = tonumber(row.avg_package_rescue_late_cover_rate) or 0
    local perfect_window_frames = tonumber(row.avg_package_rescue_perfect_window_frames) or 0
    local perfect_uncovered_frames = tonumber(row.avg_package_rescue_perfect_uncovered_frames) or 0
    local perfect_cover_rate = tonumber(row.avg_package_rescue_perfect_cover_rate) or 0
    local avg_clutch_margin = tonumber(row.avg_package_rescue_avg_clutch_margin) or 0
    local avg_perfect_margin = tonumber(row.avg_package_rescue_avg_perfect_margin) or 0
    local avg_timed_pull_margin = tonumber(row.avg_package_rescue_avg_timed_pull_margin) or 0
    local pull_rate = tonumber(row.avg_package_rescue_pull_rate) or 0
    local timed_pull_rate = tonumber(row.avg_package_rescue_timed_pull_rate) or 0
    local pull_discipline_rate = tonumber(row.avg_package_rescue_pull_discipline_rate) or timed_pull_rate
    local perfect_pull_rate = tonumber(row.avg_package_rescue_perfect_pull_rate) or 0
    local pull_uptime = tonumber(row.avg_package_rescue_pull_uptime_rate) or 0
    local taunt_safe_frames = tonumber(row.avg_package_rescue_taunt_safe_frames) or 0
    local taunt_strain_frames = tonumber(row.avg_package_rescue_taunt_strain_frames) or 0
    local taunt_strain_rate = tonumber(row.avg_package_rescue_taunt_strain_rate) or 0
    local taunt_strain_breaks = tonumber(row.avg_package_rescue_taunt_strain_breaks) or 0
    local taunt_strain_pulses = tonumber(row.avg_package_rescue_taunt_strain_pulses) or 0
    local clutch_pulls = tonumber(row.avg_package_rescue_clutch_pulls) or 0
    local perfect_pulls = tonumber(row.avg_package_rescue_perfect_pulls) or 0
    local pull_snap_bursts = tonumber(row.avg_package_rescue_pull_snap_bursts) or 0
    local perfect_pull_snap_bursts = tonumber(row.avg_package_rescue_perfect_pull_snap_bursts) or 0
    local counter_windows = tonumber(row.avg_package_rescue_counter_windows) or 0
    local counter_damage = tonumber(row.avg_package_rescue_counter_damage_total) or 0
    local counter_stagger = tonumber(row.avg_package_rescue_counter_stagger_frames) or 0
    local fatal_misses = tonumber(row.avg_package_rescue_fatal_misses) or 0
    local survived_misses = tonumber(row.avg_package_rescue_survived_misses) or 0

    add_report_row(
        rows,
        class_name,
        "info",
        "package_micro",
        "avg_package_rescue_save_rate",
        save_rate,
        0,
        string.format(
            "%s rescue reads in %s averaged %.1f opportunities/run: save %.1f%%, clutch %.1f%% (%.1ff avg), perfect %.1f%% (%.1ff avg), early %.1f%%, timing %.2f, cues %.1f/%.1f perfect, pull %.1f%%, pull discipline %.1f%%, timed pulls %.1f%% (%.1ff avg), perfect pulls %.1f%%, taunt uptime %.1f%%, strain %.1f%%, counter %.1f windows, %.1f damage, %.1f stagger, fatal miss %.1f, survived miss %.1f.",
            class_name,
            encounter,
            rescue_opportunities,
            save_rate * 100,
            clutch_rate * 100,
            avg_clutch_margin,
            perfect_rate * 100,
            avg_perfect_margin,
            early_rate * 100,
            timing_score,
            window_cues,
            perfect_window_cues,
            pull_rate * 100,
            pull_discipline_rate * 100,
            timed_pull_rate * 100,
            avg_timed_pull_margin,
            perfect_pull_rate * 100,
            pull_uptime * 100,
            taunt_strain_rate * 100,
            counter_windows,
            counter_damage,
            counter_stagger,
            fatal_misses,
            survived_misses
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_rescue_clock",
        "avg_package_rescue_late_window_frames",
        late_window_frames,
        0,
        string.format(
            "%s rescue danger clock in %s averaged %.1f late-window frames/run (%.1f uncovered, %.1f%% covered) and %.1f perfect-window frames/run (%.1f uncovered, %.1f%% covered).",
            class_name,
            encounter,
            late_window_frames,
            late_uncovered_frames,
            late_cover_rate * 100,
            perfect_window_frames,
            perfect_uncovered_frames,
            perfect_cover_rate * 100
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_rescue_margin",
        "avg_package_rescue_avg_clutch_margin",
        avg_clutch_margin,
        0,
        string.format(
            "%s rescue margins in %s averaged %.1f frames for clutch saves, %.1f frames for perfect saves, and %.1f frames for timed pull saves.",
            class_name,
            encounter,
            avg_clutch_margin,
            avg_perfect_margin,
            avg_timed_pull_margin
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_pull_margin",
        "avg_package_rescue_avg_timed_pull_margin",
        avg_timed_pull_margin,
        0,
        string.format(
            "%s timed rescue pulls in %s averaged %.1f frames between block and strike.",
            class_name,
            encounter,
            avg_timed_pull_margin
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_rescue_cues",
        "avg_package_rescue_window_cues",
        window_cues,
        0,
        string.format(
            "%s rescue windows in %s emitted %.1f clutch/perfect window cues and %.1f perfect-window cues per run.",
            class_name,
            encounter,
            window_cues,
            perfect_window_cues
        )
    )

    if pull_rate > 0 then
        add_report_row(
            rows,
            class_name,
            "info",
            "package_pull_timing",
            "avg_package_rescue_timed_pull_rate",
            timed_pull_rate,
            0,
            string.format(
                "%s rescue pulls in %s averaged %.1f clutch pulls, %.1f perfect pulls, %.1f snap bursts, and %.1f perfect snap bursts/run; pull discipline is %.1f%% and %.1f%% of pulls started inside the late save window at %.1f frames on average.",
                class_name,
                encounter,
                clutch_pulls,
                perfect_pulls,
                pull_snap_bursts,
                perfect_pull_snap_bursts,
                pull_discipline_rate * 100,
                timed_pull_rate * 100,
                avg_timed_pull_margin
            )
        )
        add_report_row(
            rows,
            class_name,
            taunt_strain_breaks > 0 and "warning" or "info",
            "package_pull_strain",
            "avg_package_rescue_taunt_strain_rate",
            taunt_strain_rate,
            0.30,
            string.format(
                "%s rescue pulls in %s spent %.1f frames in clutch/perfect snap safety and %.1f frames in early-pull strain; strain triggered %.1f resistance pulses/run.",
                class_name,
                encounter,
                taunt_safe_frames,
                taunt_strain_frames,
                taunt_strain_pulses
            )
        )
    end

    add_report_row(
        rows,
        class_name,
        "info",
        "package_counter",
        "avg_package_rescue_counter_windows",
        counter_windows,
        0,
        string.format(
            "%s rescue counters in %s averaged %.1f openings/run for %.1f boss damage and %.1f stagger frames.",
            class_name,
            encounter,
            counter_windows,
            counter_damage,
            counter_stagger
        )
    )

    add_report_row(
        rows,
        class_name,
        "info",
        "package_rescue_miss_buffer",
        "avg_package_rescue_fatal_misses",
        fatal_misses,
        0,
        string.format(
            "%s rescue misses in %s averaged %.1f fatal misses and %.1f buffered misses per run.",
            class_name,
            encounter,
            fatal_misses,
            survived_misses
        )
    )

    if runs >= min_runs and save_rate < 0.50 then
        add_report_row(
            rows,
            class_name,
            "critical",
            "package_micro",
            "avg_package_rescue_save_rate",
            save_rate,
            0.50,
            string.format(
                "%s saved only %.1f%% of rescue strikes in %s; ally failure may dominate the encounter.",
                class_name,
                save_rate * 100,
                encounter
            )
        )
    end

    if pull_uptime >= 0.30 and win_rate >= 0.50 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_micro",
            "avg_package_rescue_pull_uptime_rate",
            pull_uptime,
            0.30,
            string.format(
                "%s spent %.1f%% of %s under rescue pull/taunt control; verify forced target control is not carrying the clear.",
                class_name,
                pull_uptime * 100,
                encounter
            )
        )
    end

    if pull_uptime >= 0.25 and timed_pull_rate < 0.35 and pull_rate >= 0.50 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_micro",
            "avg_package_rescue_pull_discipline_rate",
            pull_discipline_rate,
            0.35,
            string.format(
                "%s is using pull on %.1f%% of saves in %s, but pull discipline is only %.1f%%; this is likely target-control cheese rather than few-frame rescue micro.",
                class_name,
                pull_rate * 100,
                encounter,
                pull_discipline_rate * 100
            )
        )
    end

    if taunt_strain_rate >= 0.35 and taunt_strain_breaks > 0 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_pull_strain",
            "avg_package_rescue_taunt_strain_rate",
            taunt_strain_rate,
            0.35,
            string.format(
                "%s converted %.1f%% of pull uptime in %s into taunt strain; early forced-target control is being punished instead of treated as a clean rescue.",
                class_name,
                taunt_strain_rate * 100,
                encounter
            )
        )
    end

    if save_rate >= 0.50 and early_rate >= 0.60 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_micro",
            "avg_package_rescue_early_rate",
            early_rate,
            0.60,
            string.format(
                "%s saved %.1f%% of %s rescue strikes early; the lane may be solved by parking on the line instead of late body-block reads.",
                class_name,
                early_rate * 100,
                encounter
            )
        )
    end

    if save_rate >= 0.50 and early_rate >= 0.50 and late_cover_rate >= 0.80 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_micro",
            "avg_package_rescue_late_cover_rate",
            late_cover_rate,
            0.80,
            string.format(
                "%s covered %.1f%% of late rescue-window frames in %s while saving %.1f%% early; the line may be solved by parking before the few-frame read.",
                class_name,
                late_cover_rate * 100,
                encounter,
                early_rate * 100
            )
        )
    end

    if win_rate >= 0.75 and save_rate >= 0.75 and clutch_rate < 0.20 and perfect_rate <= 0.01 then
        add_report_row(
            rows,
            class_name,
            "warning",
            "package_micro",
            "avg_package_rescue_clutch_rate",
            clutch_rate,
            0.20,
            string.format(
                "%s is clearing %s with reliable saves but only %.1f%% clutch and %.1f%% perfect timing; the rescue may be solved by early parking instead of few-frame micro.",
                class_name,
                encounter,
                clutch_rate * 100,
                perfect_rate * 100
            )
        )
    end
end

local function balance_report_rows(class_rows, min_runs, relative_threshold)
    local rows = {}
    local baselines_by_group = {}
    min_runs = min_runs or 3
    relative_threshold = relative_threshold or 0.25

    for _, row in ipairs(class_rows) do
        local class_name = report_class_name(row)
        local start_level = tonumber(row.start_level) or 1
        local loadout = row.loadout or ""
        local bot_profile = row.bot_profile or ""
        local baseline_key = tostring(start_level) .. "\n" .. loadout .. "\n" .. bot_profile
        baselines_by_group[baseline_key] = baselines_by_group[baseline_key]
            or metric_baselines(class_rows, start_level, loadout, bot_profile)
        local baselines = baselines_by_group[baseline_key]
        local runs = tonumber(row.runs) or 0
        if runs < min_runs then
            add_report_row(
                rows,
                class_name,
                "info",
                "sample_size",
                "runs",
                runs,
                min_runs,
                string.format("%s has %d runs; use at least %d before treating balance flags as stable.", class_name, runs, min_runs)
            )
        end

        local stalls = tonumber(row.bot_stalls) or 0
        if stalls > 0 then
            add_report_row(
                rows,
                class_name,
                "critical",
                "automation",
                "bot_stalls",
                stalls,
                0,
                string.format("%s stalled in %d runs; balance data may be distorted by navigation or input behavior.", class_name, stalls)
            )
        end

        local idle_rate = tonumber(row.idle_frame_rate) or 0
        if idle_rate > 0.15 then
            add_report_row(
                rows,
                class_name,
                "warning",
                "automation",
                "idle_frame_rate",
                idle_rate,
                0.15,
                string.format("%s spent %.1f%% of controlled frames idle.", class_name, idle_rate * 100)
            )
        end

        local rest_rate = tonumber(row.rest_frame_rate) or 0
        if rest_rate > 0.35 then
            add_report_row(
                rows,
                class_name,
                "warning",
                "pacing",
                "rest_frame_rate",
                rest_rate,
                0.35,
                string.format("%s spent %.1f%% of controlled frames resting.", class_name, rest_rate * 100)
            )
        end

        local portal_stop_rate = tonumber(row.portal_stop_frame_rate) or 0
        if portal_stop_rate > 0.35 then
            add_report_row(
                rows,
                class_name,
                "warning",
                "automation",
                "portal_stop_frame_rate",
                portal_stop_rate,
                0.35,
                string.format("%s spent %.1f%% of controlled frames waiting on portals.", class_name, portal_stop_rate * 100)
            )
        end

        local death_rate = tonumber(row.death_rate) or 0
        if runs >= min_runs and death_rate >= 0.50 then
            add_report_row(
                rows,
                class_name,
                "critical",
                "survival",
                "death_rate",
                death_rate,
                0.50,
                string.format("%s died in %.1f%% of runs.", class_name, death_rate * 100)
            )
        end

        if row.class == "Necromancer" then
            local regen_recovered = tonumber(row.avg_necromancer_regen_recovered_total) or 0
            local gross_damage = tonumber(row.avg_damage_taken_total) or 0
            if regen_recovered > 0 and gross_damage > 0 then
                local adjusted_damage = tonumber(row.avg_regen_adjusted_damage_taken_total) or 0
                local offset_pct = tonumber(row.avg_necromancer_regen_offset_pct) or 0
                add_report_row(
                    rows,
                    class_name,
                    "info",
                    "necromancer_regen",
                    "avg_necromancer_regen_offset_pct",
                    offset_pct,
                    0,
                    string.format(
                        "%s gross incoming damage is %.1f/run, but Baleful Regeneration recovered %.1f/run (%.1f%% offset); compare survivability with deaths, minimum HP, and %.1f/run regen-adjusted damage.",
                        class_name,
                        gross_damage,
                        regen_recovered,
                        offset_pct * 100,
                        adjusted_damage
                    )
                )
            end
        end

        if row.class == "Dicebinder" then
            local rolls = tonumber(row.avg_dice_rolls_total) or 0
            if rolls > 0 then
                local frames = math.max(1, tonumber(row.avg_frames) or 1)
                local buff_frames = tonumber(row.avg_dice_buff_active_frames) or 0
                local dash_refreshes = tonumber(row.avg_dice_dash_effective_refreshes) or 0
                local power_refreshes = tonumber(row.avg_dice_power_effective_refreshes) or 0
                local jackpots = tonumber(row.avg_dice_jackpot_rolls) or 0
                local animation_frames = tonumber(row.avg_dice_roll_animation_frames) or 0
                local buildup_fx = tonumber(row.avg_dice_roll_buildup_fx) or 0
                local face_swaps = tonumber(row.avg_dice_roll_face_swaps) or 0
                local reveal_cues = tonumber(row.avg_dice_roll_reveal_cues) or 0
                local lock_cues = tonumber(row.avg_dice_roll_lock_cues) or 0
                local surge_cues = tonumber(row.avg_dice_roll_surge_cues) or 0
                local finale_fx = tonumber(row.avg_dice_roll_finale_fx) or 0
                local suspense_pips = tonumber(row.avg_dice_roll_suspense_pips) or 0
                local guard_cues = tonumber(row.avg_dice_roll_guard_cues) or 0
                local trim_frames = tonumber(row.avg_dice_cooldown_trim_frames) or 0
                local bag_refills = tonumber(row.avg_dice_bag_refills) or 0
                local anti_repeat_rerolls = tonumber(row.avg_dice_anti_repeat_rerolls) or 0
                local longest_high_streak = tonumber(row.avg_dice_longest_high_streak) or 0
                local longest_utility_streak = tonumber(row.avg_dice_longest_utility_streak) or 0
                local high_streak_breaks = tonumber(row.avg_dice_high_streak_breaks) or 0
                local low_streak_breaks = tonumber(row.avg_dice_low_streak_breaks) or 0
                local repeats = tonumber(row.avg_dice_repeat_rolls) or 0
                local high_rolls = (tonumber(row.avg_dice_roll_5_count) or 0) + (tonumber(row.avg_dice_roll_6_count) or 0)
                local max_face = 0
                for face = 1, 6 do
                    max_face = math.max(max_face, tonumber(row["avg_dice_roll_" .. tostring(face) .. "_count"]) or 0)
                end
                add_report_row(
                    rows,
                    class_name,
                    "info",
                    "dicebinder",
                    "avg_dice_rolls_total",
                    rolls,
                    0,
                    string.format(
                        "%s skill dice averaged %.1f rolls/run, %.1f animation frames/run, %.1f face swaps/run, %.1f buildup FX/run, %.1f surge cues/run, %.1f lock cues/run, %.1f suspense pips/run, %.1f guard cues/run, %.1f reveal cues/run, %.1f finale FX/run, %.1f cooldown-trim frames, %.1f effective Dash refreshes, %.1f effective Power refreshes, %.1f jackpots, %.1f%% high rolls, %.1f high-roll streak, %.1f utility streak, %.1f high-streak guards, %.1f dry-streak guards, %.1f repeat rolls, %.1f anti-repeat rerolls, %.1f%% max-face share, %.1f bag refills, and %.1f%% buff uptime.",
                        class_name,
                        rolls,
                        animation_frames,
                        face_swaps,
                        buildup_fx,
                        surge_cues,
                        lock_cues,
                        suspense_pips,
                        guard_cues,
                        reveal_cues,
                        finale_fx,
                        trim_frames,
                        dash_refreshes,
                        power_refreshes,
                        jackpots,
                        safe_div(high_rolls, rolls) * 100,
                        longest_high_streak,
                        longest_utility_streak,
                        high_streak_breaks,
                        low_streak_breaks,
                        repeats,
                        anti_repeat_rerolls,
                        safe_div(max_face, rolls) * 100,
                        bag_refills,
                        buff_frames / frames * 100
                    )
                )
            end
        end

        add_package_report_rows(rows, row, class_name, runs, min_runs)
        add_drill_scorecard_report_rows(rows, row, class_name, runs, min_runs)

        local combat_rate = tonumber(row.combat_frame_rate) or 0
        local combat_rate_baseline = baselines.combat_frame_rate or 0
        local combat_rate_delta_pct = combat_rate_baseline ~= 0
            and (combat_rate - combat_rate_baseline) / math.abs(combat_rate_baseline)
            or 0
        local low_combat_contact = combat_rate_baseline > 0
            and combat_rate_delta_pct <= -relative_threshold
        local high_combat_contact = combat_rate_baseline > 0
            and combat_rate_delta_pct >= relative_threshold
        if combat_rate_baseline > 0 and math.abs(combat_rate_delta_pct) >= relative_threshold then
            local severity = low_combat_contact and "warning" or "info"
            add_report_row(
                rows,
                class_name,
                severity,
                "contact",
                "combat_frame_rate",
                combat_rate,
                combat_rate_baseline,
                string.format(
                    "%s combat contact rate is %.1f%% versus %.1f%% baseline; kill metrics may reflect encounter exposure.",
                    class_name,
                    combat_rate * 100,
                    combat_rate_baseline * 100
                )
            )
        end

        for _, metric in ipairs(REPORT_METRICS) do
            local value = tonumber(row[metric.key]) or 0
            local baseline = baselines[metric.key] or 0
            local delta = value - baseline
            local delta_pct = baseline ~= 0 and delta / math.abs(baseline) or 0
            local flagged
            if metric.absolute_delta_threshold then
                flagged = math.abs(delta) >= metric.absolute_delta_threshold
            else
                flagged = math.abs(delta_pct) >= relative_threshold
            end
            if flagged and metric.key == "avg_kills" and delta < 0 and low_combat_contact then
                local combat_kill_rate = tonumber(row.kills_per_1000_combat_frames) or 0
                local combat_kill_baseline = baselines.kills_per_1000_combat_frames or 0
                if combat_kill_baseline > 0
                    and combat_kill_rate >= combat_kill_baseline * (1 - relative_threshold)
                then
                    flagged = false
                end
            end
            if flagged and metric.key == "avg_kills" then
                local combat_kill_rate = tonumber(row.kills_per_1000_combat_frames) or 0
                local combat_kill_baseline = baselines.kills_per_1000_combat_frames or 0
                if combat_kill_baseline > 0
                    and combat_kill_rate >= combat_kill_baseline * (1 - relative_threshold)
                    and combat_kill_rate <= combat_kill_baseline * (1 + relative_threshold)
                then
                    flagged = false
                    add_report_row(
                        rows,
                        class_name,
                        "info",
                        "pacing",
                        metric.key,
                        value,
                        baseline,
                        string.format(
                            "%s raw kill count differs from baseline, but combat-adjusted kill pacing is in range.",
                            class_name
                        )
                    )
                end
            end
            if flagged and metric.key == "kills_per_1000_combat_frames"
                and delta < 0 and high_combat_contact then
                local controlled_kill_rate = tonumber(row.kills_per_1000_controlled_frames) or 0
                local controlled_kill_baseline = baselines.kills_per_1000_controlled_frames or 0
                if controlled_kill_baseline > 0
                    and controlled_kill_rate >= controlled_kill_baseline * (1 - relative_threshold)
                then
                    flagged = false
                end
            end
            if flagged and metric.key == "kills_per_1000_combat_frames"
                and delta > 0 and low_combat_contact then
                local controlled_kill_rate = tonumber(row.kills_per_1000_controlled_frames) or 0
                local controlled_kill_baseline = baselines.kills_per_1000_controlled_frames or 0
                if controlled_kill_baseline > 0
                    and controlled_kill_rate <= controlled_kill_baseline * (1 + relative_threshold)
                then
                    flagged = false
                    add_report_row(
                        rows,
                        class_name,
                        "info",
                        "pacing",
                        metric.key,
                        value,
                        baseline,
                        string.format(
                            "%s combat kill rate is %.3f versus %.3f baseline under lower contact, but overall kill pacing is in range.",
                            class_name,
                            value,
                            baseline
                        )
                    )
                end
            end
            if flagged and metric.key == "kills_per_1000_combat_frames" and delta > 0 then
                local win_rate = tonumber(row.win_rate) or 0
                local win_baseline = baselines.win_rate or 0
                local death_rate = tonumber(row.death_rate) or 0
                local death_baseline = baselines.death_rate or 0
                local avg_frames = tonumber(row.avg_frames) or 0
                local frames_baseline = baselines.avg_frames or 0
                local worse_survival = (win_baseline > 0 and win_rate < win_baseline)
                    or death_rate > death_baseline
                local not_materially_faster = frames_baseline > 0
                    and avg_frames >= frames_baseline * (1 - relative_threshold)
                if worse_survival and not_materially_faster then
                    flagged = false
                    add_report_row(
                        rows,
                        class_name,
                        "info",
                        "pacing",
                        metric.key,
                        value,
                        baseline,
                        string.format(
                            "%s combat kill rate is %.3f versus %.3f baseline, but survival is worse and objective clear time is not materially faster.",
                            class_name,
                            value,
                            baseline
                        )
                    )
                end
            end
            if flagged and metric.key == "kills_per_1000_combat_frames" and delta < 0 then
                local controlled_kill_rate = tonumber(row.kills_per_1000_controlled_frames) or 0
                local controlled_kill_baseline = baselines.kills_per_1000_controlled_frames or 0
                if controlled_kill_baseline > 0
                    and controlled_kill_rate >= controlled_kill_baseline * (1 - relative_threshold)
                then
                    flagged = false
                    add_report_row(
                        rows,
                        class_name,
                        "info",
                        "pacing",
                        metric.key,
                        value,
                        baseline,
                        string.format(
                            "%s combat kill rate is %.3f versus %.3f baseline, but overall kill pacing is in range.",
                            class_name,
                            value,
                            baseline
                        )
                    )
                end
            end
            if flagged then
                add_report_row(
                    rows,
                    class_name,
                    report_severity(metric, delta) or "info",
                    "class_balance",
                    metric.key,
                    value,
                    baseline,
                    report_message(class_name, metric, value, baseline, delta_pct)
                )
            end
        end
    end

    if #rows == 0 then
        rows[#rows + 1] = {
            class = "",
            severity = "info",
            category = "class_balance",
            metric = "none",
            value = "",
            baseline = "",
            delta = "",
            delta_pct = "",
            message = "No balance flags generated for this run set.",
        }
    end
    return rows
end

local function nearest_enemy_distance_sqr(source_map, xy)
    local best
    for _, obj in ipairs(source_map.instances or {}) do
        if obj.is_enemy ~= false and obj.stats and obj.x and obj.y then
            local dx = obj.x - xy[1]
            local dy = obj.y - xy[2]
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

local function generate_start_map_desc(desc_name, options)
    options = options or {}
    load_core_modules()
    local NewMaps = require "maps.NewMaps"
    local MapUtils = require "maps.MapUtils"
    local SourceMap = require "core.SourceMap"
    local Vaults = require "maps.Vaults"

    local desc = require(desc_name)
    local generate_hooks = {}
    if options.map_profile and options.map_profile ~= "" then
        generate_hooks.overworld_profile = options.map_profile
    end
    local source_map = desc:compile({}, {}, generate_hooks)
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

local function game_init(run, telemetry, bot_options)
    load_core_modules()
    GameState.clear_players()
    GameState.register_player(
        "Autoplay",
        run.class_name,
        function(player)
            return require("ai.AutomatedPlayerInputSource").create(player, telemetry, bot_options)
        end,
        true,
        0,
        0
    )

    EngineInternal.init_gamestate()
    random_seed(run.seed)
    if content_lab_arena_requested(bot_options) then
        EngineHooks.first_map_create = function()
            return load_content_launch().generate_encounter_arena {
                encounter = bot_options.encounter,
                monsters = bot_options.monsters,
                package_intensity = bot_options.package_intensity,
                package_components = bot_options.package_components,
            }
        end
    elseif bot_options.start_map_desc and bot_options.start_map_desc ~= "" then
        EngineHooks.first_map_create = function()
            return generate_start_map_desc(bot_options.start_map_desc, {
                map_profile = bot_options.map_profile,
            })
        end
    end
    EngineInternal.start_game()
    local entry = World.players[1]
    local player = entry and entry.instance
    for _ = 2, (run.start_level or 1) do
        if player then
            player:gain_level()
        end
    end
    if player then
        apply_loadout(player, run.loadout_specs)
        local estats = player:effective_stats()
        player.stats.hp = estats.max_hp
        player.stats.mp = estats.max_mp
        player:add_effect("BalanceDamageTracker", {time_left = 10000000, telemetry = telemetry})
        attach_objective_damage_tracker(player, run.objective_enemy, telemetry)
        attach_damage_dealt_trackers(player, telemetry)
    end
    GameState.input_capture()
    if not GameState.input_handle() then
        error("Game exited during initial input handling.")
    end
    GameState.frame = 0
end

local function should_count_monster_death(monster)
    local entry = World.players[1]
    local player = entry and entry.instance
    if not player then
        return true
    end
    if monster.is_enemy == false then
        return false
    end
    return monster.team ~= player.team
end

local function install_run_hooks(events_module, base_player_death, base_monster_death, run_state, telemetry)
    load_core_modules()
    local Events = events_module

    Events.events.PlayerDeath = function(player)
        local entry = World.players[1]
        if entry and player == entry.instance then
            telemetry.min_hp = 0
            telemetry.min_hp_pct = 0
            run_state.outcome = "death"
            run_state.done = true
            return false
        end
        if base_player_death then
            return base_player_death(player)
        end
        return true
    end

    Events.events.MonsterDeath = function(monster)
        if should_count_monster_death(monster) then
            telemetry.monster_kills = telemetry.monster_kills or {}
            telemetry.monster_kills[monster.name] = (telemetry.monster_kills[monster.name] or 0) + 1
        end
        if base_monster_death then
            return base_monster_death(monster)
        end
    end

    EngineHooks.game_won = function()
        run_state.outcome = "won"
        run_state.done = true
    end
end

local function package_outcome_to_run_outcome(outcome)
    if outcome == "clear" then
        return "cleared"
    elseif outcome == "ally down" then
        return "ally_down"
    elseif outcome == "party down" then
        return "death"
    elseif outcome and outcome ~= "" then
        return "package_failed"
    end
    return nil
end

local function add_run(runs, run_id, class_name, start_level, seed, seed_group, class_run_index, loadout, objective_enemy)
    runs[#runs + 1] = {
        run_id = run_id,
        class_name = class_name,
        start_level = start_level,
        seed = seed,
        seed_group = seed_group,
        class_run_index = class_run_index,
        loadout_name = loadout.name,
        loadout_specs = loadout.specs_by_class[class_name] or {},
        objective_enemy = objective_enemy or "",
    }
end

local function build_runs(classes, levels, runs_per_class, seed, seed_mode, loadout, objective_enemy)
    local runs = {}
    local run_id = 1
    if seed_mode == "unique" then
        for _, class_name in ipairs(classes) do
            for _, start_level in ipairs(levels) do
                for idx = 1, runs_per_class do
                    add_run(runs, run_id, class_name, start_level, seed + run_id - 1, run_id, idx, loadout, objective_enemy)
                    run_id = run_id + 1
                end
            end
        end
    else
        for level_idx, start_level in ipairs(levels) do
            for idx = 1, runs_per_class do
                local seed_group = (level_idx - 1) * runs_per_class + idx
                local paired_seed = seed + seed_group - 1
                for _, class_name in ipairs(classes) do
                    add_run(runs, run_id, class_name, start_level, paired_seed, seed_group, idx, loadout, objective_enemy)
                    run_id = run_id + 1
                end
            end
        end
    end
    return runs
end

local function run_balance(raw_args)
    local parser = argparse("lanarts", "Run automated balance simulations.")
    parser:option("--steps", "Maximum frames per run.", "20000")
    parser:option("--runs", "Runs per class.", "1")
    parser:option("--seed", "First simulation seed.", "12345678")
    parser:option("--seed_mode", "Seed scheduling: paired reuses each seed across classes; unique uses a fresh seed for every run.", "paired")
    parser:option("--classes", "Comma-separated classes to run.", DEFAULT_CLASSES)
    parser:option("--levels", "Comma-separated starting levels to run.", "1")
    parser:option("--start_map_desc", "Optional map_desc module to start each run in, such as map_descs.PixullochiaDepths.", nil)
    parser:option("--map_profile", "Optional map-generation profile for --start_map_desc, such as open_meadow.", "")
    parser:option("--encounter", "Optional content-lab encounter id to start each run in an arena.", "")
    parser:option("--monsters", "Comma-separated monster names to start each run in a content-lab arena.", "")
    parser:option("--package_intensity", "Content-lab package pressure preset for arena encounters.", "standard")
    parser:option("--package_components", "Optional comma-separated package components: orbit,pulse,sweep,summon,escort,rescue.", "")
    parser:option("--loadout_preset", "Equipment preset for each class: none or late.", "none")
    parser:option("--loadout", "Comma-separated item specs to add to every run, using Item or Item*COUNT.", "")
    parser:option("--loadout_label", "Optional label for this loadout in outputs.", "")
    parser:option("--spell_policy", "Spell selection policy: mixed, priority, round_robin, none.", "mixed")
    parser:option("--bot_profile", "Automated-player skill profile: perfect, skilled, human, or sloppy.", "perfect")
    parser:option("--objective_enemy", "Optional enemy name that completes start-map-desc runs when no copies remain alive.", "")
    parser:option("--hunt_objective", "If true, path directly toward --objective_enemy while it is off-screen.", "false")
    parser:option("--sample_interval", "Frame interval for timeseries samples.", "300")
    parser:option("--draw_interval", "Frame interval for drawing visual capture frames. Use with LANARTS_SCREENSHOT_DIR.", "0")
    parser:option("--out", "Summary CSV output.", "saves/balance_runs.csv")
    parser:option("--samples_out", "Timeseries sample CSV output.", "saves/balance_samples.csv")
    parser:option("--spells_out", "Spell order CSV output.", "saves/balance_spells.csv")
    parser:option("--aggregate_out", "Class aggregate CSV output.", "saves/balance_classes.csv")
    parser:option("--spell_stats_out", "Spell aggregate CSV output.", "saves/balance_spell_stats.csv")
    parser:option("--damage_stats_out", "Damage source aggregate CSV output.", "saves/balance_damage_stats.csv")
    parser:option("--damage_dealt_stats_out", "Outgoing damage by target aggregate CSV output.", "saves/balance_damage_dealt_stats.csv")
    parser:option("--objective_damage_stats_out", "Objective damage source aggregate CSV output.", "saves/balance_objective_damage_stats.csv")
    parser:option("--enemy_stats_out", "Enemy aggregate CSV output.", "saves/balance_enemy_stats.csv")
    parser:option("--report_out", "Balance report CSV output.", "saves/balance_report.csv")
    parser:option("--min_report_runs", "Minimum runs per class before balance flags are considered stable.", "3")
    parser:option("--report_delta_pct", "Relative delta threshold for class-balance report flags.", "0.25")
    parser:option("--json_out", "Full JSON output.", "saves/balance_runs.json")
    parser:option("--invincible", "Run with invincibility enabled: true/false.", "false")
    local args = parser:parse(raw_args)

    local max_steps = tonumber(args.steps)
    local sample_interval = tonumber(args.sample_interval)
    local draw_interval = tonumber(args.draw_interval)
    local seed_mode = normalize_seed_mode(args.seed_mode)
    local classes = split_csv(args.classes)
    local loadout_preset = normalize_loadout_preset(args.loadout_preset)
    local custom_loadout_specs = parse_loadout_specs(args.loadout)
    local specs_by_class = {}
    for _, class_name in ipairs(classes) do
        specs_by_class[class_name] = loadout_specs_for_class(loadout_preset, custom_loadout_specs, class_name)
    end
    local loadout = {
        name = loadout_name(loadout_preset, custom_loadout_specs, args.loadout_label),
        preset = loadout_preset,
        custom_specs = custom_loadout_specs,
        specs_by_class = specs_by_class,
    }
    local runs = build_runs(classes, parse_levels(args.levels), tonumber(args.runs), tonumber(args.seed), seed_mode, loadout, args.objective_enemy)
    for _, run in ipairs(runs) do
        run.bot_profile = args.bot_profile
        run.start_map_desc = args.start_map_desc or ""
        run.map_profile = args.map_profile or ""
        run.encounter = args.encounter or ""
        run.monsters = args.monsters or ""
        run.package_intensity = args.package_intensity or ""
        run.package_components = args.package_components or ""
    end
    local summaries = {}
    local samples = {}
    local spell_rows = {}
    local damage_rows = {}
    local damage_dealt_target_rows = {}
    local objective_damage_rows = {}
    local json_runs = {}
    local Events
    local base_player_death
    local base_monster_death

    local run_index = 1
    local game_start

    local function finish_all()
        local class_rows = aggregate_summary_rows(summaries)
        local spell_stat_rows = aggregate_spell_rows(spell_rows, summaries)
        local damage_stat_rows = aggregate_damage_rows(damage_rows, summaries)
        local damage_dealt_stat_rows = aggregate_damage_rows(damage_dealt_target_rows, summaries, "target", "target")
        local objective_damage_stat_rows = aggregate_damage_rows(objective_damage_rows, summaries)
        local enemy_stat_rows = aggregate_enemy_rows(summaries)
        local report_rows = balance_report_rows(class_rows, tonumber(args.min_report_runs), tonumber(args.report_delta_pct))
        write_csv(args.out, summaries, {
            "run_id", "seed", "seed_group", "class_run_index", "class", "start_level", "loadout", "bot_profile",
            "objective_enemy", "start_map_desc", "map_profile", "encounter", "monsters", "package_intensity", "package_components",
            "outcome", "frames", "level", "xp",
            "hp", "max_hp", "hp_pct", "min_hp", "min_hp_pct", "mp", "max_mp", "mp_pct", "kills",
            "player_kills", "deaths", "deepest_floor", "current_floor", "map_label",
            "enemies_alive", "visible_enemies", "spells_known",
            "package_outcome", "package_frames", "package_escort_count", "package_escort_hp",
            "package_escort_max_hp", "package_escort_hp_pct", "package_rescue_saves",
            "package_primary_name", "package_primary_count", "package_primary_hp",
            "package_primary_max_hp", "package_primary_hp_pct",
            "package_aggro_target_role", "package_aggro_target_name",
            "package_aggro_frames", "package_aggro_player_frames", "package_aggro_player_rate",
            "package_aggro_ally_frames", "package_aggro_ally_rate", "package_aggro_other_frames",
            "package_aggro_other_rate", "package_aggro_none_frames", "package_aggro_none_rate",
            "package_aggro_inferred_frames", "package_aggro_inferred_rate", "package_aggro_switches",
            "package_next_threat_id", "package_next_threat_label",
            "package_next_threat_state", "package_next_threat_frames",
            "package_pressure_active_count", "package_pressure_warning_count",
            "package_pressure_total_count", "package_pressure_max_count",
            "package_pressure_overlap_frames", "package_pressure_overlap_rate",
            "package_pressure_sync_frames", "package_pressure_sync_rate",
            "package_pressure_sync_bursts", "package_pressure_sync_peak",
            "package_pressure_unique_components", "package_pressure_signature_switches",
            "package_pressure_variety_frames", "package_pressure_dominant_component",
            "package_pressure_dominant_component_frames", "package_pressure_dominance_rate",
            "package_pressure_labels",
            "package_micro_score", "package_micro_streak", "package_micro_best_streak",
            "package_micro_hits", "package_micro_dodges", "package_micro_events",
            "package_micro_thread_cues", "package_micro_streak_bursts",
            "package_micro_counter_windows", "package_micro_counter_damage_total",
            "package_micro_counter_stagger_frames",
            "package_micro_punish_total", "package_micro_punish_events",
            "package_micro_punish_break_total", "package_micro_punish_break_events",
            "package_summon_warning_count", "package_summon_warning_remaining",
            "package_orb_active_count", "package_orb_active_remaining",
            "package_pulse_warning_count", "package_pulse_warning_remaining",
            "package_pulse_active_count", "package_pulse_active_remaining",
            "package_sweep_warning_count", "package_sweep_warning_remaining",
            "package_sweep_active_count", "package_sweep_active_remaining",
            "package_rescue_opportunities", "package_rescue_save_rate",
            "package_rescue_clutch_saves", "package_rescue_clutch_rate",
            "package_rescue_perfect_saves", "package_rescue_perfect_rate",
            "package_rescue_early_saves", "package_rescue_early_rate",
            "package_rescue_timing_score",
            "package_rescue_window_cues", "package_rescue_perfect_window_cues",
            "package_rescue_late_window_frames", "package_rescue_late_uncovered_frames",
            "package_rescue_late_cover_rate", "package_rescue_perfect_window_frames",
            "package_rescue_perfect_uncovered_frames", "package_rescue_perfect_cover_rate",
            "package_rescue_perfect_window", "package_rescue_best_perfect_margin",
            "package_rescue_margin_total", "package_rescue_clutch_margin_total",
            "package_rescue_perfect_margin_total", "package_rescue_avg_clutch_margin",
            "package_rescue_avg_perfect_margin",
            "package_rescue_pulls", "package_rescue_pull_rate",
            "package_rescue_clutch_pulls", "package_rescue_perfect_pulls",
            "package_rescue_timed_pull_rate", "package_rescue_pull_discipline_rate", "package_rescue_perfect_pull_rate",
            "package_rescue_pull_margin_total", "package_rescue_timed_pull_margin_total",
            "package_rescue_perfect_pull_margin_total", "package_rescue_avg_pull_margin",
            "package_rescue_avg_timed_pull_margin",
            "package_rescue_pull_snap_bursts", "package_rescue_perfect_pull_snap_bursts",
            "package_rescue_pull_total_frames", "package_rescue_pull_uptime_rate",
            "package_rescue_taunt_safe_frames", "package_rescue_taunt_strain_frames",
            "package_rescue_taunt_strain_rate", "package_rescue_taunt_strain_breaks",
            "package_rescue_taunt_strain_pulses",
            "package_rescue_counter_windows", "package_rescue_counter_damage_total",
            "package_rescue_counter_stagger_frames",
            "package_rescue_misses",
            "package_rescue_fatal_misses", "package_rescue_survived_misses",
            "package_rescue_best_margin", "package_rescue_best_pull_margin",
            "package_rescue_last_margin", "package_rescue_last_pull_frames", "package_rescue_last_pull_margin",
            "package_rescue_next_frames", "package_rescue_active_count", "package_rescue_active_remaining",
            "package_rescue_active_blocked", "package_rescue_active_late", "package_rescue_active_perfect",
            "package_rescue_pull_active_frames", "package_rescue_pull_target",
            "spell_orders_total", "ability_orders_total", "weapon_orders", "item_orders_total",
            "dice_rolls_total", "dice_last_roll", "dice_next_roll_frames",
            "dice_pending_roll", "dice_rolling_frames",
            "dice_roll_1_count", "dice_roll_2_count", "dice_roll_3_count",
            "dice_roll_4_count", "dice_roll_5_count", "dice_roll_6_count",
            "dice_dash_refreshes", "dice_dash_effective_refreshes",
            "dice_power_refreshes", "dice_power_effective_refreshes",
            "dice_jackpot_rolls", "dice_roll_animations_total",
            "dice_roll_animation_frames", "dice_roll_buildup_fx",
            "dice_roll_face_swaps", "dice_roll_reveal_cues", "dice_roll_lock_cues",
            "dice_roll_surge_cues", "dice_roll_finale_fx", "dice_roll_suspense_pips",
            "dice_roll_guard_cues",
            "dice_cooldown_trim_frames", "dice_dash_trim_effective", "dice_power_trim_effective",
            "dice_bag_refills", "dice_anti_repeat_rerolls",
            "dice_high_rolls", "dice_utility_rolls", "dice_longest_high_streak",
            "dice_longest_utility_streak", "dice_high_streak_breaks", "dice_low_streak_breaks",
            "dice_repeat_rolls", "dice_roll_variety_switches", "dice_buff_active_frames",
            "dice_fleet_frames", "dice_guard_frames", "dice_edge_frames",
            "dice_focus_frames", "dice_dash_frames", "dice_jackpot_frames",
            "damage_taken_total", "regen_adjusted_damage_taken_total",
            "necromancer_regen_offset_pct", "damage_event_total", "damage_taken_by_source",
            "damage_events_by_source",
            "damage_dealt_total", "damage_dealt_event_total", "damage_dealt_by_target",
            "damage_dealt_events_by_target", "damage_dealt_by_source", "damage_dealt_events_by_source",
            "objective_damage_total", "objective_damage_event_total",
            "objective_damage_by_source", "objective_damage_events_by_source",
            "reaction_hold_frames", "movement_noise_frames", "spell_skip_frames",
            "item_skip_frames", "weapon_skip_frames",
            "hunt_enemy_frames", "ranged_keepaway_frames", "ranged_close_enemy_frames",
            "ranged_close_gragh_frames", "rescue_intercept_frames", "rescue_hold_frames",
            "rescue_late_reaction_frames",
            "necromancer_regen_frames", "necromancer_regen_recovered_total",
            "monster_kills", "visible_enemy_frames", "alive_enemy_counts",
            "alive_enemy_hp", "alive_enemy_max_hp",
        })
        write_csv(args.samples_out, samples, {
            "run_id", "seed", "seed_group", "class_run_index", "class", "start_level", "loadout", "bot_profile",
            "objective_enemy", "start_map_desc", "map_profile", "encounter", "monsters", "package_intensity", "package_components",
            "frame", "reason", "level", "xp",
            "hp", "max_hp", "hp_pct", "mp", "max_mp", "mp_pct", "kills",
            "player_kills", "deaths", "deepest_floor", "current_floor", "map_label",
            "enemies_alive", "visible_enemies", "alive_enemy_counts",
            "alive_enemy_hp", "alive_enemy_max_hp", "spells_known",
            "package_outcome", "package_frames", "package_escort_count", "package_escort_hp",
            "package_escort_max_hp", "package_escort_hp_pct", "package_rescue_saves",
            "package_primary_name", "package_primary_count", "package_primary_hp",
            "package_primary_max_hp", "package_primary_hp_pct",
            "package_aggro_target_role", "package_aggro_target_name",
            "package_aggro_frames", "package_aggro_player_frames", "package_aggro_player_rate",
            "package_aggro_ally_frames", "package_aggro_ally_rate", "package_aggro_other_frames",
            "package_aggro_other_rate", "package_aggro_none_frames", "package_aggro_none_rate",
            "package_aggro_inferred_frames", "package_aggro_inferred_rate", "package_aggro_switches",
            "package_next_threat_id", "package_next_threat_label",
            "package_next_threat_state", "package_next_threat_frames",
            "package_pressure_active_count", "package_pressure_warning_count",
            "package_pressure_total_count", "package_pressure_max_count",
            "package_pressure_overlap_frames", "package_pressure_overlap_rate",
            "package_pressure_sync_frames", "package_pressure_sync_rate",
            "package_pressure_sync_bursts", "package_pressure_sync_peak",
            "package_pressure_unique_components", "package_pressure_signature_switches",
            "package_pressure_variety_frames", "package_pressure_dominant_component",
            "package_pressure_dominant_component_frames", "package_pressure_dominance_rate",
            "package_pressure_labels",
            "package_micro_score", "package_micro_streak", "package_micro_best_streak",
            "package_micro_hits", "package_micro_dodges", "package_micro_events",
            "package_micro_thread_cues", "package_micro_streak_bursts",
            "package_micro_counter_windows", "package_micro_counter_damage_total",
            "package_micro_counter_stagger_frames",
            "package_micro_punish_total", "package_micro_punish_events",
            "package_micro_punish_break_total", "package_micro_punish_break_events",
            "package_summon_warning_count", "package_summon_warning_remaining",
            "package_orb_active_count", "package_orb_active_remaining",
            "package_pulse_warning_count", "package_pulse_warning_remaining",
            "package_pulse_active_count", "package_pulse_active_remaining",
            "package_sweep_warning_count", "package_sweep_warning_remaining",
            "package_sweep_active_count", "package_sweep_active_remaining",
            "package_rescue_opportunities", "package_rescue_save_rate",
            "package_rescue_clutch_saves", "package_rescue_clutch_rate",
            "package_rescue_perfect_saves", "package_rescue_perfect_rate",
            "package_rescue_early_saves", "package_rescue_early_rate",
            "package_rescue_timing_score",
            "package_rescue_window_cues", "package_rescue_perfect_window_cues",
            "package_rescue_late_window_frames", "package_rescue_late_uncovered_frames",
            "package_rescue_late_cover_rate", "package_rescue_perfect_window_frames",
            "package_rescue_perfect_uncovered_frames", "package_rescue_perfect_cover_rate",
            "package_rescue_perfect_window", "package_rescue_best_perfect_margin",
            "package_rescue_margin_total", "package_rescue_clutch_margin_total",
            "package_rescue_perfect_margin_total", "package_rescue_avg_clutch_margin",
            "package_rescue_avg_perfect_margin",
            "package_rescue_pulls", "package_rescue_pull_rate",
            "package_rescue_clutch_pulls", "package_rescue_perfect_pulls",
            "package_rescue_timed_pull_rate", "package_rescue_pull_discipline_rate", "package_rescue_perfect_pull_rate",
            "package_rescue_pull_margin_total", "package_rescue_timed_pull_margin_total",
            "package_rescue_perfect_pull_margin_total", "package_rescue_avg_pull_margin",
            "package_rescue_avg_timed_pull_margin",
            "package_rescue_pull_snap_bursts", "package_rescue_perfect_pull_snap_bursts",
            "package_rescue_pull_total_frames", "package_rescue_pull_uptime_rate",
            "package_rescue_taunt_safe_frames", "package_rescue_taunt_strain_frames",
            "package_rescue_taunt_strain_rate", "package_rescue_taunt_strain_breaks",
            "package_rescue_taunt_strain_pulses",
            "package_rescue_counter_windows", "package_rescue_counter_damage_total",
            "package_rescue_counter_stagger_frames",
            "package_rescue_misses",
            "package_rescue_fatal_misses", "package_rescue_survived_misses",
            "package_rescue_best_margin", "package_rescue_best_pull_margin",
            "package_rescue_last_margin", "package_rescue_last_pull_frames", "package_rescue_last_pull_margin",
            "package_rescue_next_frames", "package_rescue_active_count", "package_rescue_active_remaining",
            "package_rescue_active_blocked", "package_rescue_active_late", "package_rescue_active_perfect",
            "package_rescue_pull_active_frames", "package_rescue_pull_target",
            "spell_orders_total", "ability_orders_total", "weapon_orders", "item_orders_total",
            "dice_rolls_total", "dice_last_roll", "dice_next_roll_frames",
            "dice_pending_roll", "dice_rolling_frames",
            "dice_roll_1_count", "dice_roll_2_count", "dice_roll_3_count",
            "dice_roll_4_count", "dice_roll_5_count", "dice_roll_6_count",
            "dice_dash_refreshes", "dice_dash_effective_refreshes",
            "dice_power_refreshes", "dice_power_effective_refreshes",
            "dice_jackpot_rolls", "dice_roll_animations_total",
            "dice_roll_animation_frames", "dice_roll_buildup_fx",
            "dice_roll_face_swaps", "dice_roll_reveal_cues", "dice_roll_lock_cues",
            "dice_roll_surge_cues", "dice_roll_finale_fx", "dice_roll_suspense_pips",
            "dice_roll_guard_cues",
            "dice_cooldown_trim_frames", "dice_dash_trim_effective", "dice_power_trim_effective",
            "dice_bag_refills", "dice_anti_repeat_rerolls",
            "dice_high_rolls", "dice_utility_rolls", "dice_longest_high_streak",
            "dice_longest_utility_streak", "dice_high_streak_breaks", "dice_low_streak_breaks",
            "dice_repeat_rolls", "dice_roll_variety_switches", "dice_buff_active_frames",
            "dice_fleet_frames", "dice_guard_frames", "dice_edge_frames",
            "dice_focus_frames", "dice_dash_frames", "dice_jackpot_frames",
            "damage_taken_total", "regen_adjusted_damage_taken_total",
            "necromancer_regen_offset_pct", "damage_event_total", "damage_taken_by_source",
            "damage_events_by_source",
            "damage_dealt_total", "damage_dealt_event_total", "damage_dealt_by_target",
            "damage_dealt_events_by_target", "damage_dealt_by_source", "damage_dealt_events_by_source",
            "objective_damage_total", "objective_damage_event_total",
            "objective_damage_by_source", "objective_damage_events_by_source",
            "reaction_hold_frames", "movement_noise_frames", "spell_skip_frames",
            "item_skip_frames", "weapon_skip_frames",
            "hunt_enemy_frames", "ranged_keepaway_frames", "ranged_close_enemy_frames",
            "ranged_close_gragh_frames", "rescue_intercept_frames", "rescue_hold_frames",
            "rescue_late_reaction_frames",
        })
        write_csv(args.spells_out, spell_rows, {"run_id", "seed", "seed_group", "class_run_index", "class", "start_level", "loadout", "bot_profile", "spell", "orders"})
        write_csv(args.damage_stats_out, damage_stat_rows, {
            "class", "start_level", "loadout", "bot_profile", "source", "class_runs", "runs_hit",
            "hit_rate", "total_damage", "total_events", "avg_damage_per_run",
            "avg_damage_per_hit_run", "avg_events_per_run", "max_damage_in_run",
            "damage_share",
        })
        write_csv(args.damage_dealt_stats_out, damage_dealt_stat_rows, {
            "class", "start_level", "loadout", "bot_profile", "target", "class_runs", "runs_hit",
            "hit_rate", "total_damage", "total_events", "avg_damage_per_run",
            "avg_damage_per_hit_run", "avg_events_per_run", "max_damage_in_run",
            "damage_share",
        })
        write_csv(args.objective_damage_stats_out, objective_damage_stat_rows, {
            "class", "start_level", "loadout", "bot_profile", "source", "class_runs", "runs_hit",
            "hit_rate", "total_damage", "total_events", "avg_damage_per_run",
            "avg_damage_per_hit_run", "avg_events_per_run", "max_damage_in_run",
            "damage_share",
        })
        write_csv(args.aggregate_out, class_rows, {
            "class", "start_level", "loadout", "bot_profile", "objective_enemy", "start_map_desc",
            "map_profile", "encounter", "monsters", "package_intensity", "package_components",
            "runs", "wins", "clears", "ally_downs", "package_failures", "deaths", "step_limits", "bot_stalls",
            "game_loop_exits", "win_rate", "death_rate", "outcome_counts",
            "combat_frame_rate", "explore_frame_rate", "rest_frame_rate",
            "idle_frame_rate", "portal_stop_frame_rate",
            "kills_per_1000_controlled_frames", "kills_per_1000_combat_frames",
            "damage_dealt_per_1000_controlled_frames", "damage_dealt_per_1000_combat_frames",
            "avg_damage_taken_total", "avg_regen_adjusted_damage_taken_total",
            "avg_necromancer_regen_offset_pct", "avg_damage_event_total",
            "avg_damage_dealt_total", "avg_damage_dealt_event_total",
            "avg_min_hp_pct", "min_min_hp_pct",
            "avg_objective_damage_total", "avg_objective_damage_event_total",
            "avg_reaction_hold_frames", "avg_movement_noise_frames",
            "avg_spell_skip_frames", "avg_item_skip_frames", "avg_weapon_skip_frames",
            "avg_dice_rolls_total", "avg_dice_roll_1_count", "avg_dice_roll_2_count",
            "avg_dice_roll_3_count", "avg_dice_roll_4_count", "avg_dice_roll_5_count",
            "avg_dice_roll_6_count", "avg_dice_dash_refreshes", "avg_dice_dash_effective_refreshes",
            "avg_dice_power_refreshes", "avg_dice_power_effective_refreshes", "avg_dice_jackpot_rolls",
            "avg_dice_roll_animations_total", "avg_dice_roll_animation_frames",
            "avg_dice_roll_buildup_fx", "avg_dice_roll_face_swaps", "avg_dice_roll_reveal_cues", "avg_dice_roll_lock_cues",
            "avg_dice_roll_surge_cues", "avg_dice_roll_finale_fx", "avg_dice_roll_suspense_pips", "avg_dice_roll_guard_cues", "avg_dice_cooldown_trim_frames",
            "avg_dice_dash_trim_effective", "avg_dice_power_trim_effective", "avg_dice_bag_refills", "avg_dice_anti_repeat_rerolls",
            "avg_dice_high_rolls", "avg_dice_utility_rolls", "avg_dice_longest_high_streak",
            "avg_dice_longest_utility_streak", "avg_dice_high_streak_breaks", "avg_dice_low_streak_breaks",
            "avg_dice_repeat_rolls", "avg_dice_roll_variety_switches",
            "avg_dice_buff_active_frames", "avg_dice_fleet_frames", "avg_dice_guard_frames",
            "avg_dice_edge_frames", "avg_dice_focus_frames", "avg_dice_dash_frames", "avg_dice_jackpot_frames",
            "avg_ranged_keepaway_frames", "avg_ranged_close_enemy_frames",
            "avg_ranged_close_gragh_frames", "avg_rescue_intercept_frames",
            "avg_rescue_hold_frames", "avg_rescue_late_reaction_frames", "avg_necromancer_regen_frames",
            "avg_necromancer_regen_recovered_total",
            "avg_package_frames", "avg_package_escort_hp_pct", "avg_package_rescue_saves",
            "avg_package_aggro_frames", "avg_package_aggro_player_frames", "avg_package_aggro_player_rate",
            "avg_package_aggro_ally_frames", "avg_package_aggro_ally_rate",
            "avg_package_aggro_other_frames", "avg_package_aggro_other_rate",
            "avg_package_aggro_none_frames", "avg_package_aggro_none_rate",
            "avg_package_aggro_inferred_frames", "avg_package_aggro_inferred_rate",
            "avg_package_aggro_switches",
            "avg_package_rescue_opportunities", "avg_package_rescue_save_rate",
            "avg_package_rescue_clutch_saves", "avg_package_rescue_clutch_rate",
            "avg_package_rescue_perfect_saves", "avg_package_rescue_perfect_rate",
            "avg_package_rescue_early_saves", "avg_package_rescue_early_rate",
            "avg_package_rescue_timing_score",
            "avg_package_rescue_window_cues", "avg_package_rescue_perfect_window_cues",
            "avg_package_rescue_late_window_frames", "avg_package_rescue_late_uncovered_frames",
            "avg_package_rescue_late_cover_rate", "avg_package_rescue_perfect_window_frames",
            "avg_package_rescue_perfect_uncovered_frames", "avg_package_rescue_perfect_cover_rate",
            "avg_package_rescue_margin_total", "avg_package_rescue_clutch_margin_total",
            "avg_package_rescue_perfect_margin_total", "avg_package_rescue_avg_clutch_margin",
            "avg_package_rescue_avg_perfect_margin",
            "avg_package_rescue_pulls", "avg_package_rescue_pull_rate",
            "avg_package_rescue_clutch_pulls", "avg_package_rescue_perfect_pulls",
            "avg_package_rescue_timed_pull_rate", "avg_package_rescue_pull_discipline_rate", "avg_package_rescue_perfect_pull_rate",
            "avg_package_rescue_pull_margin_total", "avg_package_rescue_timed_pull_margin_total",
            "avg_package_rescue_perfect_pull_margin_total", "avg_package_rescue_avg_pull_margin",
            "avg_package_rescue_avg_timed_pull_margin",
            "avg_package_rescue_pull_snap_bursts", "avg_package_rescue_perfect_pull_snap_bursts",
            "avg_package_rescue_pull_total_frames", "avg_package_rescue_pull_uptime_rate",
            "avg_package_rescue_taunt_safe_frames", "avg_package_rescue_taunt_strain_frames",
            "avg_package_rescue_taunt_strain_rate", "avg_package_rescue_taunt_strain_breaks",
            "avg_package_rescue_taunt_strain_pulses",
            "avg_package_rescue_counter_windows", "avg_package_rescue_counter_damage_total",
            "avg_package_rescue_counter_stagger_frames",
            "avg_package_rescue_misses",
            "avg_package_rescue_fatal_misses", "avg_package_rescue_survived_misses",
            "avg_package_pressure_max_count", "avg_package_pressure_overlap_rate",
            "avg_package_pressure_sync_frames", "avg_package_pressure_sync_rate",
            "avg_package_pressure_sync_bursts", "avg_package_pressure_sync_peak",
            "avg_package_pressure_unique_components", "avg_package_pressure_signature_switches",
            "avg_package_pressure_variety_frames", "avg_package_pressure_dominant_component_frames",
            "avg_package_pressure_dominance_rate",
            "avg_package_micro_score", "avg_package_micro_best_streak",
            "avg_package_micro_hits", "avg_package_micro_dodges",
            "avg_package_micro_thread_cues", "avg_package_micro_streak_bursts",
            "avg_package_micro_counter_windows", "avg_package_micro_counter_damage_total",
            "avg_package_micro_counter_stagger_frames",
            "avg_package_micro_punish_total", "avg_package_micro_punish_events",
            "avg_package_micro_punish_break_total", "avg_package_micro_punish_break_events",
            "avg_package_sweep_warning_count", "avg_package_sweep_active_count",
        })
        write_csv(args.spell_stats_out, spell_stat_rows, {
            "class", "start_level", "loadout", "bot_profile", "spell", "class_runs", "runs_using", "use_rate",
            "total_orders", "avg_orders_per_run", "avg_orders_per_using_run",
            "max_orders_in_run",
        })
        write_csv(args.enemy_stats_out, enemy_stat_rows, {
            "class", "start_level", "loadout", "bot_profile", "monster", "runs", "runs_visible", "runs_killing",
            "xpaward", "kills", "visible_enemy_frames",
            "runs_alive_at_end", "alive_at_end", "end_hp", "end_max_hp",
            "avg_kills_per_run", "avg_visible_enemy_frames_per_run",
            "avg_alive_at_end_per_run", "avg_end_hp_per_run", "avg_end_hp_pct_when_alive",
            "kill_share", "visible_frame_share", "xp_from_kills", "xp_share",
            "kills_per_1000_visible_enemy_frames",
            "kills_per_1000_controlled_frames",
            "kills_per_1000_combat_frames",
        })
        write_csv(args.report_out, report_rows, {
            "severity", "category", "class", "metric", "value", "baseline",
            "delta", "delta_pct", "drill_id", "drill_label", "scorecard_id",
            "target", "comparator", "threshold", "passed", "reason", "message",
        })
        if args.json_out and args.json_out ~= "" then
            file_dump_json(args.json_out, {
                schema_version = 21,
                seed_mode = seed_mode,
                start_map_desc = args.start_map_desc,
                map_profile = args.map_profile,
                encounter = args.encounter,
                monsters = args.monsters,
                package_intensity = args.package_intensity,
                package_components = args.package_components,
                bot_profile = args.bot_profile,
                loadout_preset = loadout_preset,
                loadout = loadout.name,
                loadout_specs_by_class = specs_by_class,
                runs = json_runs,
                class_stats = class_rows,
                spell_stats = spell_stat_rows,
                damage_stats = damage_stat_rows,
                damage_dealt_stats = damage_dealt_stat_rows,
                objective_damage_stats = objective_damage_stat_rows,
                enemy_stats = enemy_stat_rows,
                balance_report = report_rows,
            })
        end
        print(string.format(
            "Balance run complete: %d runs, %d samples. Wrote %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s.",
            #summaries,
            #samples,
            args.out,
            args.samples_out,
            args.spells_out,
            args.damage_stats_out,
            args.damage_dealt_stats_out,
            args.objective_damage_stats_out,
            args.aggregate_out,
            args.spell_stats_out,
            args.enemy_stats_out,
            args.report_out,
            args.json_out
        ))
        for idx, row in ipairs(report_rows) do
            if idx > 10 then
                print(string.format("... %d more report rows in %s", #report_rows - 10, args.report_out))
                break
            end
            print(string.format("[%s] %s", row.severity, row.message))
        end
        return nil
    end

    local function next_run()
        local run = runs[run_index]
        run_index = run_index + 1
        if not run then
            return finish_all()
        end

        local telemetry = {
            spell_orders = {},
            spell_slot_orders = {},
            item_orders = {},
            monster_kills = {},
            visible_enemy_frames = {},
            damage_taken_by_source = {},
            damage_events_by_source = {},
            damage_dealt_by_target = {},
            damage_dealt_events_by_target = {},
            damage_dealt_by_source = {},
            damage_dealt_events_by_source = {},
            objective_damage_by_source = {},
            objective_damage_events_by_source = {},
        }
        local run_state = {done = false, outcome = "step_limit"}
        local bot_options = {
            seed = run.seed + 991,
            spell_policy = args.spell_policy,
            skill_profile = args.bot_profile,
            start_map_desc = args.start_map_desc,
            map_profile = args.map_profile,
            encounter = args.encounter,
            monsters = args.monsters,
            package_intensity = args.package_intensity,
            package_components = args.package_components,
            objective_enemy = parse_bool(args.hunt_objective) and args.objective_enemy or "",
        }

        if not Events then
            Events = require "Events"
            base_player_death = Events.events.PlayerDeath
            base_monster_death = Events.events.MonsterDeath
        end
        install_run_hooks(Events, base_player_death, base_monster_death, run_state, telemetry)
        game_init(run, telemetry, bot_options)
        sample_run(run, telemetry, samples, "start")

        local function step()
            load_core_modules()
            local GameLoop = require "GameLoop"
            if run_state.done then
                sample_run(run, telemetry, samples, run_state.outcome)
                local summary = summarize_run(run, telemetry, run_state.outcome)
                summaries[#summaries + 1] = summary
                json_runs[#json_runs + 1] = {summary = summary, telemetry = telemetry}
                for _, row in ipairs(flatten_spell_rows(run, telemetry)) do
                    spell_rows[#spell_rows + 1] = row
                end
                for _, row in ipairs(flatten_damage_rows(run, telemetry)) do
                    damage_rows[#damage_rows + 1] = row
                end
                for _, row in ipairs(flatten_damage_dealt_target_rows(run, telemetry)) do
                    damage_dealt_target_rows[#damage_dealt_target_rows + 1] = row
                end
                for _, row in ipairs(flatten_damage_rows(run, telemetry, "objective_damage_by_source", "objective_damage_events_by_source")) do
                    objective_damage_rows[#objective_damage_rows + 1] = row
                end
                print(string.format(
                    "Run %d seed=%s class=%s start_level=%d outcome=%s frames=%d level=%d kills=%d",
                    summary.run_id,
                    summary.seed,
                    summary.class,
                    summary.start_level,
                    summary.outcome,
                    summary.frames,
                    summary.level,
                    summary.kills
                ))
                return next_run
            end

            if max_steps and GameState.frame >= max_steps then
                run_state.outcome = "step_limit"
                run_state.done = true
                return step
            end
            if sample_interval and sample_interval > 0 and GameState.frame > 0
                and GameState.frame % sample_interval == 0 then
                sample_run(run, telemetry, samples, "interval")
            end
            if draw_interval and draw_interval > 0 and GameState.frame > 0
                and GameState.frame % draw_interval == 0 then
                GameState.draw()
            end

            if not GameLoop.game_step(true) then
                run_state.outcome = "game_loop_exit"
                run_state.done = true
                return step
            end

            local entry = World.players[1]
            local player = entry and entry.instance
            attach_objective_damage_tracker(player, args.objective_enemy, telemetry)
            attach_damage_dealt_trackers(player, telemetry)
            local package_state = content_lab_package_state(player)
            local package_run_outcome = package_state and package_outcome_to_run_outcome(package_state.outcome)
            if not run_state.done and package_run_outcome then
                run_state.outcome = package_run_outcome
                run_state.done = true
                return step
            end
            if not run_state.done and args.objective_enemy and args.objective_enemy ~= ""
                and player and hostile_alive_enemy_named_count(player, args.objective_enemy) == 0 then
                run_state.outcome = "cleared"
                run_state.done = true
                return step
            end
            if not run_state.done and (args.start_map_desc and args.start_map_desc ~= ""
                or content_lab_arena_requested(args) and not package_state)
                and player and hostile_alive_enemy_count(player) == 0 then
                run_state.outcome = "cleared"
                run_state.done = true
                return step
            end

            local source = player and player.input_source
            if source and source.should_end and source:should_end() then
                run_state.outcome = "bot_stalled"
                run_state.done = true
            end
            return step
        end
        return step
    end

    game_start = function()
        return ResourceLoading.ensure_resources_before(function()
            return next_run()
        end)
    end

    return StartEngine.start_engine {
        settings = {
            fullscreen = false,
            view_width = 640,
            view_height = 480,
            font = "fonts/Gudea-Regular.ttf",
            menu_font = "fonts/alagard_by_pix3m-d6awiwp.ttf",
            time_per_step = 0,
            invincible = parse_bool(args.invincible),
            regen_on_death = false,
            autouse_health_potions = true,
            autouse_mana_potions = true,
        },
        entry_point = game_start,
        on_exit = function() end,
    }
end

return run_balance
