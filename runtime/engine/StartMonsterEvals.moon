argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"
Map = require "core.Map"
World = require "core.World"

MAX_TIME = 10000000
CONFIGS = nil
DEFAULT_CONFIGS = "BaseFighter,BaseNecromancer,BaseStormcaller,BasePyrocaster"

split_csv = (value) ->
    result = {}
    for part in tostring(value or "")\gmatch("[^,]+")
        part = part\gsub("^%s+", "")\gsub("%s+$", "")
        append result, part if part ~= ""
    return result

format_rate = (value) ->
    string.format("%.3f", tonumber(value) or 0)

safe_div = (num, denom) ->
    return 0 if not denom or denom == 0
    (num or 0) / denom

normalize_key = (value) ->
    lowered = string.lower(tostring(value or ""))
    normalized = string.gsub(lowered, "[^%w]", "")
    return normalized

csv_escape = (value) ->
    return "" if value == nil
    value = tostring(value)
    if value\find('[,"\n]')
        return '"' .. value\gsub('"', '""') .. '"'
    return value

sorted_extra_keys = (rows, preferred) ->
    seen = {}
    extras = {}
    for key in *preferred
        seen[key] = true
    for row in *rows
        for key, _ in pairs(row)
            if not seen[key]
                seen[key] = true
                append extras, key
    table.sort(extras)
    keys = {}
    for key in *preferred
        append keys, key
    for key in *extras
        append keys, key
    return keys

write_csv = (path, rows, preferred_keys) ->
    return if not path or path == ""
    preferred_keys = preferred_keys or {}
    keys = sorted_extra_keys(rows, preferred_keys)
    lines = {table.concat(keys, ",")}
    for row in *rows
        values = {}
        for key in *keys
            append values, csv_escape(row[key])
        append lines, table.concat(values, ",")
    file_dump_string(path, table.concat(lines, "\n"))

monster_aliases = () ->
    aliases = {}
    for name in *table.sorted_key_list(enemies)
        aliases[normalize_key(name)] = name
    return aliases

parse_monster_names = (value) ->
    parts = split_csv(value)
    error("Need --monster NAME,LIST or --monster all.") if #parts == 0
    return table.sorted_key_list(enemies) if #parts == 1 and normalize_key(parts[1]) == "all"

    aliases = monster_aliases()
    names, seen = {}, {}
    for part in *parts
        resolved = aliases[normalize_key(part)]
        if not resolved
            error("Unknown monster '#{part}'. Use compact aliases like GiantRat, or --monster all.")
        if not seen[resolved]
            seen[resolved] = true
            append names, resolved
    return names

parse_config_names = (value) ->
    names = split_csv(value)
    return split_csv(DEFAULT_CONFIGS) if #names == 0
    names

parse_levels = (value) ->
    levels = {}
    for part in *split_csv(value)
        level = tonumber(part)
        append levels, math.floor(level) if level and level >= 1
    append levels, 1 if #levels == 0
    return levels

config_by_name = () ->
    result = {}
    for config in *CONFIGS
        result[config.name] = config
    return result

parse_configs = (value) ->
    by_name = config_by_name()
    selected = {}
    for name in *parse_config_names(value)
        config = by_name[name]
        error("Unknown config '#{name}'.") if not config
        append selected, config
    return selected

enemy_xpaward = (monster) ->
    entry = enemies and enemies[monster]
    return tonumber(entry and entry.xpaward) or 0

sorted_group_keys = (groups) ->
    keys = {}
    for key, _ in pairs(groups)
        append keys, key
    table.sort(keys)
    return keys

group_key = (config_name, level, monster) ->
    "#{config_name}\n#{level}\n#{monster}"

config_level_key = (config_name, level) ->
    "#{config_name}\n#{level}"

add_metric = (aggregate, row, metric) ->
    value = tonumber(row[metric]) or 0
    aggregate.sums[metric] = (aggregate.sums[metric] or 0) + value
    if aggregate.mins[metric] == nil or value < aggregate.mins[metric]
        aggregate.mins[metric] = value
    if aggregate.maxes[metric] == nil or value > aggregate.maxes[metric]
        aggregate.maxes[metric] = value

