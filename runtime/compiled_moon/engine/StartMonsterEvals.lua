local argparse = require("argparse")
local EngineInternal = require("core.EngineInternal")
local ResourceLoading = require("engine.ResourceLoading")
local StartEngine = require("engine.StartEngine")
local EngineHooks = require("engine.EngineBase")
local Settings = require("engine.Settings")
local EventLog = require("ui.EventLog")
local Map = require("core.Map")
local World = require("core.World")
local MAX_TIME = 10000000
local CONFIGS = nil
local DEFAULT_CONFIGS = "BaseFighter,BaseNecromancer,BaseStormcaller,BasePyrocaster"
local SPAWN_SOURCE_FILES = {
  "map_descs/Crypt.moon",
  "map_descs/GraghsLair.moon",
  "map_descs/Hell.moon",
  "map_descs/HiveDepths.moon",
  "map_descs/HiveEntrance.json",
  "map_descs/HiveEntrance.moon",
  "map_descs/OgreLair.moon",
  "map_descs/OutpostEntrance.moon",
  "map_descs/OutpostStockroom.moon",
  "map_descs/Overworld.moon",
  "map_descs/PixullochiaDepths.moon",
  "map_descs/PixullochiaEntrance.moon",
  "map_descs/SnakePitDepths.moon",
  "map_descs/SnakePitEntrance.moon",
  "map_descs/TempleChamber.moon",
  "map_descs/TempleEntrance.moon",
  "map_descs/TempleSanctum.moon",
  "map_descs/Underdungeon.moon",
  "map_descs/World.moon",
  "maps/NewDungeons.moon",
  "maps/OldMaps.lua",
  "maps/Places.moon",
  "maps/Region1.lua"
}
local SUMMON_SOURCE_FILES = {
  "enemies/Enemies.moon"
}
local SOURCE_CONTEXT_CACHE = { }
local split_csv
split_csv = function(value)
  local result = { }
  for part in tostring(value or ""):gmatch("[^,]+") do
    part = part:gsub("^%s+", ""):gsub("%s+$", "")
    if part ~= "" then
      append(result, part)
    end
  end
  return result
end
local format_rate
format_rate = function(value)
  return string.format("%.3f", tonumber(value) or 0)
end
local safe_div
safe_div = function(num, denom)
  if not denom or denom == 0 then
    return 0
  end
  return (num or 0) / denom
end
local normalize_key
normalize_key = function(value)
  local lowered = string.lower(tostring(value or ""))
  local normalized = string.gsub(lowered, "[^%w]", "")
  return normalized
end
local uses_net_damage_for_threat
uses_net_damage_for_threat = function(config_name)
  return normalize_key(config_name):find("necromancer", 1, true) ~= nil
end
local csv_escape
csv_escape = function(value)
  if value == nil then
    return ""
  end
  value = tostring(value)
  if value:find('[,"\n]') then
    return '"' .. value:gsub('"', '""') .. '"'
  end
  return value
end
local sorted_extra_keys
sorted_extra_keys = function(rows, preferred)
  local seen = { }
  local extras = { }
  for _index_0 = 1, #preferred do
    local key = preferred[_index_0]
    seen[key] = true
  end
  for _index_0 = 1, #rows do
    local row = rows[_index_0]
    for key, _ in pairs(row) do
      if not seen[key] then
        seen[key] = true
        append(extras, key)
      end
    end
  end
  table.sort(extras)
  local keys = { }
  for _index_0 = 1, #preferred do
    local key = preferred[_index_0]
    append(keys, key)
  end
  for _index_0 = 1, #extras do
    local key = extras[_index_0]
    append(keys, key)
  end
  return keys
end
local write_csv
write_csv = function(path, rows, preferred_keys)
  if not path or path == "" then
    return
  end
  preferred_keys = preferred_keys or { }
  local keys = sorted_extra_keys(rows, preferred_keys)
  local lines = {
    table.concat(keys, ",")
  }
  for _index_0 = 1, #rows do
    local row = rows[_index_0]
    local values = { }
    for _index_1 = 1, #keys do
      local key = keys[_index_1]
      append(values, csv_escape(row[key]))
    end
    append(lines, table.concat(values, ","))
  end
  return file_dump_string(path, table.concat(lines, "\n"))
end
local strip_line_comment
strip_line_comment = function(line)
  if not (line) then
    return ""
  end
  return line:gsub("%-%-.*$", "")
end
local line_has_quoted_name
line_has_quoted_name = function(line, name)
  if not (line and name) then
    return false
  end
  if line:find('"' .. name .. '"', 1, true) then
    return true
  end
  if line:find("'" .. name .. "'", 1, true) then
    return true
  end
  return false
end
local source_refs_for_name
source_refs_for_name = function(name, files, only_summoner_lines)
  if only_summoner_lines == nil then
    only_summoner_lines = false
  end
  local refs = { }
  for _index_0 = 1, #files do
    local _continue_0 = false
    repeat
      local file = files[_index_0]
      local contents = file_as_string(file)
      if not (contents) then
        _continue_0 = true
        break
      end
      local line_no = 0
      for line in (contents .. "\n"):gmatch("(.-)\n") do
        local _continue_1 = false
        repeat
          line_no = line_no + 1
          local active_line = strip_line_comment(line)
          if active_line == "" then
            _continue_1 = true
            break
          end
          if only_summoner_lines and not active_line:find("summoner_base", 1, true) then
            _continue_1 = true
            break
          end
          if line_has_quoted_name(active_line, name) then
            append(refs, tostring(file) .. ":" .. tostring(line_no))
          end
          _continue_1 = true
        until true
        if not _continue_1 then
          break
        end
      end
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return refs
end
local monster_source_context
monster_source_context = function(name)
  local cached = SOURCE_CONTEXT_CACHE[name]
  if cached then
    return cached
  end
  local map_refs = source_refs_for_name(name, SPAWN_SOURCE_FILES)
  local summon_refs = source_refs_for_name(name, SUMMON_SOURCE_FILES, true)
  local refs = { }
  for _index_0 = 1, #map_refs do
    local ref = map_refs[_index_0]
    append(refs, "map:" .. tostring(ref))
  end
  for _index_0 = 1, #summon_refs do
    local ref = summon_refs[_index_0]
    append(refs, "summon:" .. tostring(ref))
  end
  local status
  if #map_refs > 0 then
    status = "map_spawned"
  elseif #summon_refs > 0 then
    status = "summoned_only"
  else
    status = "definition_only"
  end
  cached = {
    status = status,
    refs = refs
  }
  SOURCE_CONTEXT_CACHE[name] = cached
  return cached
end
local encounter_source_context
encounter_source_context = function(spawns)
  local has_map, has_summon, all_map = false, false, true
  local refs = { }
  for _index_0 = 1, #spawns do
    local monster = spawns[_index_0]
    local context = monster_source_context(monster)
    if context.status == "map_spawned" then
      has_map = true
    else
      all_map = false
    end
    if context.status == "summoned_only" then
      has_summon = true
    end
    local _list_0 = context.refs
    for _index_1 = 1, #_list_0 do
      local ref = _list_0[_index_1]
      if #refs < 12 then
        append(refs, tostring(monster) .. "@" .. tostring(ref))
      end
    end
  end
  local status
  if all_map and #spawns > 0 then
    status = "map_spawned"
  elseif has_map then
    status = "mixed_map_spawned"
  elseif has_summon then
    status = "summoned_only"
  else
    status = "definition_only"
  end
  return {
    status = status,
    refs = table.concat(refs, ";")
  }
