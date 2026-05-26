local argparse = require "argparse"
local EngineInternal = require "core.EngineInternal"
local ResourceLoading = require "engine.ResourceLoading"
local StartEngine = require "engine.StartEngine"
local EngineHooks = require "engine.EngineBase"

local DEFAULT_CLASSES = "Fighter,Templar,Rogue,Necromancer,Pyrocaster,Stormcaller"
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

local function group_key(class_name, start_level, loadout)
    return table.concat({
        tostring(class_name or ""),
        tostring(start_level or 1),
        tostring(loadout or ""),
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
            "White Robe",
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
        error("Invalid --loadout_preset '" .. preset .. "'. Expected one of: late, none.")
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

local function visible_enemy_count(player)
    load_core_modules()
    local count = 0
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if not enemy.destroyed and Map.object_visible(enemy, enemy.xy, player) then
            count = count + 1
        end
    end
    return count
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
    return {
        player = player,
        map_label = map_label,
        enemies_alive = #enemies,
        visible_enemies = visible_enemy_count(player),
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

local function sample_run(run, telemetry, samples, reason)
    local snap = player_snapshot()
    if not snap then
        return
    end
    local spell_orders_total = count_values(telemetry.spell_orders)
    local hostile_kills = count_values(telemetry.monster_kills)
    local necromancer_pain_orders = telemetry.necromancer_pain_orders or 0
    samples[#samples + 1] = {
        run_id = run.run_id,
        seed = tostring(run.seed),
        seed_group = run.seed_group,
        class_run_index = run.class_run_index,
        class = run.class_name,
        start_level = run.start_level or 1,
        loadout = run.loadout_name or "",
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
        spells_known = snap.spells_known,
        spell_orders_total = spell_orders_total,
        ability_orders_total = spell_orders_total + necromancer_pain_orders,
        weapon_orders = telemetry.weapon_orders or 0,
        item_orders_total = count_values(telemetry.item_orders),
        necromancer_close_frames = telemetry.necromancer_close_frames or 0,
        necromancer_pain_orders = necromancer_pain_orders,
        necromancer_pain_during_regen_orders = telemetry.necromancer_pain_during_regen_orders or 0,
        necromancer_regen_combat_frames = telemetry.necromancer_regen_combat_frames or 0,
        necromancer_retreat_frames = telemetry.necromancer_retreat_frames or 0,
        pyrocaster_ring_combat_frames = telemetry.pyrocaster_ring_combat_frames or 0,
        pyrocaster_ring_close_frames = telemetry.pyrocaster_ring_close_frames or 0,
        pyrocaster_ring_reposition_frames = telemetry.pyrocaster_ring_reposition_frames or 0,
    }
end

local function summarize_run(run, telemetry, outcome)
    local snap = player_snapshot() or {}
    local spell_orders_total = count_values(telemetry.spell_orders)
    local hostile_kills = count_values(telemetry.monster_kills)
    local necromancer_pain_orders = telemetry.necromancer_pain_orders or 0
    return {
        run_id = run.run_id,
        seed = tostring(run.seed),
        seed_group = run.seed_group,
        class_run_index = run.class_run_index,
        class = run.class_name,
        start_level = run.start_level or 1,
        loadout = run.loadout_name or "",
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
        deepest_floor = snap.deepest_floor or 0,
        current_floor = snap.current_floor or 0,
        map_label = snap.map_label or "",
        enemies_alive = snap.enemies_alive or 0,
        visible_enemies = snap.visible_enemies or 0,
        spells_known = snap.spells_known or "",
        spell_orders_total = spell_orders_total,
        ability_orders_total = spell_orders_total + necromancer_pain_orders,
        weapon_orders = telemetry.weapon_orders or 0,
        item_orders_total = count_values(telemetry.item_orders),
        controlled_frames = telemetry.controlled_frames or 0,
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
            spell = spell,
            orders = count,
        }
    end
    return rows
end

local SUMMARY_METRICS = {
    "frames", "level", "xp", "hp_pct", "mp_pct", "kills", "player_kills", "deaths",
    "deepest_floor", "current_floor", "spell_orders_total", "ability_orders_total", "weapon_orders",
    "item_orders_total", "controlled_frames", "combat_frames",
    "explore_frames", "rest_frames", "idle_frames", "random_walk_frames",
    "unstick_frames", "portal_stop_frames", "necromancer_close_frames",
    "necromancer_pain_orders", "necromancer_pain_during_regen_orders",
    "necromancer_regen_combat_frames", "necromancer_retreat_frames",
    "pyrocaster_ring_combat_frames", "pyrocaster_ring_close_frames",
    "pyrocaster_ring_reposition_frames",
    "max_same_square_frames", "max_visible_enemies",
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
        local key = group_key(class_name, start_level, loadout)
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = class_name,
                start_level = start_level,
                loadout = loadout,
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
        local row = {
            class = aggregate.class,
            start_level = aggregate.start_level,
            loadout = aggregate.loadout,
            runs = aggregate.runs,
            wins = aggregate.outcomes.won or 0,
            deaths = aggregate.outcomes.death or 0,
            step_limits = aggregate.outcomes.step_limit or 0,
            bot_stalls = aggregate.outcomes.bot_stalled or 0,
            game_loop_exits = aggregate.outcomes.game_loop_exit or 0,
            win_rate = format_rate((aggregate.outcomes.won or 0) / runs),
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
        row.combat_frame_rate = format_rate(combat_frames / controlled_frames)
        row.explore_frame_rate = format_rate((aggregate.sums.explore_frames or 0) / controlled_frames)
        row.rest_frame_rate = format_rate((aggregate.sums.rest_frames or 0) / controlled_frames)
        row.idle_frame_rate = format_rate((aggregate.sums.idle_frames or 0) / controlled_frames)
        row.portal_stop_frame_rate = format_rate((aggregate.sums.portal_stop_frames or 0) / controlled_frames)
        row.kills_per_1000_controlled_frames = format_rate(kills / controlled_frames * 1000)
        row.kills_per_1000_combat_frames = format_rate(combat_frames > 0 and kills / combat_frames * 1000 or 0)
        rows[#rows + 1] = row
    end
    return rows
end

local function class_run_counts(summaries)
    local counts = {}
    for _, row in ipairs(summaries) do
        local key = group_key(row.class, row.start_level, row.loadout)
        counts[key] = (counts[key] or 0) + 1
    end
    return counts
end

local function aggregate_spell_rows(spell_rows, summaries)
    local run_counts = class_run_counts(summaries)
    local groups = {}
    for _, row in ipairs(spell_rows) do
        local run_group = group_key(row.class, row.start_level, row.loadout)
        local key = run_group .. "\n" .. tostring(row.spell or "")
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = row.class,
                start_level = tonumber(row.start_level) or 1,
                loadout = row.loadout or "",
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

local function aggregate_enemy_rows(summaries)
    local class_totals = {}
    local enemy_groups = {}

    for _, row in ipairs(summaries) do
        local class_name = row.class or ""
        local start_level = tonumber(row.start_level) or 1
        local loadout = row.loadout or ""
        local run_group = group_key(class_name, start_level, loadout)
        local class_total = class_totals[run_group]
        if not class_total then
            class_total = {
                class = class_name,
                start_level = start_level,
                loadout = loadout,
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

        for monster, _ in pairs(seen) do
            local key = run_group .. "\n" .. monster
            local group = enemy_groups[key]
            if not group then
                group = {
                    class = class_name,
                    start_level = start_level,
                    loadout = loadout,
                    monster = monster,
                    run_group = run_group,
                    runs_visible = 0,
                    runs_killing = 0,
                    kills = 0,
                    visible_enemy_frames = 0,
                    xpaward = enemy_xpaward(monster),
                    xp_from_kills = 0,
                }
                enemy_groups[key] = group
            end

            local kill_count = kills[monster] or 0
            local frame_count = visible_frames[monster] or 0
            if kill_count > 0 then
                group.runs_killing = group.runs_killing + 1
            end
            if frame_count > 0 then
                group.runs_visible = group.runs_visible + 1
            end
            group.kills = group.kills + kill_count
            group.visible_enemy_frames = group.visible_enemy_frames + frame_count
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
            monster = group.monster,
            runs = class_total.runs or 0,
            runs_visible = group.runs_visible,
            runs_killing = group.runs_killing,
            xpaward = group.xpaward,
            kills = group.kills,
            visible_enemy_frames = group.visible_enemy_frames,
            avg_kills_per_run = format_rate(group.kills / runs),
            avg_visible_enemy_frames_per_run = format_rate(group.visible_enemy_frames / runs),
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
        key = "kills_per_1000_combat_frames",
        label = "kills per 1000 combat frames",
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

local function metric_baselines(class_rows, start_level, loadout)
    local baselines = {}
    for _, metric in ipairs(BASELINE_METRICS) do
        local total = 0
        local count = 0
        for _, row in ipairs(class_rows) do
            local same_level = not start_level or (tonumber(row.start_level) or 1) == start_level
            local same_loadout = (row.loadout or "") == (loadout or "")
            if same_level and same_loadout then
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
    rows[#rows + 1] = {
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
end

local function report_class_name(row)
    local class_name = row.class or ""
    local start_level = tonumber(row.start_level) or 1
    local loadout = row.loadout or ""
    local suffix = loadout ~= "" and (" " .. loadout) or ""
    if start_level > 1 then
        return string.format("%s L%d%s", class_name, start_level, suffix)
    end
    return class_name .. suffix
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
        local baseline_key = tostring(start_level) .. "\n" .. loadout
        baselines_by_group[baseline_key] = baselines_by_group[baseline_key]
            or metric_baselines(class_rows, start_level, loadout)
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

local function generate_start_map_desc(desc_name)
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
    if bot_options.start_map_desc and bot_options.start_map_desc ~= "" then
        EngineHooks.first_map_create = function()
            return generate_start_map_desc(bot_options.start_map_desc)
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

local function add_run(runs, run_id, class_name, start_level, seed, seed_group, class_run_index, loadout)
    runs[#runs + 1] = {
        run_id = run_id,
        class_name = class_name,
        start_level = start_level,
        seed = seed,
        seed_group = seed_group,
        class_run_index = class_run_index,
        loadout_name = loadout.name,
        loadout_specs = loadout.specs_by_class[class_name] or {},
    }
end

local function build_runs(classes, levels, runs_per_class, seed, seed_mode, loadout)
    local runs = {}
    local run_id = 1
    if seed_mode == "unique" then
        for _, class_name in ipairs(classes) do
            for _, start_level in ipairs(levels) do
                for idx = 1, runs_per_class do
                    add_run(runs, run_id, class_name, start_level, seed + run_id - 1, run_id, idx, loadout)
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
                    add_run(runs, run_id, class_name, start_level, paired_seed, seed_group, idx, loadout)
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
    parser:option("--loadout_preset", "Equipment preset for each class: none or late.", "none")
    parser:option("--loadout", "Comma-separated item specs to add to every run, using Item or Item*COUNT.", "")
    parser:option("--loadout_label", "Optional label for this loadout in outputs.", "")
    parser:option("--spell_policy", "Spell selection policy: mixed, priority, round_robin, none.", "mixed")
    parser:option("--sample_interval", "Frame interval for timeseries samples.", "300")
    parser:option("--out", "Summary CSV output.", "saves/balance_runs.csv")
    parser:option("--samples_out", "Timeseries sample CSV output.", "saves/balance_samples.csv")
    parser:option("--spells_out", "Spell order CSV output.", "saves/balance_spells.csv")
    parser:option("--aggregate_out", "Class aggregate CSV output.", "saves/balance_classes.csv")
    parser:option("--spell_stats_out", "Spell aggregate CSV output.", "saves/balance_spell_stats.csv")
    parser:option("--enemy_stats_out", "Enemy aggregate CSV output.", "saves/balance_enemy_stats.csv")
    parser:option("--report_out", "Balance report CSV output.", "saves/balance_report.csv")
    parser:option("--min_report_runs", "Minimum runs per class before balance flags are considered stable.", "3")
    parser:option("--report_delta_pct", "Relative delta threshold for class-balance report flags.", "0.25")
    parser:option("--json_out", "Full JSON output.", "saves/balance_runs.json")
    parser:option("--invincible", "Run with invincibility enabled: true/false.", "false")
    local args = parser:parse(raw_args)

    local max_steps = tonumber(args.steps)
    local sample_interval = tonumber(args.sample_interval)
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
    local runs = build_runs(classes, parse_levels(args.levels), tonumber(args.runs), tonumber(args.seed), seed_mode, loadout)
    local summaries = {}
    local samples = {}
    local spell_rows = {}
    local json_runs = {}
    local Events
    local base_player_death
    local base_monster_death

    local run_index = 1
    local game_start

    local function finish_all()
        local class_rows = aggregate_summary_rows(summaries)
        local spell_stat_rows = aggregate_spell_rows(spell_rows, summaries)
        local enemy_stat_rows = aggregate_enemy_rows(summaries)
        local report_rows = balance_report_rows(class_rows, tonumber(args.min_report_runs), tonumber(args.report_delta_pct))
        write_csv(args.out, summaries, {
            "run_id", "seed", "seed_group", "class_run_index", "class", "start_level", "loadout", "outcome", "frames", "level", "xp",
            "hp", "max_hp", "hp_pct", "mp", "max_mp", "mp_pct", "kills",
            "player_kills", "deaths", "deepest_floor", "current_floor", "map_label",
            "enemies_alive", "visible_enemies", "spells_known",
            "spell_orders_total", "ability_orders_total", "weapon_orders", "item_orders_total",
            "monster_kills", "visible_enemy_frames",
        })
        write_csv(args.samples_out, samples, {
            "run_id", "seed", "seed_group", "class_run_index", "class", "start_level", "loadout", "frame", "reason", "level", "xp",
            "hp", "max_hp", "hp_pct", "mp", "max_mp", "mp_pct", "kills",
            "player_kills", "deaths", "deepest_floor", "current_floor", "map_label",
            "enemies_alive", "visible_enemies", "spells_known",
            "spell_orders_total", "ability_orders_total", "weapon_orders", "item_orders_total",
        })
        write_csv(args.spells_out, spell_rows, {"run_id", "seed", "seed_group", "class_run_index", "class", "start_level", "loadout", "spell", "orders"})
        write_csv(args.aggregate_out, class_rows, {
            "class", "start_level", "loadout", "runs", "wins", "deaths", "step_limits", "bot_stalls",
            "game_loop_exits", "win_rate", "death_rate", "outcome_counts",
            "combat_frame_rate", "explore_frame_rate", "rest_frame_rate",
            "idle_frame_rate", "portal_stop_frame_rate",
            "kills_per_1000_controlled_frames", "kills_per_1000_combat_frames",
        })
        write_csv(args.spell_stats_out, spell_stat_rows, {
            "class", "start_level", "loadout", "spell", "class_runs", "runs_using", "use_rate",
            "total_orders", "avg_orders_per_run", "avg_orders_per_using_run",
            "max_orders_in_run",
        })
        write_csv(args.enemy_stats_out, enemy_stat_rows, {
            "class", "start_level", "loadout", "monster", "runs", "runs_visible", "runs_killing",
            "xpaward", "kills", "visible_enemy_frames",
            "avg_kills_per_run", "avg_visible_enemy_frames_per_run",
            "kill_share", "visible_frame_share", "xp_from_kills", "xp_share",
            "kills_per_1000_visible_enemy_frames",
            "kills_per_1000_controlled_frames",
            "kills_per_1000_combat_frames",
        })
        write_csv(args.report_out, report_rows, {
            "severity", "category", "class", "metric", "value", "baseline",
            "delta", "delta_pct", "message",
        })
        if args.json_out and args.json_out ~= "" then
            file_dump_json(args.json_out, {
                schema_version = 4,
                seed_mode = seed_mode,
                start_map_desc = args.start_map_desc,
                loadout_preset = loadout_preset,
                loadout = loadout.name,
                loadout_specs_by_class = specs_by_class,
                runs = json_runs,
                class_stats = class_rows,
                spell_stats = spell_stat_rows,
                enemy_stats = enemy_stat_rows,
                balance_report = report_rows,
            })
        end
        print(string.format(
            "Balance run complete: %d runs, %d samples. Wrote %s, %s, %s, %s, %s, %s, %s, %s.",
            #summaries,
            #samples,
            args.out,
            args.samples_out,
            args.spells_out,
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
        }
        local run_state = {done = false, outcome = "step_limit"}
        local bot_options = {
            seed = run.seed + 991,
            spell_policy = args.spell_policy,
            start_map_desc = args.start_map_desc,
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

            if not GameLoop.game_step(true) then
                run_state.outcome = "game_loop_exit"
                run_state.done = true
                return step
            end

            local entry = World.players[1]
            local player = entry and entry.instance
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