add_contact_metric = (aggregate, row, metric) ->
    value = tonumber(row[metric]) or 0
    aggregate.contact_sums[metric] = (aggregate.contact_sums[metric] or 0) + value
    if aggregate.contact_mins[metric] == nil or value < aggregate.contact_mins[metric]
        aggregate.contact_mins[metric] = value
    if aggregate.contact_maxes[metric] == nil or value > aggregate.contact_maxes[metric]
        aggregate.contact_maxes[metric] = value

SUMMARY_METRICS = {
    "frames", "player_damage", "gross_player_damage", "net_player_damage",
    "player_healing", "min_player_hp", "enemy_hp", "enemy_max_hp", "enemy_hp_pct",
    "defender_count", "attacker_count", "melee_count",
    "projectile_count", "receive_melee_count", "enemy_count_alive"
}

CONTACT_METRICS = {
    "defender_count", "attacker_count", "melee_count",
    "projectile_count", "receive_melee_count"
}

had_contact = (row) ->
    return true if (tonumber(row.player_damage) or 0) > 0
    for metric in *CONTACT_METRICS
        return true if (tonumber(row[metric]) or 0) > 0
    return false

aggregate_stats = (stats) ->
    groups = {}
    for row in *stats
        key = group_key(row.config_name, row.level, row.monster)
        aggregate = groups[key]
        if not aggregate
            aggregate = {
                config_name: row.config_name
                level: tonumber(row.level) or 1
                monster: row.monster
                runs: 0
                contact_runs: 0
                xpaward: tonumber(row.xpaward) or 0
                outcomes: {}
                sums: {}
                mins: {}
                maxes: {}
                contact_sums: {}
                contact_mins: {}
                contact_maxes: {}
            }
            groups[key] = aggregate
        aggregate.runs += 1
        contact = had_contact(row)
        aggregate.contact_runs += 1 if contact
        outcome = row.outcome or "unknown"
        if outcome == "step_limit" and not contact
            outcome = "no_contact_step_limit"
        aggregate.outcomes[outcome] = (aggregate.outcomes[outcome] or 0) + 1
        for metric in *SUMMARY_METRICS
            add_metric(aggregate, row, metric)
            add_contact_metric(aggregate, row, metric) if contact

    rows = {}
    for key in *sorted_group_keys(groups)
        aggregate = groups[key]
        runs = math.max(1, aggregate.runs)
        contact_runs = math.max(1, aggregate.contact_runs)
        avg_frames = safe_div(aggregate.sums.frames, runs)
        avg_damage = safe_div(aggregate.sums.player_damage, runs)
        damage_per_1000_frames = safe_div(aggregate.sums.player_damage, aggregate.sums.frames) * 1000
        damage_per_xp = safe_div(avg_damage, math.max(1, aggregate.xpaward))
        xp_per_100_damage = safe_div(aggregate.xpaward * 100, avg_damage)
        contact_avg_frames = safe_div(aggregate.contact_sums.frames, contact_runs)
        contact_avg_damage = safe_div(aggregate.contact_sums.player_damage, contact_runs)
        contact_damage_per_1000_frames = safe_div(aggregate.contact_sums.player_damage, aggregate.contact_sums.frames) * 1000
        contact_damage_per_xp = safe_div(contact_avg_damage, math.max(1, aggregate.xpaward))
        contact_xp_per_100_damage = safe_div(aggregate.xpaward * 100, contact_avg_damage)
        avg_gross_damage = safe_div(aggregate.sums.gross_player_damage, runs)
        avg_net_damage = safe_div(aggregate.sums.net_player_damage, runs)
        avg_healing = safe_div(aggregate.sums.player_healing, runs)
        avg_min_player_hp = safe_div(aggregate.sums.min_player_hp, runs)
        contact_avg_gross_damage = safe_div(aggregate.contact_sums.gross_player_damage, contact_runs)
        contact_avg_net_damage = safe_div(aggregate.contact_sums.net_player_damage, contact_runs)
        contact_avg_healing = safe_div(aggregate.contact_sums.player_healing, contact_runs)
        contact_avg_min_player_hp = safe_div(aggregate.contact_sums.min_player_hp, contact_runs)
        append rows, {
            config_name: aggregate.config_name
            level: aggregate.level
            monster: aggregate.monster
            runs: aggregate.runs
            contact_runs: aggregate.contact_runs
            contact_run_rate: format_rate safe_div(aggregate.contact_runs, runs)
            xpaward: aggregate.xpaward
            enemy_kill_rate: format_rate safe_div(aggregate.outcomes.enemy_killed or 0, runs)
            step_limit_rate: format_rate safe_div(aggregate.outcomes.step_limit or 0, runs)
            no_contact_step_limit_rate: format_rate safe_div(aggregate.outcomes.no_contact_step_limit or 0, runs)
            player_death_rate: format_rate safe_div(aggregate.outcomes.player_death or 0, runs)
            avg_frames: format_rate avg_frames
            min_frames: aggregate.mins.frames or 0
            max_frames: aggregate.maxes.frames or 0
            avg_player_damage: format_rate avg_damage
            min_player_damage: aggregate.mins.player_damage or 0
            max_player_damage: aggregate.maxes.player_damage or 0
            avg_gross_player_damage: format_rate avg_gross_damage
            avg_net_player_damage: format_rate avg_net_damage
            avg_player_healing: format_rate avg_healing
            avg_min_player_hp: format_rate avg_min_player_hp
            min_player_hp: aggregate.mins.min_player_hp or 0
            damage_per_1000_frames: format_rate damage_per_1000_frames
            damage_per_xp: format_rate damage_per_xp
            xp_per_100_damage: format_rate xp_per_100_damage
            contact_avg_frames: format_rate contact_avg_frames
            contact_min_frames: aggregate.contact_mins.frames or 0
            contact_max_frames: aggregate.contact_maxes.frames or 0
            contact_avg_player_damage: format_rate contact_avg_damage
            contact_min_player_damage: aggregate.contact_mins.player_damage or 0
            contact_max_player_damage: aggregate.contact_maxes.player_damage or 0
            contact_avg_gross_player_damage: format_rate contact_avg_gross_damage
            contact_avg_net_player_damage: format_rate contact_avg_net_damage
            contact_avg_player_healing: format_rate contact_avg_healing
            contact_avg_min_player_hp: format_rate contact_avg_min_player_hp
            contact_min_player_hp: aggregate.contact_mins.min_player_hp or 0
            contact_damage_per_1000_frames: format_rate contact_damage_per_1000_frames
            contact_damage_per_xp: format_rate contact_damage_per_xp
            contact_xp_per_100_damage: format_rate contact_xp_per_100_damage
            avg_enemy_hp_pct: format_rate safe_div(aggregate.sums.enemy_hp_pct, runs)
            avg_enemy_count_alive: format_rate safe_div(aggregate.sums.enemy_count_alive, runs)
            avg_defender_count: format_rate safe_div(aggregate.sums.defender_count, runs)
            avg_attacker_count: format_rate safe_div(aggregate.sums.attacker_count, runs)
            avg_melee_count: format_rate safe_div(aggregate.sums.melee_count, runs)
            avg_projectile_count: format_rate safe_div(aggregate.sums.projectile_count, runs)
            avg_receive_melee_count: format_rate safe_div(aggregate.sums.receive_melee_count, runs)
        }
    return rows