end
local monster_aliases
monster_aliases = function()
  local aliases = { }
  local _list_0 = table.sorted_key_list(enemies)
  for _index_0 = 1, #_list_0 do
    local name = _list_0[_index_0]
    aliases[normalize_key(name)] = name
  end
  return aliases
end
local resolve_monster_name
resolve_monster_name = function(part, aliases)
  aliases = aliases or monster_aliases()
  local resolved = aliases[normalize_key(part)]
  if not resolved then
    error("Unknown monster '" .. tostring(part) .. "'. Use compact aliases like GiantRat, or --monster all.")
  end
  return resolved
end
local parse_monster_names
parse_monster_names = function(value)
  local parts = split_csv(value)
  if #parts == 0 then
    return { }
  end
  if #parts == 1 and normalize_key(parts[1]) == "all" then
    return table.sorted_key_list(enemies)
  end
  local aliases = monster_aliases()
  local names, seen = { }, { }
  for _index_0 = 1, #parts do
    local part = parts[_index_0]
    local resolved = resolve_monster_name(part, aliases)
    if not seen[resolved] then
      seen[resolved] = true
      append(names, resolved)
    end
  end
  return names
end
local pack_label
pack_label = function(spawns)
  local counts, ordered = { }, { }
  for _index_0 = 1, #spawns do
    local name = spawns[_index_0]
    if not (counts[name]) then
      counts[name] = 0
      append(ordered, name)
    end
    counts[name] = counts[name] + 1
  end
  local parts = { }
  for _index_0 = 1, #ordered do
    local name = ordered[_index_0]
    local count = counts[name]
    append(parts, (function()
      if count > 1 then
        return tostring(name) .. " x" .. tostring(count)
      else
        return name
      end
    end)())
  end
  return table.concat(parts, " + ")
end
local encounter_types
encounter_types = function(spawns)
  local seen, ordered = { }, { }
  for _index_0 = 1, #spawns do
    local _continue_0 = false
    repeat
      local monster = spawns[_index_0]
      local entry = enemies and enemies[monster]
      if not (entry and entry.types) then
        _continue_0 = true
        break
      end
      local _list_0 = entry.types
      for _index_1 = 1, #_list_0 do
        local type = _list_0[_index_1]
        if not (seen[type]) then
          seen[type] = true
          append(ordered, type)
        end
      end
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return table.concat(ordered, "|")
end
local make_encounter
make_encounter = function(name, spawns)
  local xpaward = 0
  for _index_0 = 1, #spawns do
    local monster = spawns[_index_0]
    local entry = enemies and enemies[monster]
    xpaward = xpaward + (tonumber(entry and entry.xpaward) or 0)
  end
  local source_context = encounter_source_context(spawns)
  return {
    name = name,
    spawns = spawns,
    spawn_count = #spawns,
    monster_spawns = table.concat(spawns, "|"),
    xpaward = xpaward,
    monster_types = encounter_types(spawns),
    source_status = source_context.status,
    source_refs = source_context.refs
  }
end
local parse_pack_spawn
parse_pack_spawn = function(part, aliases)
  part = tostring(part or ""):gsub("^%s+", ""):gsub("%s+$", "")
  if part == "" then
    error("Empty monster in --pack spec.")
  end
  local name_part, count_part = part:match("^(.-)%*(%d+)$")
  local count = tonumber(count_part) or 1
  local name = name_part or part
  local resolved = resolve_monster_name(name, aliases)
  local spawns = { }
  for i = 1, count do
    append(spawns, resolved)
  end
  return spawns
end
local parse_pack_specs
parse_pack_specs = function(value)
  local encounters = { }
  if not value or tostring(value) == "" then
    return encounters
  end
  local aliases = monster_aliases()
  for raw_spec in tostring(value):gmatch("[^;]+") do
    local _continue_0 = false
    repeat
      local spec = raw_spec:gsub("^%s+", ""):gsub("%s+$", "")
      if spec == "" then
        _continue_0 = true
        break
      end
      local label, body = spec:match("^([^=:]+)[:=](.+)$")
      if not (body) then
        body = spec
        label = nil
      end
      local spawns = { }
      for part in body:gmatch("[^+]+") do
        local _list_0 = parse_pack_spawn(part, aliases)
        for _index_0 = 1, #_list_0 do
          local monster = _list_0[_index_0]
          append(spawns, monster)
        end
      end
      if #spawns == 0 then
        error("Pack '" .. tostring(spec) .. "' did not resolve to any monsters.")
      end
      label = label and label:gsub("^%s+", ""):gsub("%s+$", "")
      append(encounters, make_encounter(label and label ~= "" and label or pack_label(spawns), spawns))
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return encounters
end
local parse_encounters
parse_encounters = function(monster_value, pack_value)
  local encounters = { }
  local _list_0 = parse_monster_names(monster_value)
  for _index_0 = 1, #_list_0 do
    local monster = _list_0[_index_0]
    append(encounters, make_encounter(monster, {
      monster
    }))
  end
  local _list_1 = parse_pack_specs(pack_value)
  for _index_0 = 1, #_list_1 do
    local encounter = _list_1[_index_0]
    append(encounters, encounter)
  end
  if #encounters == 0 then
    error("Need --monster NAME,LIST, --monster all, or --pack LABEL=Monster+Monster*COUNT.")
  end
  return encounters
end
local parse_config_names
parse_config_names = function(value)
  local names = split_csv(value)
  if #names == 0 then
    return split_csv(DEFAULT_CONFIGS)
  end
  return names
end
local parse_levels
parse_levels = function(value)
  local levels = { }
  local _list_0 = split_csv(value)
  for _index_0 = 1, #_list_0 do
    local part = _list_0[_index_0]
    local level = tonumber(part)
    if level and level >= 1 then
      append(levels, math.floor(level))
    end
  end
  if #levels == 0 then
    append(levels, 1)
  end
  return levels
end
local config_by_name
config_by_name = function()
  local result = { }
  for _index_0 = 1, #CONFIGS do
    local config = CONFIGS[_index_0]
    result[config.name] = config
  end
  return result
end
local parse_configs
parse_configs = function(value)
  local by_name = config_by_name()
  local selected = { }
  local _list_0 = parse_config_names(value)
  for _index_0 = 1, #_list_0 do
    local name = _list_0[_index_0]
    local config = by_name[name]
    if not config then
      error("Unknown config '" .. tostring(name) .. "'.")
    end
    append(selected, config)
  end
  return selected
end
local enemy_xpaward
enemy_xpaward = function(monster)
  local entry = enemies and enemies[monster]
  return tonumber(entry and entry.xpaward) or 0
end
local enemy_types
enemy_types = function(monster)
  local entry = enemies and enemies[monster]
  if not (entry and entry.types) then
    return ""
  end
  return table.concat(entry.types, "|")
end
local enemy_primary_resistance
enemy_primary_resistance = function(monster, primary_type)
  if not (primary_type and primary_type ~= "") then
    return 0
  end
  local entry = enemies and enemies[monster]
  local raw = entry and entry.resistances and entry.resistances[primary_type]
  return tonumber(raw) or 0
