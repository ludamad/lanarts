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

local function group_key(class_name, start_level)
    return tostring(class_name or "") .. "\n" .. tostring(start_level or 1)
end

local function parse_bool(value)
    value = tostring(value or ""):lower()
    return value == "1" or value == "true" or value == "yes" or value == "on"
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

local function sorted_group_keys(groups)
    local keys = {}
    for key, _ in pairs(groups) do
        keys[#keys + 1] = key
    end
    table.sort(keys)
    return keys
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
    local necromancer_pain_orders = telemetry.necromancer_pain_orders or 0
    samples[#samples + 1] = {
        run_id = run.run_id,
        seed = run.seed,
        class = run.class_name,
        start_level = run.start_level or 1,
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
        kills = snap.kills,
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
    }
end

local function summarize_run(run, telemetry, outcome)
    local snap = player_snapshot() or {}
    local spell_orders_total = count_values(telemetry.spell_orders)
    local necromancer_pain_orders = telemetry.necromancer_pain_orders or 0
    return {
        run_id = run.run_id,
        seed = run.seed,
        class = run.class_name,
        start_level = run.start_level or 1,
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
        kills = snap.kills or 0,
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
        max_same_square_frames = telemetry.max_same_square_frames or 0,
        max_visible_enemies = telemetry.max_visible_enemies or 0,
        spell_orders = encode_counts(telemetry.spell_orders),
        item_orders = encode_counts(telemetry.item_orders),
        monster_kills = encode_counts(telemetry.monster_kills),
    }
end

local function flatten_spell_rows(run, telemetry)
    local rows = {}
    for spell, count in pairs(telemetry.spell_orders or {}) do
        rows[#rows + 1] = {
            run_id = run.run_id,
            seed = run.seed,
            class = run.class_name,
            start_level = run.start_level or 1,
            spell = spell,
            orders = count,
        }
    end
    return rows
end

local SUMMARY_METRICS = {
    "frames", "level", "xp", "hp_pct", "mp_pct", "kills", "deaths",
    "deepest_floor", "current_floor", "spell_orders_total", "ability_orders_total", "weapon_orders",
    "item_orders_total", "controlled_frames", "combat_frames",
    "explore_frames", "rest_frames", "idle_frames", "random_walk_frames",
    "unstick_frames", "portal_stop_frames", "necromancer_close_frames",
    "necromancer_pain_orders", "necromancer_pain_during_regen_orders",
    "necromancer_regen_combat_frames", "necromancer_retreat_frames",
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
        local key = group_key(class_name, start_level)
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = class_name,
                start_level = start_level,
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
        row.combat_frame_rate = format_rate((aggregate.sums.combat_frames or 0) / controlled_frames)
        row.explore_frame_rate = format_rate((aggregate.sums.explore_frames or 0) / controlled_frames)
        row.rest_frame_rate = format_rate((aggregate.sums.rest_frames or 0) / controlled_frames)
        row.idle_frame_rate = format_rate((aggregate.sums.idle_frames or 0) / controlled_frames)
        row.portal_stop_frame_rate = format_rate((aggregate.sums.portal_stop_frames or 0) / controlled_frames)
        rows[#rows + 1] = row
    end
    return rows
end

local function class_run_counts(summaries)
    local counts = {}
    for _, row in ipairs(summaries) do
        local key = group_key(row.class, row.start_level)
        counts[key] = (counts[key] or 0) + 1
    end
    return counts
end

local function aggregate_spell_rows(spell_rows, summaries)
    local run_counts = class_run_counts(summaries)
    local groups = {}
    for _, row in ipairs(spell_rows) do
        local run_group = group_key(row.class, row.start_level)
        local key = run_group .. "\n" .. tostring(row.spell or "")
        local aggregate = groups[key]
        if not aggregate then
            aggregate = {
                class = row.class,
                start_level = tonumber(row.start_level) or 1,
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

local function metric_baselines(class_rows, start_level)
    local baselines = {}
    for _, metric in ipairs(REPORT_METRICS) do
        local total = 0
        local count = 0
        for _, row in ipairs(class_rows) do
            if not start_level or (tonumber(row.start_level) or 1) == start_level then
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
    if start_level > 1 then
        return string.format("%s L%d", class_name, start_level)
    end
    return class_name
end

local function balance_report_rows(class_rows, min_runs, relative_threshold)
    local rows = {}
    local baselines_by_level = {}
    min_runs = min_runs or 3
    relative_threshold = relative_threshold or 0.25

    for _, row in ipairs(class_rows) do
        local class_name = report_class_name(row)
        local start_level = tonumber(row.start_level) or 1
        baselines_by_level[start_level] = baselines_by_level[start_level]
            or metric_baselines(class_rows, start_level)
        local baselines = baselines_by_level[start_level]
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
    EngineInternal.start_game()
    local entry = World.players[1]
    local player = entry and entry.instance
    for _ = 2, (run.start_level or 1) do
        if player then
            player:gain_level()
        end
    end
    if player then
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
        telemetry.monster_kills = telemetry.monster_kills or {}
        telemetry.monster_kills[monster.name] = (telemetry.monster_kills[monster.name] or 0) + 1
        if base_monster_death then
            return base_monster_death(monster)
        end
    end

    EngineHooks.game_won = function()
        run_state.outcome = "won"
        run_state.done = true
    end
end

local function build_runs(classes, levels, runs_per_class, seed)
    local runs = {}
    local run_id = 1
    for _, class_name in ipairs(classes) do
        for _, start_level in ipairs(levels) do
            for idx = 1, runs_per_class do
                runs[#runs + 1] = {
                    run_id = run_id,
                    class_name = class_name,
                    start_level = start_level,
                    seed = seed + run_id - 1,
                    class_run_index = idx,
                }
                run_id = run_id + 1
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
    parser:option("--classes", "Comma-separated classes to run.", DEFAULT_CLASSES)
    parser:option("--levels", "Comma-separated starting levels to run.", "1")
    parser:option("--spell_policy", "Spell selection policy: mixed, priority, round_robin, none.", "mixed")
    parser:option("--sample_interval", "Frame interval for timeseries samples.", "300")
    parser:option("--out", "Summary CSV output.", "saves/balance_runs.csv")
    parser:option("--samples_out", "Timeseries sample CSV output.", "saves/balance_samples.csv")
    parser:option("--spells_out", "Spell order CSV output.", "saves/balance_spells.csv")
    parser:option("--aggregate_out", "Class aggregate CSV output.", "saves/balance_classes.csv")
    parser:option("--spell_stats_out", "Spell aggregate CSV output.", "saves/balance_spell_stats.csv")
    parser:option("--report_out", "Balance report CSV output.", "saves/balance_report.csv")
    parser:option("--min_report_runs", "Minimum runs per class before balance flags are considered stable.", "3")
    parser:option("--report_delta_pct", "Relative delta threshold for class-balance report flags.", "0.25")
    parser:option("--json_out", "Full JSON output.", "saves/balance_runs.json")
    parser:option("--invincible", "Run with invincibility enabled: true/false.", "false")
    local args = parser:parse(raw_args)

    local max_steps = tonumber(args.steps)
    local sample_interval = tonumber(args.sample_interval)
    local runs = build_runs(split_csv(args.classes), parse_levels(args.levels), tonumber(args.runs), tonumber(args.seed))
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
        local report_rows = balance_report_rows(class_rows, tonumber(args.min_report_runs), tonumber(args.report_delta_pct))
        write_csv(args.out, summaries, {
            "run_id", "seed", "class", "start_level", "outcome", "frames", "level", "xp",
            "hp", "max_hp", "hp_pct", "mp", "max_mp", "mp_pct", "kills",
            "deaths", "deepest_floor", "current_floor", "map_label",
            "enemies_alive", "visible_enemies", "spells_known",
            "spell_orders_total", "ability_orders_total", "weapon_orders", "item_orders_total",
        })
        write_csv(args.samples_out, samples, {
            "run_id", "seed", "class", "start_level", "frame", "reason", "level", "xp",
            "hp", "max_hp", "hp_pct", "mp", "max_mp", "mp_pct", "kills",
            "deaths", "deepest_floor", "current_floor", "map_label",
            "enemies_alive", "visible_enemies", "spells_known",
            "spell_orders_total", "ability_orders_total", "weapon_orders", "item_orders_total",
        })
        write_csv(args.spells_out, spell_rows, {"run_id", "seed", "class", "start_level", "spell", "orders"})
        write_csv(args.aggregate_out, class_rows, {
            "class", "start_level", "runs", "wins", "deaths", "step_limits", "bot_stalls",
            "game_loop_exits", "win_rate", "death_rate", "outcome_counts",
        })
        write_csv(args.spell_stats_out, spell_stat_rows, {
            "class", "start_level", "spell", "class_runs", "runs_using", "use_rate",
            "total_orders", "avg_orders_per_run", "avg_orders_per_using_run",
            "max_orders_in_run",
        })
        write_csv(args.report_out, report_rows, {
            "severity", "category", "class", "metric", "value", "baseline",
            "delta", "delta_pct", "message",
        })
        if args.json_out and args.json_out ~= "" then
            file_dump_json(args.json_out, {
                schema_version = 1,
                runs = json_runs,
                class_stats = class_rows,
                spell_stats = spell_stat_rows,
                balance_report = report_rows,
            })
        end
        print(string.format(
            "Balance run complete: %d runs, %d samples. Wrote %s, %s, %s, %s, %s, %s, %s.",
            #summaries,
            #samples,
            args.out,
            args.samples_out,
            args.spells_out,
            args.aggregate_out,
            args.spell_stats_out,
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
        }
        local run_state = {done = false, outcome = "step_limit"}
        local bot_options = {
            seed = run.seed + 991,
            spell_policy = args.spell_policy,
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
                    "Run %d seed=%d class=%s start_level=%d outcome=%s frames=%d level=%d kills=%d",
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