mean_by_config_level = (rows, metric) ->
    groups = {}
    for row in *rows
        value = tonumber(row[metric]) or 0
        continue if value <= 0
        key = config_level_key(row.config_name, row.level)
        group = groups[key]
        if not group
            group = {sum: 0, count: 0}
            groups[key] = group
        group.sum += value
        group.count += 1
    means = {}
    for key, group in pairs(groups)
        means[key] = safe_div(group.sum, group.count)
    return means

build_report = (aggregate_rows, delta_pct, min_report_runs) ->
    delta_pct = tonumber(delta_pct) or 0.5
    min_report_runs = tonumber(min_report_runs) or 1
    reports = {}
    damage_per_xp_mean = mean_by_config_level(aggregate_rows, "contact_damage_per_xp")
    damage_rate_mean = mean_by_config_level(aggregate_rows, "contact_damage_per_1000_frames")
    frames_mean = mean_by_config_level(aggregate_rows, "contact_avg_frames")

    for row in *aggregate_rows
        continue if (tonumber(row.runs) or 0) < min_report_runs
        key = config_level_key(row.config_name, row.level)
        kill_rate = tonumber(row.enemy_kill_rate) or 0
        step_limit_rate = tonumber(row.step_limit_rate) or 0
        no_contact_step_limit_rate = tonumber(row.no_contact_step_limit_rate) or 0
        damage_per_xp = tonumber(row.contact_damage_per_xp) or 0
        damage_rate = tonumber(row.contact_damage_per_1000_frames) or 0
        avg_frames = tonumber(row.contact_avg_frames) or 0
        xpaward = tonumber(row.xpaward) or 0
        threat_baseline = damage_per_xp_mean[key] or 0
        damage_baseline = damage_rate_mean[key] or 0
        frame_baseline = frames_mean[key] or 0

        if step_limit_rate > 0
            append reports, {
                severity: "warning"
                category: "toughness"
                config_name: row.config_name
                level: row.level
                monster: row.monster
                metric: "step_limit_rate"
                value: row.step_limit_rate
                baseline: "0.000"
                delta: row.step_limit_rate
                delta_pct: row.step_limit_rate
                message: "#{row.monster} survives to the step limit in #{row.step_limit_rate} of #{row.config_name} L#{row.level} runs."
            }

        if no_contact_step_limit_rate > 0
            append reports, {
                severity: "info"
                category: "contact"
                config_name: row.config_name
                level: row.level
                monster: row.monster
                metric: "no_contact_step_limit_rate"
                value: row.no_contact_step_limit_rate
                baseline: "0.000"
                delta: row.no_contact_step_limit_rate
                delta_pct: row.no_contact_step_limit_rate
                message: "#{row.monster} had no combat contact before the step limit in #{row.no_contact_step_limit_rate} of #{row.config_name} L#{row.level} runs."
            }

        if threat_baseline > 0 and damage_per_xp > threat_baseline * (1 + delta_pct)
            append reports, {
                severity: "warning"
                category: "reward"
                config_name: row.config_name
                level: row.level
                monster: row.monster
                metric: "contact_damage_per_xp"
                value: row.contact_damage_per_xp
                baseline: format_rate threat_baseline
                delta: format_rate(damage_per_xp - threat_baseline)
                delta_pct: format_rate safe_div(damage_per_xp - threat_baseline, threat_baseline)
                message: "#{row.monster} deals #{row.contact_damage_per_xp} contact damage per XP versus #{format_rate(threat_baseline)} baseline for #{row.config_name} L#{row.level}."
            }

        if xpaward <= 0 and damage_rate > 0
            append reports, {
                severity: "warning"
                category: "reward"
                config_name: row.config_name
                level: row.level
                monster: row.monster
                metric: "xpaward"
                value: xpaward
                baseline: ""
                delta: ""
                delta_pct: ""
                message: "#{row.monster} deals damage but awards no XP in #{row.config_name} L#{row.level} evals."
            }

        if damage_baseline > 0 and damage_rate > damage_baseline * (1 + delta_pct)
            append reports, {
                severity: "info"
                category: "threat"
                config_name: row.config_name
                level: row.level
                monster: row.monster
                metric: "contact_damage_per_1000_frames"
                value: row.contact_damage_per_1000_frames
                baseline: format_rate damage_baseline
                delta: format_rate(damage_rate - damage_baseline)
                delta_pct: format_rate safe_div(damage_rate - damage_baseline, damage_baseline)
                message: "#{row.monster} contact damage pace is #{row.contact_damage_per_1000_frames} per 1000 frames versus #{format_rate(damage_baseline)} baseline for #{row.config_name} L#{row.level}."
            }

        if kill_rate == 1 and frame_baseline > 0 and avg_frames > frame_baseline * (1 + delta_pct)
            append reports, {
                severity: "info"
                category: "durability"
                config_name: row.config_name
                level: row.level
                monster: row.monster
                metric: "contact_avg_frames"
                value: row.contact_avg_frames
                baseline: format_rate frame_baseline
                delta: format_rate(avg_frames - frame_baseline)
                delta_pct: format_rate safe_div(avg_frames - frame_baseline, frame_baseline)
                message: "#{row.monster} takes #{row.contact_avg_frames} contact frames to kill versus #{format_rate(frame_baseline)} baseline for #{row.config_name} L#{row.level}."
            }

    table.sort reports, (a, b) ->
        if a.severity != b.severity
            return a.severity > b.severity
        if a.category != b.category
            return a.category < b.category
        if a.config_name != b.config_name
            return a.config_name < b.config_name
        if tostring(a.level) != tostring(b.level)
            return tostring(a.level) < tostring(b.level)
        return a.monster < b.monster
    return reports