end
local encounter_primary_resistance
encounter_primary_resistance = function(encounter, primary_type)
  if not (encounter and encounter.spawns and #encounter.spawns > 0) then
    return 0
  end
  local total = 0
  local _list_0 = encounter.spawns
  for _index_0 = 1, #_list_0 do
    local monster = _list_0[_index_0]
    total = total + enemy_primary_resistance(monster, primary_type)
  end
  return safe_div(total, #encounter.spawns)
end
local resistance_multiplier
resistance_multiplier = function(raw_resist)
  raw_resist = tonumber(raw_resist) or 0
  if raw_resist <= -100 then
    return 1.5
  end
  if raw_resist >= 100 then
    return 0.5
  end
  local resist = math.max(math.min(math.floor(raw_resist), 5), -5)
  return 1 - resist / 10
end
local sorted_group_keys
sorted_group_keys = function(groups)
  local keys = { }
  for key, _ in pairs(groups) do
    append(keys, key)
  end
  table.sort(keys)
  return keys
end
local group_key
group_key = function(config_name, level, monster)
  return tostring(config_name) .. "\n" .. tostring(level) .. "\n" .. tostring(monster)
end
local config_level_key
config_level_key = function(config_name, level)
  return tostring(config_name) .. "\n" .. tostring(level)
end
local add_metric
add_metric = function(aggregate, row, metric)
  local value = tonumber(row[metric]) or 0
  aggregate.sums[metric] = (aggregate.sums[metric] or 0) + value
  if aggregate.mins[metric] == nil or value < aggregate.mins[metric] then
    aggregate.mins[metric] = value
  end
  if aggregate.maxes[metric] == nil or value > aggregate.maxes[metric] then
    aggregate.maxes[metric] = value
  end
end
local add_contact_metric
add_contact_metric = function(aggregate, row, metric)
  local value = tonumber(row[metric]) or 0
  aggregate.contact_sums[metric] = (aggregate.contact_sums[metric] or 0) + value
  if aggregate.contact_mins[metric] == nil or value < aggregate.contact_mins[metric] then
    aggregate.contact_mins[metric] = value
  end
  if aggregate.contact_maxes[metric] == nil or value > aggregate.contact_maxes[metric] then
    aggregate.contact_maxes[metric] = value
  end
end
local SUMMARY_METRICS = {
  "frames",
  "player_damage",
  "gross_player_damage",
  "net_player_damage",
  "player_healing",
  "min_player_hp",
  "enemy_hp",
  "enemy_max_hp",
  "enemy_hp_pct",
  "sapped_frames",
  "sapped_frame_rate",
  "defender_count",
  "attacker_count",
  "melee_count",
  "projectile_count",
  "receive_melee_count",
  "enemy_count_alive"
}
local CONTACT_METRICS = {
  "sapped_frames",
  "defender_count",
  "attacker_count",
  "melee_count",
  "projectile_count",
  "receive_melee_count"
}
local had_contact
had_contact = function(row)
  if (tonumber(row.player_damage) or 0) > 0 then
    return true
  end
  for _index_0 = 1, #CONTACT_METRICS do
    local metric = CONTACT_METRICS[_index_0]
    if (tonumber(row[metric]) or 0) > 0 then
      return true
    end
  end
  return false
end
local aggregate_stats
aggregate_stats = function(stats)
  local groups = { }
  for _index_0 = 1, #stats do
    local row = stats[_index_0]
    local key = group_key(row.config_name, row.level, row.monster)
    local aggregate = groups[key]
    if not aggregate then
      aggregate = {
        config_name = row.config_name,
        level = tonumber(row.level) or 1,
        monster = row.monster,
        runs = 0,
        contact_runs = 0,
        spawn_count = tonumber(row.spawn_count) or 1,
        monster_spawns = row.monster_spawns or row.monster,
        xpaward = tonumber(row.xpaward) or 0,
        source_status = row.source_status or "",
        source_refs = row.source_refs or "",
        config_primary_type = row.config_primary_type or "",
        monster_types = row.monster_types or "",
        primary_resistance = tonumber(row.primary_resistance) or 0,
        primary_resistance_multiplier = tonumber(row.primary_resistance_multiplier) or 1,
        outcomes = { },
        sums = { },
        mins = { },
        maxes = { },
        contact_sums = { },
        contact_mins = { },
        contact_maxes = { }
      }
      groups[key] = aggregate
    end
    aggregate.runs = aggregate.runs + 1
    local contact = had_contact(row)
    if contact then
      aggregate.contact_runs = aggregate.contact_runs + 1
    end
    local outcome = row.outcome or "unknown"
    if outcome == "game_won" then
      outcome = "enemy_killed"
    end
    if outcome == "step_limit" and not contact then
      outcome = "no_contact_step_limit"
    end
    aggregate.outcomes[outcome] = (aggregate.outcomes[outcome] or 0) + 1
    for _index_1 = 1, #SUMMARY_METRICS do
      local metric = SUMMARY_METRICS[_index_1]
      add_metric(aggregate, row, metric)
      if contact then
        add_contact_metric(aggregate, row, metric)
      end
    end
  end
  local rows = { }
  local _list_0 = sorted_group_keys(groups)
  for _index_0 = 1, #_list_0 do
    local key = _list_0[_index_0]
    local aggregate = groups[key]
    local runs = math.max(1, aggregate.runs)
    local contact_runs = math.max(1, aggregate.contact_runs)
    local avg_frames = safe_div(aggregate.sums.frames, runs)
    local avg_damage = safe_div(aggregate.sums.player_damage, runs)
    local damage_per_1000_frames = safe_div(aggregate.sums.player_damage, aggregate.sums.frames) * 1000
    local damage_per_xp = safe_div(avg_damage, math.max(1, aggregate.xpaward))
    local xp_per_100_damage = safe_div(aggregate.xpaward * 100, avg_damage)
    local contact_avg_frames = safe_div(aggregate.contact_sums.frames, contact_runs)
    local contact_avg_damage = safe_div(aggregate.contact_sums.player_damage, contact_runs)
    local contact_damage_per_1000_frames = safe_div(aggregate.contact_sums.player_damage, aggregate.contact_sums.frames) * 1000
    local contact_damage_per_xp = safe_div(contact_avg_damage, math.max(1, aggregate.xpaward))
    local contact_xp_per_100_damage = safe_div(aggregate.xpaward * 100, contact_avg_damage)
    local avg_gross_damage = safe_div(aggregate.sums.gross_player_damage, runs)
    local avg_net_damage = safe_div(aggregate.sums.net_player_damage, runs)
    local avg_healing = safe_div(aggregate.sums.player_healing, runs)
    local avg_min_player_hp = safe_div(aggregate.sums.min_player_hp, runs)
    local contact_avg_gross_damage = safe_div(aggregate.contact_sums.gross_player_damage, contact_runs)
    local contact_avg_net_damage = safe_div(aggregate.contact_sums.net_player_damage, contact_runs)
    local contact_avg_healing = safe_div(aggregate.contact_sums.player_healing, contact_runs)
    local contact_avg_min_player_hp = safe_div(aggregate.contact_sums.min_player_hp, contact_runs)
    local avg_sapped_frames = safe_div(aggregate.sums.sapped_frames, runs)
    local sapped_frames_per_1000_frames = safe_div(aggregate.sums.sapped_frames, aggregate.sums.frames) * 1000
    local contact_avg_sapped_frames = safe_div(aggregate.contact_sums.sapped_frames, contact_runs)
    local contact_sapped_frames_per_1000_frames = safe_div(aggregate.contact_sums.sapped_frames, aggregate.contact_sums.frames) * 1000
    local net_damage_per_1000_frames = safe_div(aggregate.sums.net_player_damage, aggregate.sums.frames) * 1000
    local net_damage_per_xp = safe_div(avg_net_damage, math.max(1, aggregate.xpaward))
    local contact_net_damage_per_1000_frames = safe_div(aggregate.contact_sums.net_player_damage, aggregate.contact_sums.frames) * 1000
    local contact_net_damage_per_xp = safe_div(contact_avg_net_damage, math.max(1, aggregate.xpaward))
    local net_threat_mode = uses_net_damage_for_threat(aggregate.config_name)
    local threat_damage_mode
    if net_threat_mode then
      threat_damage_mode = "net"
    else
      threat_damage_mode = "gross"
    end
    local avg_threat_damage
    if net_threat_mode then
      avg_threat_damage = avg_net_damage
    else
      avg_threat_damage = avg_damage
    end
    local threat_damage_per_1000_frames
    if net_threat_mode then
      threat_damage_per_1000_frames = net_damage_per_1000_frames
    else
      threat_damage_per_1000_frames = damage_per_1000_frames
    end
    local threat_damage_per_xp
    if net_threat_mode then
      threat_damage_per_xp = net_damage_per_xp
    else
      threat_damage_per_xp = damage_per_xp
    end
    local contact_avg_threat_damage
    if net_threat_mode then
      contact_avg_threat_damage = contact_avg_net_damage
    else
      contact_avg_threat_damage = contact_avg_damage
    end
    local contact_threat_damage_per_1000_frames
    if net_threat_mode then
      contact_threat_damage_per_1000_frames = contact_net_damage_per_1000_frames
    else
      contact_threat_damage_per_1000_frames = contact_damage_per_1000_frames
    end
    local contact_threat_damage_per_xp
    if net_threat_mode then
      contact_threat_damage_per_xp = contact_net_damage_per_xp
    else
      contact_threat_damage_per_xp = contact_damage_per_xp
    end
    local threat_xp_per_100_damage = safe_div(aggregate.xpaward * 100, avg_threat_damage)
    local contact_threat_xp_per_100_damage = safe_div(aggregate.xpaward * 100, contact_avg_threat_damage)
    append(rows, {
      config_name = aggregate.config_name,
      level = aggregate.level,
      monster = aggregate.monster,
      runs = aggregate.runs,
      contact_runs = aggregate.contact_runs,
      contact_run_rate = format_rate(safe_div(aggregate.contact_runs, runs)),
      spawn_count = aggregate.spawn_count,
      monster_spawns = aggregate.monster_spawns,
      xpaward = aggregate.xpaward,
      source_status = aggregate.source_status,
      source_refs = aggregate.source_refs,
      config_primary_type = aggregate.config_primary_type,
      monster_types = aggregate.monster_types,
      primary_resistance = format_rate(aggregate.primary_resistance),
      primary_resistance_multiplier = format_rate(aggregate.primary_resistance_multiplier),
      enemy_kill_rate = format_rate(safe_div(aggregate.outcomes.enemy_killed or 0, runs)),
      step_limit_rate = format_rate(safe_div(aggregate.outcomes.step_limit or 0, runs)),
      no_contact_step_limit_rate = format_rate(safe_div(aggregate.outcomes.no_contact_step_limit or 0, runs)),
      player_death_rate = format_rate(safe_div(aggregate.outcomes.player_death or 0, runs)),
      avg_frames = format_rate(avg_frames),
      min_frames = aggregate.mins.frames or 0,
      max_frames = aggregate.maxes.frames or 0,
      avg_player_damage = format_rate(avg_damage),
      min_player_damage = aggregate.mins.player_damage or 0,
      max_player_damage = aggregate.maxes.player_damage or 0,
      avg_gross_player_damage = format_rate(avg_gross_damage),
      avg_net_player_damage = format_rate(avg_net_damage),
      avg_player_healing = format_rate(avg_healing),
      avg_min_player_hp = format_rate(avg_min_player_hp),
      min_player_hp = aggregate.mins.min_player_hp or 0,
      avg_sapped_frames = format_rate(avg_sapped_frames),
      sapped_frames_per_1000_frames = format_rate(sapped_frames_per_1000_frames),
      damage_per_1000_frames = format_rate(damage_per_1000_frames),
      damage_per_xp = format_rate(damage_per_xp),
      xp_per_100_damage = format_rate(xp_per_100_damage),
      net_damage_per_1000_frames = format_rate(net_damage_per_1000_frames),
      net_damage_per_xp = format_rate(net_damage_per_xp),
      threat_damage_mode = threat_damage_mode,
      avg_threat_player_damage = format_rate(avg_threat_damage),
      threat_damage_per_1000_frames = format_rate(threat_damage_per_1000_frames),
      threat_damage_per_xp = format_rate(threat_damage_per_xp),
      threat_xp_per_100_damage = format_rate(threat_xp_per_100_damage),
      contact_avg_frames = format_rate(contact_avg_frames),
      contact_min_frames = aggregate.contact_mins.frames or 0,
      contact_max_frames = aggregate.contact_maxes.frames or 0,
      contact_avg_player_damage = format_rate(contact_avg_damage),
      contact_min_player_damage = aggregate.contact_mins.player_damage or 0,
      contact_max_player_damage = aggregate.contact_maxes.player_damage or 0,
      contact_avg_gross_player_damage = format_rate(contact_avg_gross_damage),
      contact_avg_net_player_damage = format_rate(contact_avg_net_damage),
      contact_avg_player_healing = format_rate(contact_avg_healing),
      contact_avg_min_player_hp = format_rate(contact_avg_min_player_hp),
      contact_min_player_hp = aggregate.contact_mins.min_player_hp or 0,
      contact_avg_sapped_frames = format_rate(contact_avg_sapped_frames),
      contact_sapped_frames_per_1000_frames = format_rate(contact_sapped_frames_per_1000_frames),
      contact_damage_per_1000_frames = format_rate(contact_damage_per_1000_frames),
      contact_damage_per_xp = format_rate(contact_damage_per_xp),
      contact_xp_per_100_damage = format_rate(contact_xp_per_100_damage),
      contact_net_damage_per_1000_frames = format_rate(contact_net_damage_per_1000_frames),
      contact_net_damage_per_xp = format_rate(contact_net_damage_per_xp),
      contact_avg_threat_player_damage = format_rate(contact_avg_threat_damage),
      contact_threat_damage_per_1000_frames = format_rate(contact_threat_damage_per_1000_frames),
      contact_threat_damage_per_xp = format_rate(contact_threat_damage_per_xp),
      contact_threat_xp_per_100_damage = format_rate(contact_threat_xp_per_100_damage),
      avg_enemy_hp_pct = format_rate(safe_div(aggregate.sums.enemy_hp_pct, runs)),
      avg_enemy_count_alive = format_rate(safe_div(aggregate.sums.enemy_count_alive, runs)),
      avg_defender_count = format_rate(safe_div(aggregate.sums.defender_count, runs)),
      avg_attacker_count = format_rate(safe_div(aggregate.sums.attacker_count, runs)),
      avg_melee_count = format_rate(safe_div(aggregate.sums.melee_count, runs)),
      avg_projectile_count = format_rate(safe_div(aggregate.sums.projectile_count, runs)),
      avg_receive_melee_count = format_rate(safe_div(aggregate.sums.receive_melee_count, runs))
    })
  end
  return rows
end
local mean_by_config_level
mean_by_config_level = function(rows, metric)
  local groups = { }
  for _index_0 = 1, #rows do
    local _continue_0 = false
    repeat
      local row = rows[_index_0]
      local value = tonumber(row[metric]) or 0
      if value <= 0 then
        _continue_0 = true
        break
      end
      local key = config_level_key(row.config_name, row.level)
      local group = groups[key]
      if not group then
        group = {
          sum = 0,
          count = 0
        }
        groups[key] = group
      end
      group.sum = group.sum + value
      group.count = group.count + 1
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  local means = { }
  for key, group in pairs(groups) do
    means[key] = safe_div(group.sum, group.count)
  end
  return means
end
local matchup_note
matchup_note = function(row)
  local primary_resistance = tonumber(row.primary_resistance) or 0
  local primary_type = row.config_primary_type or ""
  if not (primary_type ~= "" and primary_resistance > 0) then
    return ""
  end
  return " " .. tostring(row.monster) .. " resists " .. tostring(primary_type) .. " damage (resist " .. tostring(row.primary_resistance) .. ", multiplier " .. tostring(row.primary_resistance_multiplier) .. ")."
end
local source_note
source_note = function(row)
  local status = row.source_status or ""
  if status == "definition_only" then
    return " Source context: no active map or summoner references found."
  end
  if status == "summoned_only" then
    return " Source context: summon-only encounter."
  end
  if status == "mixed_map_spawned" then
    return " Source context: mixed map-spawned encounter."
  end
  return ""
end
local normalize_inactive_warning_severity
normalize_inactive_warning_severity = function(value)
  value = string.lower(tostring(value or "info"))
  if value == "warning" then
    return "warning"
  end
  return "info"
end
local warning_severity_for
warning_severity_for = function(row, inactive_warning_severity)
  local status = row.source_status or ""
  if status == "definition_only" or status == "summoned_only" then
    return inactive_warning_severity
  end
  return "warning"
end
local with_matchup_context
with_matchup_context = function(row, data)
  data.source_status = row.source_status or ""
  data.source_refs = row.source_refs or ""
  data.config_primary_type = row.config_primary_type or ""
  data.monster_types = row.monster_types or ""
  data.primary_resistance = row.primary_resistance or ""
  data.primary_resistance_multiplier = row.primary_resistance_multiplier or ""
  data.spawn_count = row.spawn_count or ""
  data.monster_spawns = row.monster_spawns or ""
  data.threat_damage_mode = row.threat_damage_mode or ""
  data.contact_avg_gross_player_damage = row.contact_avg_gross_player_damage or ""
  data.contact_avg_net_player_damage = row.contact_avg_net_player_damage or ""
  data.contact_avg_player_healing = row.contact_avg_player_healing or ""
  data.contact_avg_sapped_frames = row.contact_avg_sapped_frames or ""
  data.contact_sapped_frames_per_1000_frames = row.contact_sapped_frames_per_1000_frames or ""
  return data
end
local threat_damage_label
threat_damage_label = function(row)
  if (row.threat_damage_mode or "") == "net" then
    return "net contact damage"
  end
  return "contact damage"
end
local threat_damage_note
threat_damage_note = function(row)
  if not ((row.threat_damage_mode or "") == "net") then
    return ""
  end
  return " Gross contact damage " .. tostring(row.contact_avg_gross_player_damage) .. ", healing " .. tostring(row.contact_avg_player_healing) .. "."
end
local build_report
build_report = function(aggregate_rows, delta_pct, min_report_runs, inactive_warning_severity)
  delta_pct = tonumber(delta_pct) or 0.5
  min_report_runs = tonumber(min_report_runs) or 1
  inactive_warning_severity = normalize_inactive_warning_severity(inactive_warning_severity)
  local reports = { }
  local damage_per_xp_mean = mean_by_config_level(aggregate_rows, "contact_threat_damage_per_xp")
  local damage_rate_mean = mean_by_config_level(aggregate_rows, "contact_threat_damage_per_1000_frames")
  local frames_mean = mean_by_config_level(aggregate_rows, "contact_avg_frames")
  for _index_0 = 1, #aggregate_rows do
    local _continue_0 = false
    repeat
      local row = aggregate_rows[_index_0]
      if (tonumber(row.runs) or 0) < min_report_runs then
        _continue_0 = true
        break
      end
      local key = config_level_key(row.config_name, row.level)
      local kill_rate = tonumber(row.enemy_kill_rate) or 0
      local step_limit_rate = tonumber(row.step_limit_rate) or 0
      local no_contact_step_limit_rate = tonumber(row.no_contact_step_limit_rate) or 0
      local contact_runs = tonumber(row.contact_runs) or 0
      local damage_per_xp = tonumber(row.contact_threat_damage_per_xp) or 0
      local damage_rate = tonumber(row.contact_threat_damage_per_1000_frames) or 0
      local avg_frames = tonumber(row.contact_avg_frames) or 0
      local sapped_rate = tonumber(row.contact_sapped_frames_per_1000_frames) or 0
      local xpaward = tonumber(row.xpaward) or 0
      local threat_baseline = damage_per_xp_mean[key] or 0
      local damage_baseline = damage_rate_mean[key] or 0
      local frame_baseline = frames_mean[key] or 0
      if step_limit_rate > 0 then
        append(reports, with_matchup_context(row, {
          severity = warning_severity_for(row, inactive_warning_severity),
          category = "toughness",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "step_limit_rate",
          value = row.step_limit_rate,
          baseline = "0.000",
          delta = row.step_limit_rate,
          delta_pct = row.step_limit_rate,
          message = tostring(row.monster) .. " survives to the step limit in " .. tostring(row.step_limit_rate) .. " of " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. " runs." .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      if no_contact_step_limit_rate > 0 then
        append(reports, with_matchup_context(row, {
          severity = "info",
          category = "contact",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "no_contact_step_limit_rate",
          value = row.no_contact_step_limit_rate,
          baseline = "0.000",
          delta = row.no_contact_step_limit_rate,
          delta_pct = row.no_contact_step_limit_rate,
          message = tostring(row.monster) .. " had no combat contact before the step limit in " .. tostring(row.no_contact_step_limit_rate) .. " of " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. " runs." .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      if sapped_rate > 0 then
        append(reports, with_matchup_context(row, {
          severity = "info",
          category = "utility",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "contact_sapped_frames_per_1000_frames",
          value = row.contact_sapped_frames_per_1000_frames,
          baseline = "0.000",
          delta = row.contact_sapped_frames_per_1000_frames,
          delta_pct = "",
          message = tostring(row.monster) .. " kept " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. " Sapped for " .. tostring(row.contact_sapped_frames_per_1000_frames) .. " frames per 1000 contact frames. Average Sapped frames: " .. tostring(row.contact_avg_sapped_frames) .. "." .. tostring(source_note(row))
        }))
      end
      if threat_baseline > 0 and damage_per_xp > threat_baseline * (1 + delta_pct) then
        append(reports, with_matchup_context(row, {
          severity = warning_severity_for(row, inactive_warning_severity),
          category = "reward",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "contact_threat_damage_per_xp",
          value = row.contact_threat_damage_per_xp,
          baseline = format_rate(threat_baseline),
          delta = format_rate(damage_per_xp - threat_baseline),
          delta_pct = format_rate(safe_div(damage_per_xp - threat_baseline, threat_baseline)),
          message = tostring(row.monster) .. " deals " .. tostring(row.contact_threat_damage_per_xp) .. " " .. tostring(threat_damage_label(row)) .. " per XP versus " .. tostring(format_rate(threat_baseline)) .. " baseline for " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. "." .. tostring(threat_damage_note(row)) .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      if xpaward <= 0 and damage_rate > 0 then
        append(reports, with_matchup_context(row, {
          severity = warning_severity_for(row, inactive_warning_severity),
          category = "reward",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "xpaward",
          value = xpaward,
          baseline = "",
          delta = "",
          delta_pct = "",
          message = tostring(row.monster) .. " deals damage but awards no XP in " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. " evals." .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      if damage_baseline > 0 and damage_rate > damage_baseline * (1 + delta_pct) then
        append(reports, with_matchup_context(row, {
          severity = "info",
          category = "threat",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "contact_threat_damage_per_1000_frames",
          value = row.contact_threat_damage_per_1000_frames,
          baseline = format_rate(damage_baseline),
          delta = format_rate(damage_rate - damage_baseline),
          delta_pct = format_rate(safe_div(damage_rate - damage_baseline, damage_baseline)),
          message = tostring(row.monster) .. " " .. tostring(threat_damage_label(row)) .. " pace is " .. tostring(row.contact_threat_damage_per_1000_frames) .. " per 1000 frames versus " .. tostring(format_rate(damage_baseline)) .. " baseline for " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. "." .. tostring(threat_damage_note(row)) .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      if kill_rate == 1 and contact_runs > 0 and damage_baseline > 0 and damage_rate < damage_baseline * (1 - delta_pct) then
        append(reports, with_matchup_context(row, {
          severity = "info",
          category = "underthreat",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "contact_threat_damage_per_1000_frames",
          value = row.contact_threat_damage_per_1000_frames,
          baseline = format_rate(damage_baseline),
          delta = format_rate(damage_rate - damage_baseline),
          delta_pct = format_rate(safe_div(damage_rate - damage_baseline, damage_baseline)),
          message = tostring(row.monster) .. " " .. tostring(threat_damage_label(row)) .. " pace is only " .. tostring(row.contact_threat_damage_per_1000_frames) .. " per 1000 frames versus " .. tostring(format_rate(damage_baseline)) .. " baseline for " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. "." .. tostring(threat_damage_note(row)) .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      if kill_rate == 1 and frame_baseline > 0 and avg_frames > frame_baseline * (1 + delta_pct) then
        append(reports, with_matchup_context(row, {
          severity = "info",
          category = "durability",
          config_name = row.config_name,
          level = row.level,
          monster = row.monster,
          metric = "contact_avg_frames",
          value = row.contact_avg_frames,
          baseline = format_rate(frame_baseline),
          delta = format_rate(avg_frames - frame_baseline),
          delta_pct = format_rate(safe_div(avg_frames - frame_baseline, frame_baseline)),
          message = tostring(row.monster) .. " takes " .. tostring(row.contact_avg_frames) .. " contact frames to kill versus " .. tostring(format_rate(frame_baseline)) .. " baseline for " .. tostring(row.config_name) .. " L" .. tostring(row.level) .. "." .. tostring(matchup_note(row)) .. tostring(source_note(row))
        }))
      end
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  table.sort(reports, function(a, b)
    if a.severity ~= b.severity then
      return a.severity > b.severity
    end
    if a.category ~= b.category then
      return a.category < b.category
    end
    if a.config_name ~= b.config_name then
      return a.config_name < b.config_name
    end
    if tostring(a.level) ~= tostring(b.level) then
      return tostring(a.level) < tostring(b.level)
    end
    return a.monster < b.monster
  end)
  return reports
end
local game_init
game_init = function(seed, class_name, monster_spawns, input_func, level)
  local input_source
  input_source = function(player)
    player:add_effect("DebugMonsterEvalEffect", MAX_TIME)
    for i = 2, level do
      player:gain_level()
    end
    local source = require("input.ProgrammableInputSource").create(player)
    source.step = input_func
    player.stats.max_hp = 10000
    player.stats.hp = 10000
    return source
  end
  local GameState = require("core.GameState")
  GameState.clear_players()
  GameState.register_player('Playabot', class_name, input_source, true, 0, 0)
  log("Initializing GameState object...")
  EngineInternal.init_gamestate()
  random_seed(seed)
  Engine.first_map_create = function()
    return require("map_descs.DebugArena"):generate_and_spawn(monster_spawns)
  end
  EngineInternal.start_game()
  GameState.input_capture()
  GameState.input_handle()
  GameState.frame = 0
end
local _iterate
_iterate = function(tbl, f, next)
  local idx = 1
  local iterate
  iterate = function()
    local val = tbl[idx]
    idx = idx + 1
    if val == nil then
      return next()
    else
      return f(val, iterate)
    end
  end
  return iterate
end
local active_player
active_player = function()
  local player_entry = World.players and World.players[1]
  return player_entry and player_entry.instance
end
local make_run_state
make_run_state = function()
  return {
    gross_player_damage = 0,
    player_healing = 0,
    min_player_hp = nil,
    sapped_frames = 0
  }
end
local capture_player_hp
capture_player_hp = function(run_state)
  local player = active_player()
  if not (run_state and player and player.stats) then
    return nil
  end
  local hp = player.stats.hp
  if run_state.min_player_hp == nil or hp < run_state.min_player_hp then
    run_state.min_player_hp = hp
  end
  return hp
end
local record_player_hp_delta
record_player_hp_delta = function(run_state, before_hp)
  local player = active_player()
  if not (run_state and before_hp and player and player.stats) then
    return
  end
  local after_hp = player.stats.hp
  if after_hp < before_hp then
    run_state.gross_player_damage = run_state.gross_player_damage + (before_hp - after_hp)
  elseif after_hp > before_hp then
    run_state.player_healing = run_state.player_healing + (after_hp - before_hp)
  end
  if run_state.min_player_hp == nil or after_hp < run_state.min_player_hp then
    run_state.min_player_hp = after_hp
  end
end
local record_player_status
record_player_status = function(run_state)
  local player = active_player()
  if not (run_state and player) then
    return
  end
  if player:has_effect("Sapped") then
    run_state.sapped_frames = run_state.sapped_frames + 1
  end
end
local nearest_enemy
nearest_enemy = function(player)
  if not (player) then
    return nil, 0
  end
  local closest, closest_dist = nil, nil
  local _list_0 = Map.enemies_list(player)
  for _index_0 = 1, #_list_0 do
    local _continue_0 = false
    repeat
      local enemy = _list_0[_index_0]
      if enemy.destroyed then
        _continue_0 = true
        break
      end
      local dx, dy = enemy.x - player.x, enemy.y - player.y
      local dist = dx * dx + dy * dy
      if not closest_dist or dist < closest_dist then
        closest = enemy
        closest_dist = dist
      end
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return closest, closest_dist or 0
end
local move_toward_nearest_enemy
move_toward_nearest_enemy = function(source, desired_range)
  local player = source and source.player
  local enemy, dist = nearest_enemy(player)
  if not (enemy) then
    return false
  end
  desired_range = desired_range or 48
  if dist <= desired_range * desired_range then
    return false
  end
  local dx, dy = enemy.x - player.x, enemy.y - player.y
  local deadzone = 6
  local dirx
  if math.abs(dx) < deadzone then
    dirx = 0
  elseif dx > 0 then
    dirx = 1
  else
    dirx = -1
  end
  local diry
  if math.abs(dy) < deadzone then
    diry = 0
  elseif dy > 0 then
    diry = 1
  else
    diry = -1
  end
  if dirx == 0 and diry == 0 then
    return false
  end
  source:set("move_direction", {
    dirx,
    diry
  })
  return true
end
local necromancer_eval_input
necromancer_eval_input = function(self)
  local player = self.player
  if player and player.stats then
    local max_hp = player:effective_stats().max_hp
    local frame = require("core.GameState").frame or 0
    if player.stats.hp < max_hp and not player:has_effect("Baleful Regeneration") then
      if (rawget(self, "regen_retry_at") or 0) <= frame then
        self:set("use_spell_slot", 0)
        rawset(self, "regen_retry_at", frame + 45)
        return
      end
    end
  end
  move_toward_nearest_enemy(self, 72)
  return self:set("should_use_weapon", true)
end
local pyrocaster_eval_input
pyrocaster_eval_input = function(self)
  local player = self.player
  if player and player.stats then
    local frame = require("core.GameState").frame or 0
    if player.stats.level >= 3 and not player:has_effect("Dragonform") then
      if (rawget(self, "dragon_retry_at") or 0) <= frame then
        self:set("use_spell_slot", 2)
        rawset(self, "dragon_retry_at", frame + 90)
        return
      end
    end
    if player.stats.level >= 2 and not player:has_effect("Ring of Flames Stat Boost") then
      if (rawget(self, "ring_retry_at") or 0) <= frame then
        self:set("use_spell_slot", 1)
        rawset(self, "ring_retry_at", frame + 90)
        return
      end
    end
  end
  move_toward_nearest_enemy(self, 62)
  return self:set("use_spell_slot", 0)
end
CONFIGS = {
  {
    name = "BaseFighter",
    class = "Fighter",
    primary_damage_type = "Bludgeon",
    input = function(self)
      move_toward_nearest_enemy(self, 40)
      return self:set("should_use_weapon", true)
    end
  },
  {
    name = "BaseNecromancer",
    class = "Necromancer",
    primary_damage_type = "Black",
    input = necromancer_eval_input
  },
  {
    name = "BaseStormcaller",
    class = "Stormcaller",
    primary_damage_type = "White",
    input = function(self)
      move_toward_nearest_enemy(self, 120)
      return self:set("use_spell_slot", 0)
    end
  },
  {
    name = "BasePyrocaster",
    class = "Pyrocaster",
    primary_damage_type = "Red",
    input = pyrocaster_eval_input
  }
}
local dump_stats
dump_stats = function(config, encounter, seed, run_index, outcome, run_state)
  local GameState = require("core.GameState")
  local config_name = config.name
  local config_primary_type = config.primary_damage_type or ""
  local primary_resistance = encounter_primary_resistance(encounter, config_primary_type)
  local primary_resistance_multiplier = resistance_multiplier(primary_resistance)
  local monster = encounter.name
  local player = World.players[1].instance
  local enemy_hp, enemy_max_hp = 0, 0
  local enemies_alive = Map.enemies_list(player)
  for _index_0 = 1, #enemies_alive do
    local _des_0 = enemies_alive[_index_0]
    local stats
    stats = _des_0.stats
    enemy_hp = enemy_hp + stats.hp
    enemy_max_hp = enemy_max_hp + stats.max_hp
  end
  local defender_count, attacker_count, melee_count, projectile_count, receive_melee_count
  do
    local _obj_0 = player:get_effect("DebugMonsterEvalEffect")
    defender_count, attacker_count, melee_count, projectile_count, receive_melee_count = _obj_0.defender_count, _obj_0.attacker_count, _obj_0.melee_count, _obj_0.projectile_count, _obj_0.receive_melee_count
  end
  local net_player_damage = math.max(0, player.stats.max_hp - player.stats.hp)
  local gross_player_damage = math.max(run_state and run_state.gross_player_damage or 0, net_player_damage)
  local player_healing = run_state and run_state.player_healing or 0
  local min_player_hp = run_state and run_state.min_player_hp or player.stats.hp
  local sapped_frames = run_state and run_state.sapped_frames or 0
  local sapped_frame_rate = safe_div(sapped_frames, math.max(1, GameState.frame))
  return {
    config_name = config_name,
    run_index = run_index,
    seed = tostring(seed),
    level = player.stats.level,
    monster = monster,
    spawn_count = encounter.spawn_count,
    monster_spawns = encounter.monster_spawns,
    xpaward = encounter.xpaward,
    source_status = encounter.source_status,
    source_refs = encounter.source_refs,
    config_primary_type = config_primary_type,
    monster_types = encounter.monster_types,
    primary_resistance = primary_resistance,
    primary_resistance_multiplier = primary_resistance_multiplier,
    outcome = outcome or "unknown",
    frames = GameState.frame,
    player_damage = gross_player_damage,
    gross_player_damage = gross_player_damage,
    net_player_damage = net_player_damage,
    player_healing = player_healing,
    min_player_hp = min_player_hp,
    sapped_frames = sapped_frames,
    sapped_frame_rate = sapped_frame_rate,
    enemy_hp = enemy_hp,
    enemy_max_hp = enemy_max_hp,
    enemy_hp_pct = safe_div(enemy_hp, math.max(1, enemy_max_hp)),
    enemy_count_alive = #enemies_alive,
    defender_count = defender_count,
    attacker_count = attacker_count,
    melee_count = melee_count,
    projectile_count = projectile_count,
    receive_melee_count = receive_melee_count
  }
end
local run_bot_tests
run_bot_tests = function(raw_args)
  local parser = argparse("lanarts", "Run monster balance simulations.")
  parser:option("--steps", "Maximum number of steps to take.", 10000)
  parser:option("--runs", "Number of paired seeds to run per config and monster.", 1)
  parser:option("--event_log", "Event log to write to.", nil)
  parser:option("--seed", "Starting seed.", "12345678")
  parser:option("--level", "Player level.", 1)
  parser:option("--levels", "Comma-separated player levels. Overrides --level when set.", nil)
  parser:option("--monster", "Monster to fight. Use comma-separated compact names like GiantRat,Hound or all.", nil)
  parser:option("--pack", "Encounter packs. Format Label=Monster+Monster*COUNT;Other=Hydra+Golem.", nil)
  parser:option("--configs", "Comma-separated eval configs.", DEFAULT_CONFIGS)
  parser:option("--out", "Raw monster run CSV output.", "saves/monster_stats.csv")
  parser:option("--aggregate_out", "Aggregate monster balance CSV output.", "saves/monster_aggregate.csv")
  parser:option("--report_out", "Enemy balance report CSV output.", "saves/monster_report.csv")
  parser:option("--json_out", "JSON output containing raw, aggregate, and report rows.", "saves/monster_stats.json")
  parser:option("--report_delta_pct", "Relative deviation needed for report warnings.", 0.5)
  parser:option("--min_report_runs", "Minimum aggregate runs before report warnings are emitted.", 1)
  parser:option("--inactive_warning_severity", "Severity for warning-class findings on definition-only or summon-only encounters: info or warning.", "info")
  parser:option("--comparison_event_log", "Event log to compare to.", nil)
  local args = parser:parse(raw_args)
  local game_start, config_eval_step, monster_eval_step, game_step
  game_start = function()
    return ResourceLoading.ensure_resources_before(config_eval_step)
  end
  local game_exit
  game_exit = function(msg)
    print(msg)
    return nil
  end
  config_eval_step = function()
    local stats = { }
    local selected_configs = parse_configs(args.configs)
    local selected_encounters = parse_encounters(args.monster, args.pack)
    local selected_levels
    if args.levels then
      selected_levels = parse_levels(args.levels)
    else
      selected_levels = parse_levels(args.level)
    end
    local level_eval_step
    level_eval_step = function(level, next_level)
      return _iterate(selected_configs, monster_eval_step(level, stats, selected_encounters), next_level)
    end
    return _iterate(selected_levels, level_eval_step, function()
      local aggregate_rows = aggregate_stats(stats)
      local report_rows = build_report(aggregate_rows, args.report_delta_pct, args.min_report_runs, args.inactive_warning_severity)
      write_csv(args.out, stats, {
        "config_name",
        "run_index",
        "seed",
        "level",
        "monster",
        "spawn_count",
        "monster_spawns",
        "xpaward",
        "source_status",
        "source_refs",
        "config_primary_type",
        "monster_types",
        "primary_resistance",
        "primary_resistance_multiplier",
        "outcome",
        "frames",
        "player_damage",
        "gross_player_damage",
        "net_player_damage",
        "player_healing",
        "min_player_hp",
        "sapped_frames",
        "sapped_frame_rate",
        "enemy_hp",
        "enemy_max_hp",
        "enemy_hp_pct",
        "enemy_count_alive",
        "defender_count",
        "attacker_count",
        "melee_count",
        "projectile_count",
        "receive_melee_count"
      })
      write_csv(args.aggregate_out, aggregate_rows, {
        "config_name",
        "level",
        "monster",
        "runs",
        "contact_runs",
        "contact_run_rate",
        "spawn_count",
        "monster_spawns",
        "xpaward",
        "source_status",
        "source_refs",
        "config_primary_type",
        "monster_types",
        "primary_resistance",
        "primary_resistance_multiplier",
        "enemy_kill_rate",
        "step_limit_rate",
        "no_contact_step_limit_rate",
        "player_death_rate",
        "avg_frames",
        "min_frames",
        "max_frames",
        "avg_player_damage",
        "min_player_damage",
        "max_player_damage",
        "avg_gross_player_damage",
        "avg_net_player_damage",
        "avg_player_healing",
        "avg_min_player_hp",
        "min_player_hp",
        "avg_sapped_frames",
        "sapped_frames_per_1000_frames",
        "damage_per_1000_frames",
        "damage_per_xp",
        "xp_per_100_damage",
        "net_damage_per_1000_frames",
        "net_damage_per_xp",
        "threat_damage_mode",
        "avg_threat_player_damage",
        "threat_damage_per_1000_frames",
        "threat_damage_per_xp",
        "threat_xp_per_100_damage",
        "contact_avg_frames",
        "contact_min_frames",
        "contact_max_frames",
        "contact_avg_player_damage",
        "contact_min_player_damage",
        "contact_max_player_damage",
        "contact_avg_gross_player_damage",
        "contact_avg_net_player_damage",
        "contact_avg_player_healing",
        "contact_avg_min_player_hp",
        "contact_min_player_hp",
        "contact_avg_sapped_frames",
        "contact_sapped_frames_per_1000_frames",
        "contact_damage_per_1000_frames",
        "contact_damage_per_xp",
        "contact_xp_per_100_damage",
        "contact_net_damage_per_1000_frames",
        "contact_net_damage_per_xp",
        "contact_avg_threat_player_damage",
        "contact_threat_damage_per_1000_frames",
        "contact_threat_damage_per_xp",
        "contact_threat_xp_per_100_damage",
        "avg_enemy_hp_pct",
        "avg_enemy_count_alive",
        "avg_defender_count",
        "avg_attacker_count",
        "avg_melee_count",
        "avg_projectile_count",
        "avg_receive_melee_count"
      })
      write_csv(args.report_out, report_rows, {
        "severity",
        "category",
        "config_name",
        "level",
        "monster",
        "spawn_count",
        "monster_spawns",
        "source_status",
        "source_refs",
        "config_primary_type",
        "monster_types",
        "primary_resistance",
        "primary_resistance_multiplier",
        "threat_damage_mode",
        "contact_avg_gross_player_damage",
        "contact_avg_net_player_damage",
        "contact_avg_player_healing",
        "contact_avg_sapped_frames",
        "contact_sapped_frames_per_1000_frames",
        "metric",
        "value",
        "baseline",
        "delta",
        "delta_pct",
        "message"
      })
      if args.json_out and args.json_out ~= "" then
        file_dump_json(args.json_out, {
          schema_version = 7,
          seed = tostring(args.seed),
          runs_per_config_monster = tonumber(args.runs),
          runs_per_config_encounter = tonumber(args.runs),
          pack_specs = args.pack,
          raw = stats,
          aggregate = aggregate_rows,
          report = report_rows
        })
      end
      print(string.format("Monster eval complete: %d raw runs, %d aggregate rows, %d report rows. Wrote %s, %s, %s, %s.", #stats, #aggregate_rows, #report_rows, args.out, args.aggregate_out, args.report_out, args.json_out))
      for idx, row in ipairs(report_rows) do
        if idx > 10 then
          break
        end
        print(string.format("[%s] %s", row.severity, row.message))
      end
      if #report_rows > 10 then
        print(string.format("... %d more report rows in %s", #report_rows - 10, args.report_out))
      end
      return game_exit("Done.")
    end)
  end
  monster_eval_step = function(level, stats, encounters)
    return function(config, next_config)
      local run_eval_step
      run_eval_step = function(run_index, next_run)
        local seed = tonumber(args.seed) + run_index - 1
        local step
        step = function(encounter, next_encounter)
          print('run', run_index, 'seed', seed, 'level', level, 'config', config.name, 'mon', encounter.name)
          local run_state = make_run_state()
          local on_finish
          on_finish = function(outcome)
            append(stats, dump_stats(config, encounter, seed, run_index, outcome, run_state))
            return next_encounter()
          end
          game_init(seed, config.class, encounter.spawns, config.input, level)
          return game_step(on_finish, run_state)
        end
        return _iterate(encounters, step, next_run)
      end
      local runs
      do
        local _accum_0 = { }
        local _len_0 = 1
        for i = 1, tonumber(args.runs) do
          _accum_0[_len_0] = i
          _len_0 = _len_0 + 1
        end
        runs = _accum_0
      end
      return _iterate(runs, run_eval_step, next_config)
    end
  end
  game_step = function(next_sim, run_state)
    local step
    step = function()
      local GameState = require("core.GameState")
      local GameLoop = require("GameLoop")
      local step_only = true
      local before_hp = capture_player_hp(run_state)
      if #Map.enemies_list(World.players[1].instance) == 0 then
        return next_sim("enemy_killed")
      end
      if args.steps and GameState.frame >= tonumber(args.steps) then
        return next_sim("step_limit")
      end
      if not GameLoop.game_step(step_only) then
        record_player_hp_delta(run_state, before_hp)
        record_player_status(run_state)
        return next_sim("game_loop_exit")
      end
      record_player_hp_delta(run_state, before_hp)
      record_player_status(run_state)
      return step
    end
    require("Events").events.PlayerDeath = function()
      step = function()
        return next_sim("player_death")
      end
      return false
    end
    EngineHooks.game_won = function()
      step = function()
        return next_sim("game_won")
      end
    end
    return step()
  end
  return StartEngine.start_engine({
    settings = {
      fullscreen = false,
      view_width = 640,
      view_height = 480,
      font = 'fonts/Gudea-Regular.ttf',
      menu_font = 'fonts/alagard_by_pix3m-d6awiwp.ttf',
      time_per_step = 0,
      invincible = false,
      regen_on_death = false,
      event_log = args.event_log,
      comparison_event_log = args.comparison_event_log
    },
    entry_point = game_start,
    debug = args.debug,
    on_exit = function()
      return nil
    end
  })
end
return run_bot_tests