game_init = (seed, class_name, monster_spawns, input_func, level) ->
    input_source = (player) ->
        player\add_effect("DebugMonsterEvalEffect", MAX_TIME)
        for i=2,level
            player\gain_level()
        source = require("input.ProgrammableInputSource").create(player)
        source.step = input_func
        player.stats.max_hp = 10000
        player.stats.hp = 10000
        return source
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    GameState.register_player 'Playabot', class_name, -- Name and class
        input_source, -- input source
        true, 0, --netid
        0 -- team

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    random_seed(seed)
    Engine.first_map_create = () ->
        return require("map_descs.DebugArena")\generate_and_spawn(monster_spawns)
    EngineInternal.start_game()

    -- Set up input for first game step
    GameState.input_capture()
    GameState.input_handle()
    GameState.frame = 0

_iterate = (tbl, f, next) ->
    idx = 1
    iterate = () ->
        val = tbl[idx]
        idx += 1
        if val == nil
            return next()
        else
            return f(val, iterate)
    return iterate

active_player = () ->
    player_entry = World.players and World.players[1]
    return player_entry and player_entry.instance

make_run_state = () ->
    {
        gross_player_damage: 0
        player_healing: 0
        min_player_hp: nil
    }

capture_player_hp = (run_state) ->
    player = active_player()
    return nil unless run_state and player and player.stats
    hp = player.stats.hp
    if run_state.min_player_hp == nil or hp < run_state.min_player_hp
        run_state.min_player_hp = hp
    return hp

record_player_hp_delta = (run_state, before_hp) ->
    player = active_player()
    return unless run_state and before_hp and player and player.stats
    after_hp = player.stats.hp
    if after_hp < before_hp
        run_state.gross_player_damage += before_hp - after_hp
    elseif after_hp > before_hp
        run_state.player_healing += after_hp - before_hp
    if run_state.min_player_hp == nil or after_hp < run_state.min_player_hp
        run_state.min_player_hp = after_hp

necromancer_eval_input = () =>
    player = @player
    if player and player.stats
        max_hp = player\effective_stats().max_hp
        frame = require("core.GameState").frame or 0
        if player.stats.hp < max_hp and not player\has_effect("Baleful Regeneration")
            if (rawget(@, "regen_retry_at") or 0) <= frame
                @set("use_spell_slot", 0)
                rawset(@, "regen_retry_at", frame + 45)
                return
    @set("should_use_weapon", true)

CONFIGS = {
    {
        name: "BaseFighter"
        class: "Fighter"
        input: () =>
            @set("should_use_weapon", true)
    }
    {
        name: "BaseNecromancer"
        class: "Necromancer"
        input: necromancer_eval_input
    }
    {
        name: "BaseStormcaller"
        class: "Stormcaller"
        input: () =>
            @set("use_spell_slot", 0)
    }
    {
        name: "BasePyrocaster"
        class: "Pyrocaster"
        input: () =>
            @set("use_spell_slot", 0)
    }
}

dump_stats = (config_name, monster, seed, run_index, outcome, run_state) ->
    GameState = require("core.GameState")
    player = World.players[1].instance
    enemy_hp, enemy_max_hp = 0, 0
    enemies_alive = Map.enemies_list(player)
    for {:stats} in *enemies_alive
        enemy_hp += stats.hp
        enemy_max_hp += stats.max_hp
    { :defender_count, :attacker_count, :melee_count,
      :projectile_count, :receive_melee_count } = player\get_effect("DebugMonsterEvalEffect")
    net_player_damage = math.max(0, player.stats.max_hp - player.stats.hp)
    gross_player_damage = math.max(run_state and run_state.gross_player_damage or 0, net_player_damage)
    player_healing = run_state and run_state.player_healing or 0
    min_player_hp = run_state and run_state.min_player_hp or player.stats.hp
    return {
        :config_name
        :run_index
        seed: tostring(seed)
        level: player.stats.level
        :monster
        xpaward: enemy_xpaward(monster)
        outcome: outcome or "unknown"
        frames: GameState.frame
        player_damage: gross_player_damage
        :gross_player_damage
        :net_player_damage
        :player_healing
        :min_player_hp
        :enemy_hp, :enemy_max_hp
        enemy_hp_pct: safe_div(enemy_hp, math.max(1, enemy_max_hp))
        enemy_count_alive: #enemies_alive
        :defender_count, :attacker_count, :melee_count,
        :projectile_count, :receive_melee_count
    }

run_bot_tests = (raw_args) ->
    parser = argparse("lanarts", "Run monster balance simulations.")
    parser\option("--steps", "Maximum number of steps to take.", 10000)
    parser\option("--runs", "Number of paired seeds to run per config and monster.", 1)
    parser\option("--event_log", "Event log to write to.", nil)
    parser\option("--seed", "Starting seed.", "12345678")
    parser\option("--level", "Player level.", 1)
    parser\option("--levels", "Comma-separated player levels. Overrides --level when set.", nil)
    parser\option("--monster", "Monster to fight. Use comma-separated compact names like GiantRat,Hound or all.", nil)
    parser\option("--configs", "Comma-separated eval configs.", DEFAULT_CONFIGS)
    parser\option("--out", "Raw monster run CSV output.", "saves/monster_stats.csv")
    parser\option("--aggregate_out", "Aggregate monster balance CSV output.", "saves/monster_aggregate.csv")
    parser\option("--report_out", "Enemy balance report CSV output.", "saves/monster_report.csv")
    parser\option("--json_out", "JSON output containing raw, aggregate, and report rows.", "saves/monster_stats.json")
    parser\option("--report_delta_pct", "Relative deviation needed for report warnings.", 0.5)
    parser\option("--min_report_runs", "Minimum aggregate runs before report warnings are emitted.", 1)
    parser\option("--comparison_event_log", "Event log to compare to.", nil)
    args = parser\parse(raw_args)

    local game_start, config_eval_step, monster_eval_step, game_step

    game_start = () ->
        return ResourceLoading.ensure_resources_before(config_eval_step)

    game_exit = (msg) ->
        print msg
        return nil

    config_eval_step = () ->
        stats = {}
        selected_configs = parse_configs(args.configs)
        selected_monsters = parse_monster_names(args.monster)
        selected_levels = if args.levels then parse_levels(args.levels) else parse_levels(args.level)
        level_eval_step = (level, next_level) ->
            return _iterate selected_configs, monster_eval_step(level, stats, selected_monsters), next_level
        return _iterate selected_levels, level_eval_step, () ->
            aggregate_rows = aggregate_stats(stats)
            report_rows = build_report(aggregate_rows, args.report_delta_pct, args.min_report_runs)
            write_csv args.out, stats, {
                "config_name", "run_index", "seed", "level", "monster", "xpaward",
                "outcome", "frames", "player_damage", "gross_player_damage",
                "net_player_damage", "player_healing", "min_player_hp",
                "enemy_hp", "enemy_max_hp",
                "enemy_hp_pct", "enemy_count_alive", "defender_count", "attacker_count",
                "melee_count", "projectile_count", "receive_melee_count"
            }
            write_csv args.aggregate_out, aggregate_rows, {
                "config_name", "level", "monster", "runs", "contact_runs", "contact_run_rate", "xpaward",
                "enemy_kill_rate", "step_limit_rate", "no_contact_step_limit_rate", "player_death_rate",
                "avg_frames", "min_frames", "max_frames",
                "avg_player_damage", "min_player_damage", "max_player_damage",
                "avg_gross_player_damage", "avg_net_player_damage", "avg_player_healing",
                "avg_min_player_hp", "min_player_hp",
                "damage_per_1000_frames", "damage_per_xp", "xp_per_100_damage",
                "contact_avg_frames", "contact_min_frames", "contact_max_frames",
                "contact_avg_player_damage", "contact_min_player_damage", "contact_max_player_damage",
                "contact_avg_gross_player_damage", "contact_avg_net_player_damage",
                "contact_avg_player_healing", "contact_avg_min_player_hp", "contact_min_player_hp",
                "contact_damage_per_1000_frames", "contact_damage_per_xp", "contact_xp_per_100_damage",
                "avg_enemy_hp_pct", "avg_enemy_count_alive",
                "avg_defender_count", "avg_attacker_count", "avg_melee_count",
                "avg_projectile_count", "avg_receive_melee_count"
            }
            write_csv args.report_out, report_rows, {
                "severity", "category", "config_name", "level", "monster",
                "metric", "value", "baseline", "delta", "delta_pct", "message"
            }
            if args.json_out and args.json_out != ""
                file_dump_json args.json_out, {
                    schema_version: 2
                    seed: tostring(args.seed)
                    runs_per_config_monster: tonumber(args.runs)
                    raw: stats
                    aggregate: aggregate_rows
                    report: report_rows
                }
            print string.format(
                "Monster eval complete: %d raw runs, %d aggregate rows, %d report rows. Wrote %s, %s, %s, %s.",
                #stats, #aggregate_rows, #report_rows,
                args.out, args.aggregate_out, args.report_out, args.json_out
            )
            for idx, row in ipairs(report_rows)
                break if idx > 10
                print string.format("[%s] %s", row.severity, row.message)
            if #report_rows > 10
                print string.format("... %d more report rows in %s", #report_rows - 10, args.report_out)
            return game_exit("Done.")

    monster_eval_step = (level, stats, monster_names) -> (config, next_config) ->
        run_eval_step = (run_index, next_run) ->
            seed = tonumber(args.seed) + run_index - 1
            step = (monster_name, next_monster) ->
                print('run', run_index, 'seed', seed, 'level', level, 'config', config.name, 'mon', monster_name)
                run_state = make_run_state()
                on_finish = (outcome) ->
                    append stats, dump_stats(config.name, monster_name, seed, run_index, outcome, run_state)
                    return next_monster()
                -- Continue simulation iteration:
                game_init(seed, config.class, {monster_name}, config.input, level)
                return game_step(on_finish, run_state)
            return _iterate monster_names, step, next_run
        runs = [i for i=1,tonumber(args.runs)]
        return _iterate runs, run_eval_step, next_config

    game_step = (next_sim, run_state) ->
        step = () ->
            -- (1) Load dependencies
            GameState = require("core.GameState")
            GameLoop = require("GameLoop")

            -- Monster evals are simulation-only; drawing in headless runs can
            -- still hit platform GL paths on some machines.
            step_only = true
            before_hp = capture_player_hp(run_state)
            if #Map.enemies_list(World.players[1].instance) == 0
                return next_sim("enemy_killed")
            -- (3) Check if we are at final frame:
            if args.steps and GameState.frame >= tonumber(args.steps)
                return next_sim("step_limit")

            -- (4) Run game loop & and see if game
            -- should be saved + exited
            if not GameLoop.game_step(step_only)
                record_player_hp_delta(run_state, before_hp)
                return next_sim("game_loop_exit")
            record_player_hp_delta(run_state, before_hp)

            -- (5) Loop again
            return step
        require("Events").events.PlayerDeath = () ->
            step = () -> next_sim("player_death")
            return false
        EngineHooks.game_won = () ->
            -- Exit on next step
            step = () -> next_sim("game_won")
        return step()

    return StartEngine.start_engine {
        settings: { -- Settings object
            -- Window settings
            fullscreen: false
            view_width: 640
            view_height: 480
            -- Font settings
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: false
            regen_on_death: false
            event_log: args.event_log
            comparison_event_log: args.comparison_event_log
        },
        entry_point: game_start,
        debug: args.debug,
        on_exit: () -> nil
    }

return run_bot_tests
