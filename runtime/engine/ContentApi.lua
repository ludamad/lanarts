local json = require "json"
local Catalog = require "engine.ContentCatalog"

local M = {}

local function parse_json_object(raw_json)
    if not raw_json or raw_json == "" then
        return {}
    end
    local ok, parsed = json.parse(raw_json)
    if not ok then
        return {}
    end
    return parsed or {}
end

local function encode(value)
    return json.generate(value)
end

local function copy_object(value)
    local result = {}
    for key, entry in pairs(value or {}) do
        result[key] = entry
    end
    return result
end

local function map_algorithm_by_id(algorithm_id)
    for _, algorithm in ipairs(Catalog.map_algorithms or {}) do
        if (algorithm.id or "") == (algorithm_id or "") then
            return algorithm
        end
    end
    return nil
end

local function map_algorithm_id_for(map_desc, map_profile)
    local fallback
    for _, algorithm in ipairs(Catalog.map_algorithms or {}) do
        if (algorithm.mapDesc or "") == (map_desc or "") then
            if (algorithm.mapProfile or "") == (map_profile or "") then
                return algorithm.id or ""
            end
            if fallback == nil and (algorithm.mapProfile or "") == "" then
                fallback = algorithm.id or ""
            end
        end
    end
    return fallback or ""
end

local function normalize_map_selection(map_desc, map_profile, map_algorithm)
    map_desc = tostring(map_desc or "")
    map_profile = tostring(map_profile or "")
    map_algorithm = tostring(map_algorithm or "")
    local algorithm = map_algorithm ~= "" and map_algorithm_by_id(map_algorithm) or nil
    if algorithm then
        map_desc = algorithm.mapDesc or map_desc
        map_profile = algorithm.mapProfile or ""
    end
    if map_desc == "" and map_profile ~= "" then
        map_desc = "map_descs.Overworld"
    end
    if map_algorithm == "" then
        map_algorithm = map_algorithm_id_for(map_desc, map_profile)
    end
    return map_desc, map_profile, map_algorithm
end

local function normalized_config(config)
    config = config or {}
    local map_desc, map_profile, map_algorithm = normalize_map_selection(
        config.mapDesc or config.start_map_desc or "",
        config.mapProfile or config.map_profile or "",
        config.mapAlgorithm or config.map_algorithm or ""
    )
    return {
        className = tostring(config.className or config.class or "Fighter"),
        level = math.max(1, math.floor(tonumber(config.level) or 1)),
        seed = tostring(config.seed or "1"),
        preset = tostring(config.preset or config.loadout_preset or "none"),
        intensity = tostring(config.intensity or config.packageIntensity or config.package_intensity or "standard"),
        customLoadout = tostring(config.customLoadout or config.loadout or ""),
        mapDesc = map_desc,
        mapProfile = map_profile,
        mapAlgorithm = map_algorithm,
        encounter = tostring(config.encounter or ""),
        monsters = tostring(config.monsters or ""),
        botProfile = tostring(config.botProfile or config.bot_profile or "human"),
        runs = math.max(1, math.floor(tonumber(config.runs) or 5)),
        steps = math.max(1, math.floor(tonumber(config.steps) or 20000)),
    }
end

local function launch_args(config)
    config = normalized_config(config)
    local args = {
        "engine.StartLanarts",
        "--class",
        config.className,
        "--level",
        tostring(config.level),
        "--loadout_preset",
        config.preset,
        "--settings",
        "settings.yaml",
    }
    if config.customLoadout ~= "" then
        args[#args + 1] = "--loadout"
        args[#args + 1] = config.customLoadout
    end
    if config.encounter ~= "" then
        args[#args + 1] = "--encounter"
        args[#args + 1] = config.encounter
        args[#args + 1] = "--package_intensity"
        args[#args + 1] = config.intensity
    end
    if config.monsters ~= "" then
        args[#args + 1] = "--monsters"
        args[#args + 1] = config.monsters
    elseif config.encounter == "" and config.mapDesc ~= "" then
        args[#args + 1] = "--start_map_desc"
        args[#args + 1] = config.mapDesc
        if config.mapProfile ~= "" then
            args[#args + 1] = "--map_profile"
            args[#args + 1] = config.mapProfile
        end
    end
    if config.seed ~= "" then
        args[#args + 1] = "--seed"
        args[#args + 1] = config.seed
    end
    return args
end

local function balance_args(config)
    config = normalized_config(config)
    local args = {
        "engine.StartBalanceRun",
        "--classes",
        config.className,
        "--levels",
        tostring(config.level),
        "--runs",
        tostring(config.runs),
        "--steps",
        tostring(config.steps),
        "--seed",
        config.seed,
        "--bot_profile",
        config.botProfile,
        "--loadout_preset",
        config.preset,
    }
    if config.customLoadout ~= "" then
        args[#args + 1] = "--loadout"
        args[#args + 1] = config.customLoadout
    end
    if config.encounter ~= "" then
        args[#args + 1] = "--encounter"
        args[#args + 1] = config.encounter
        args[#args + 1] = "--package_intensity"
        args[#args + 1] = config.intensity
    end
    if config.monsters ~= "" then
        args[#args + 1] = "--monsters"
        args[#args + 1] = config.monsters
    elseif config.encounter == "" and config.mapDesc ~= "" then
        args[#args + 1] = "--start_map_desc"
        args[#args + 1] = config.mapDesc
        if config.mapProfile ~= "" then
            args[#args + 1] = "--map_profile"
            args[#args + 1] = config.mapProfile
        end
    end
    return args
end

local function copy_array(values)
    local result = {}
    for _, value in ipairs(values or {}) do
        result[#result + 1] = value
    end
    return result
end

local function class_lookup()
    local lookup = {}
    for _, class in ipairs(Catalog.classes) do
        lookup[class.id or ""] = class
    end
    return lookup
end

local function class_build_for(class_name)
    return Catalog.class_builds[class_name or ""] or {}
end

local function spell_rows_for_level(class_name, level)
    local known = {}
    local upcoming = {}
    local class_build = class_build_for(class_name)
    for _, spell in ipairs(class_build.spells or {}) do
        local row = {
            name = spell.name,
            level = spell.level,
            role = spell.role,
        }
        if (tonumber(spell.level) or 1) <= level then
            known[#known + 1] = row
        else
            upcoming[#upcoming + 1] = row
        end
    end
    return known, upcoming
end

local function level_lookup()
    local lookup = {}
    for _, level in ipairs(Catalog.levels) do
        lookup[tonumber(level.value) or 1] = level
    end
    return lookup
end

local function map_lookup()
    local lookup = {}
    for _, map in ipairs(Catalog.maps) do
        lookup[map.id or ""] = map
    end
    return lookup
end

local function encounter_lookup()
    local lookup = {}
    for _, encounter in ipairs(Catalog.encounters) do
        lookup[encounter.id or ""] = encounter
    end
    return lookup
end

local function intensity_lookup()
    local lookup = {}
    for _, intensity in ipairs(Catalog.intensities) do
        lookup[intensity.id or ""] = intensity
    end
    return lookup
end

local function selected_intensity(config)
    local intensity = intensity_lookup()[config.intensity or ""]
    if intensity then
        return intensity
    end
    return intensity_lookup().standard or {
        id = "standard",
        label = "Standard",
        tempo = 1,
        damage = 1,
        summons = 1,
    }
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

local function append_loadout_specs(dest, specs, source)
    for _, raw_spec in ipairs(specs or {}) do
        local spec = type(raw_spec) == "table" and raw_spec or parse_item_spec(raw_spec)
        if spec and spec.item and spec.item ~= "" then
            dest[#dest + 1] = {
                item = spec.item,
                amount = math.max(1, tonumber(spec.amount) or 1),
                source = source,
            }
        end
    end
end

local function parse_loadout_specs(value)
    local specs = {}
    for _, part in ipairs(split_csv(value)) do
        append_loadout_specs(specs, {part}, "custom")
    end
    return specs
end

local function has_value(values, needle)
    for _, value in ipairs(values or {}) do
        if value == needle then
            return true
        end
    end
    return false
end

local function loadout_for_config(config)
    local preset_name = config.preset or "none"
    local preset = Catalog.loadout_presets[preset_name] or Catalog.loadout_presets.none or {}
    local specs = {}
    append_loadout_specs(specs, preset.default, preset_name)
    append_loadout_specs(specs, preset[config.className], preset_name)
    append_loadout_specs(specs, parse_loadout_specs(config.customLoadout), "custom")
    return {
        id = preset_name,
        label = preset.label or preset_name,
        description = preset.description or "",
        items = specs,
    }
end

local function item_spec_for_choice(choice)
    local amount = math.max(1, tonumber(choice.amount) or 1)
    if amount > 1 then
        return string.format("%s*%d", choice.name, amount)
    end
    return choice.name
end

local function choice_relevant_to_class(choice, class_name)
    if not choice.classes or #choice.classes == 0 then
        return true
    end
    return has_value(choice.classes, class_name)
end

local function item_choices_for_config(config)
    local groups = {}
    for _, group in ipairs(Catalog.item_choices or {}) do
        local choices = {}
        for _, choice in ipairs(group.choices or {}) do
            if choice_relevant_to_class(choice, config.className) then
                choices[#choices + 1] = {
                    name = choice.name,
                    amount = math.max(1, tonumber(choice.amount) or 1),
                    spec = item_spec_for_choice(choice),
                    classes = copy_array(choice.classes or {}),
                    tags = copy_array(choice.tags or {}),
                }
            end
        end
        if #choices > 0 then
            groups[#groups + 1] = {
                id = group.id,
                label = group.label,
                choices = choices,
            }
        end
    end
    return groups
end

local function build_options_payload(config)
    local known_spells, upcoming_spells = spell_rows_for_level(config.className, config.level)
    return {
        schema = 1,
        config = config,
        classBuild = class_build_for(config.className),
        knownSpells = known_spells,
        upcomingSpells = upcoming_spells,
        loadout = loadout_for_config(config),
        itemChoices = item_choices_for_config(config),
        args = launch_args(config),
        balanceArgs = balance_args(config),
    }
end

local function scaled_frames(value, fallback, intensity)
    local frames = tonumber(value)
    if frames == nil then
        frames = fallback
    end
    local tempo = math.max(0.25, tonumber(intensity.tempo) or 1)
    return math.max(0, math.floor((tonumber(frames) or 0) / tempo + 0.5))
end

local function scaled_damage(value, fallback, intensity)
    local damage = tonumber(value)
    if damage == nil then
        damage = fallback
    end
    return (tonumber(damage) or 0) * math.max(0.1, tonumber(intensity.damage) or 1)
end

local function scaled_count(value, fallback, intensity)
    local count = tonumber(value)
    if count == nil then
        count = fallback
    end
    return math.max(1, math.ceil((tonumber(count) or 1) * math.max(0.1, tonumber(intensity.summons) or 1)))
end

local function encounter_rhythms(encounter, intensity)
    local package = encounter and encounter.package or {}
    local rows = {}
    if package.orbit then
        local duration = scaled_frames(package.orbit.duration, 1, intensity)
        rows[#rows + 1] = {
            id = "orbit",
            label = "Orbs",
            offset = scaled_frames(package.orbit.offset, 0, intensity),
            interval = scaled_frames(package.orbit.interval, 1, intensity),
            duration = duration,
            count = scaled_count(package.orbit.count, 4, intensity),
            damage = scaled_damage(package.orbit.damage, 3, intensity),
            affectsEscorts = package.orbit.affects_escorts == true,
        }
    end
    if package.pulse then
        local windup = scaled_frames(package.pulse.windup, 0, intensity)
        local active = scaled_frames(package.pulse.active, 1, intensity)
        rows[#rows + 1] = {
            id = "pulse",
            label = "AOE",
            offset = scaled_frames(package.pulse.offset, 0, intensity),
            interval = scaled_frames(package.pulse.interval, 1, intensity),
            duration = windup + active,
            activeStart = windup,
            windup = windup,
            active = active,
            radius = package.pulse.radius,
            damage = scaled_damage(package.pulse.damage, 4, intensity),
            affectsEscorts = package.pulse.affects_escorts == true,
        }
    end
    if package.escort then
        rows[#rows + 1] = {
            id = "escort",
            label = "Ally",
            offset = 0,
            interval = 600,
            duration = 600,
            monster = package.escort.monster or "ally",
            hp = package.escort.hp,
        }
    end
    if package.rescue then
        local windup = scaled_frames(package.rescue.windup, 68, intensity)
        local late_window = scaled_frames(package.rescue.late_window, 22, intensity)
        rows[#rows + 1] = {
            id = "rescue",
            label = "Rescue",
            offset = scaled_frames(package.rescue.offset, 54, intensity),
            interval = scaled_frames(package.rescue.interval, 205, intensity),
            duration = windup,
            activeStart = math.max(0, windup - late_window),
            windup = windup,
            lateWindow = late_window,
            clutchWindow = late_window,
            radius = package.rescue.radius,
            blockRadius = package.rescue.block_radius,
            allyDamage = scaled_damage(package.rescue.ally_damage, 10, intensity),
            blockDamage = scaled_damage(package.rescue.block_damage, 4, intensity),
            pullFrames = scaled_frames(package.rescue.pull_frames, 90, intensity),
        }
    end
    if package.summon then
        rows[#rows + 1] = {
            id = "summon",
            label = "Summons",
            offset = scaled_frames(package.summon.offset, 0, intensity),
            interval = scaled_frames(package.summon.interval, 1, intensity),
            duration = 20,
            monsters = copy_array(package.summon.monsters or {}),
            count = scaled_count(package.summon.count, 1, intensity),
            maxAlive = scaled_count(package.summon.max_alive, 4, intensity),
        }
    end
    return rows
end

local function warnings_for(config, selected)
    local warnings = {}
    if config.level > 5 then
        warnings[#warnings + 1] = "Level 5 is the late-game baseline; higher levels are stress tests."
    elseif config.level == 5 then
        warnings[#warnings + 1] = "Level 5 is late game for player characters."
    end
    if selected.class and selected.class.range == "far" and selected.encounter and selected.encounter.kind and selected.encounter.kind:find("duel") then
        warnings[#warnings + 1] = "Ranged classes should be checked for keep-away uptime in arena encounters."
    end
    if config.className == "Necromancer" then
        warnings[#warnings + 1] = "Necromancer damage taken must be read with regen-adjusted pressure."
    end
    if config.monsters ~= "" and config.encounter ~= "" then
        warnings[#warnings + 1] = "Custom monsters override the encounter's monster list but keep the selected package."
    end
    return warnings
end

local function resource_entry(group, name, meta, goal, config, seed_offset)
    local seed = tonumber(config.seed) or 1
    local resource_config = copy_object(config)
    resource_config.seed = tostring(seed + (seed_offset or 0))
    return {
        id = tostring(group):lower():gsub("[^a-z0-9]+", "-") .. "-" .. tostring(name):lower():gsub("[^a-z0-9]+", "-"),
        group = group,
        name = name,
        meta = meta,
        goal = goal,
        className = config.className,
        level = config.level,
        preset = config.preset,
        intensity = config.intensity,
        customLoadout = config.customLoadout,
        mapDesc = config.mapDesc,
        mapProfile = config.mapProfile,
        mapAlgorithm = config.mapAlgorithm,
        encounter = config.encounter,
        monsters = config.monsters,
        seed = resource_config.seed,
        args = launch_args(resource_config),
        balanceArgs = balance_args(resource_config),
    }
end

local function package_encounters()
    local result = {}
    for _, encounter in ipairs(Catalog.encounters) do
        if encounter.id ~= "" and encounter.package then
            result[#result + 1] = encounter
        end
    end
    return result
end

local function late_maps()
    local result = {}
    for _, map in ipairs(Catalog.maps) do
        if map.id ~= "" and (map.phase == "late" or map.phase == "mid") then
            result[#result + 1] = map
        end
    end
    return result
end

local function algorithms_for_map(map_desc)
    local result = {}
    for _, algorithm in ipairs(Catalog.map_algorithms or {}) do
        if (algorithm.mapDesc or "") == (map_desc or "") then
            result[#result + 1] = algorithm
        end
    end
    return result
end

local function overworld_algorithms()
    local result = {}
    for _, algorithm in ipairs(Catalog.map_algorithms or {}) do
        if (algorithm.mapDesc or "") == "map_descs.Overworld" then
            result[#result + 1] = algorithm
        end
    end
    return result
end

local function selected_map_algorithm(config)
    local algorithm = map_algorithm_by_id(config.mapAlgorithm or "")
    if algorithm then
        return algorithm
    end
    return map_algorithm_by_id(map_algorithm_id_for(config.mapDesc, config.mapProfile))
end

local function map_preview_entry(algorithm, config, seed_offset)
    local preview_config = copy_object(config)
    preview_config.mapDesc = algorithm.mapDesc or ""
    preview_config.mapProfile = algorithm.mapProfile or ""
    preview_config.mapAlgorithm = algorithm.id or ""
    preview_config.encounter = ""
    preview_config.monsters = ""
    local seed = tonumber(config.seed) or 1
    preview_config.seed = tostring(seed + (seed_offset or 0))
    return {
        id = algorithm.id or "",
        label = algorithm.label or algorithm.id or "Map",
        mapDesc = preview_config.mapDesc,
        mapProfile = preview_config.mapProfile,
        mapAlgorithm = preview_config.mapAlgorithm,
        preview_mode = algorithm.preview_mode or "world",
        density = algorithm.density or 0.30,
        phase = algorithm.phase or "",
        description = algorithm.description or "",
        tags = copy_array(algorithm.tags or {}),
        palette = algorithm.palette,
        signature = algorithm.signature,
        runConfig = preview_config,
        args = launch_args(preview_config),
        balanceArgs = balance_args(preview_config),
    }
end

local function map_generation_payload(config)
    local selected_map = map_lookup()[config.mapDesc or ""]
    local selected_algorithm = selected_map_algorithm(config)
    local algorithms = algorithms_for_map(config.mapDesc or "")
    if #algorithms == 0 and selected_map and selected_map.id and selected_map.id ~= "" then
        algorithms[#algorithms + 1] = {
            id = selected_map.id,
            label = selected_map.label,
            mapDesc = selected_map.id,
            mapProfile = "",
            preview_mode = selected_map.preview_mode,
            density = selected_map.density,
            phase = selected_map.phase,
            palette = selected_map.palette,
        }
    end
    if #algorithms == 0 then
        algorithms = algorithms_for_map("")
    end

    local previews = {}
    for idx, algorithm in ipairs(algorithms) do
        previews[#previews + 1] = map_preview_entry(algorithm, config, idx - 1)
    end
    if config.mapDesc ~= "map_descs.Overworld" then
        for idx, algorithm in ipairs(overworld_algorithms()) do
            if #previews >= 4 then
                break
            end
            previews[#previews + 1] = map_preview_entry(algorithm, config, 20 + idx)
        end
    end

    return {
        schema = 1,
        seed = config.seed,
        config = config,
        selected = selected_map or Catalog.maps[1],
        selectedAlgorithm = selected_algorithm,
        algorithms = algorithms,
        previews = previews,
    }
end

function M.catalog_json()
    return encode(Catalog.catalog())
end

function M.launch_args_json(raw_json)
    local config = normalized_config(parse_json_object(raw_json))
    return encode({
        schema = 1,
        config = config,
        args = launch_args(config),
    })
end

function M.balance_args_json(raw_json)
    local config = normalized_config(parse_json_object(raw_json))
    return encode({
        schema = 1,
        config = config,
        args = balance_args(config),
    })
end

function M.loadout_options_json(raw_json)
    return encode(build_options_payload(normalized_config(parse_json_object(raw_json))))
end

function M.run_summary_json(raw_json)
    local config = normalized_config(parse_json_object(raw_json))
    local known_spells, upcoming_spells = spell_rows_for_level(config.className, config.level)
    local selected = {
        class = class_lookup()[config.className],
        level = level_lookup()[config.level],
        map = map_lookup()[config.mapDesc or ""],
        mapAlgorithm = selected_map_algorithm(config),
        encounter = encounter_lookup()[config.encounter or ""],
        intensity = selected_intensity(config),
    }
    return encode({
        schema = 1,
        config = config,
        class = selected.class,
        level = selected.level or {value = config.level, label = tostring(config.level)},
        classBuild = class_build_for(config.className),
        knownSpells = known_spells,
        upcomingSpells = upcoming_spells,
        loadout = loadout_for_config(config),
        map = selected.map,
        mapAlgorithm = selected.mapAlgorithm,
        encounter = selected.encounter,
        intensity = selected.intensity,
        rhythms = encounter_rhythms(selected.encounter, selected.intensity),
        warnings = warnings_for(config, selected),
        args = launch_args(config),
        balanceArgs = balance_args(config),
    })
end

function M.map_preview_json(raw_json)
    local config = normalized_config(parse_json_object(raw_json))
    local payload = map_generation_payload(config)
    payload.encounter = encounter_lookup()[config.encounter or ""]
    return encode(payload)
end

function M.map_generation_json(raw_json)
    return encode(map_generation_payload(normalized_config(parse_json_object(raw_json))))
end

function M.resource_matrix_json(raw_json)
    local config = normalized_config(parse_json_object(raw_json))
    local resources = {}
    local base_map = config.mapDesc
    local base_encounter = config.encounter
    local base_intensity = config.intensity
    local selected_map = map_lookup()[base_map or ""]
    local selected_encounter = encounter_lookup()[base_encounter or ""]
    local selected_target_label = selected_encounter and selected_encounter.id ~= "" and selected_encounter.label
        or selected_map and selected_map.label
        or "selected map"

    for idx, class in ipairs(Catalog.classes) do
        local next_config = {}
        for key, value in pairs(config) do
            next_config[key] = value
        end
        next_config.className = class.id
        resources[#resources + 1] = resource_entry(
            "Class sweep",
            class.label or class.id,
            string.format("L%d, %s", next_config.level, selected_target_label),
            class.balance_focus or "class comparison",
            next_config,
            idx - 1
        )
    end

    if selected_encounter and selected_encounter.package then
        for idx, intensity in ipairs(Catalog.intensities) do
            local next_config = {}
            for key, value in pairs(config) do
                next_config[key] = value
            end
            next_config.intensity = intensity.id
            resources[#resources + 1] = resource_entry(
                "Pressure sweep",
                intensity.label or intensity.id,
                string.format("%s, %.2fx tempo", selected_encounter.label or selected_encounter.id, tonumber(intensity.tempo) or 1),
                intensity.description or "package pressure comparison",
                next_config,
                20 + idx
            )
        end
    end

    for idx, map in ipairs(late_maps()) do
        local next_config = {}
        for key, value in pairs(config) do
            next_config[key] = value
        end
        next_config.level = 5
        next_config.preset = next_config.preset == "none" and "late" or next_config.preset
        next_config.mapDesc = map.id
        next_config.mapProfile = ""
        next_config.mapAlgorithm = map_algorithm_id_for(map.id, "")
        next_config.encounter = ""
        resources[#resources + 1] = resource_entry(
            "Map sweep",
            map.label or map.id,
            string.format("L%d, %s loadout", next_config.level, next_config.preset),
            "late-map enemy pool comparison",
            next_config,
            40 + idx
        )
    end

    for idx, algorithm in ipairs(overworld_algorithms()) do
        local next_config = {}
        for key, value in pairs(config) do
            next_config[key] = value
        end
        next_config.level = math.min(5, math.max(1, next_config.level))
        next_config.mapDesc = algorithm.mapDesc or "map_descs.Overworld"
        next_config.mapProfile = algorithm.mapProfile or ""
        next_config.mapAlgorithm = algorithm.id or ""
        next_config.encounter = ""
        next_config.monsters = ""
        resources[#resources + 1] = resource_entry(
            "Overworld algorithm sweep",
            algorithm.label or algorithm.id,
            string.format("L%d, %s", next_config.level, algorithm.phase or "opening"),
            algorithm.description or "initial-map generation comparison",
            next_config,
            55 + idx
        )
    end

    for idx, encounter in ipairs(package_encounters()) do
        local next_config = {}
        for key, value in pairs(config) do
            next_config[key] = value
        end
        next_config.level = 5
        next_config.preset = next_config.preset == "none" and "late" or next_config.preset
        next_config.mapDesc = ""
        next_config.mapProfile = ""
        next_config.mapAlgorithm = ""
        next_config.encounter = encounter.id
        next_config.intensity = base_intensity
        resources[#resources + 1] = resource_entry(
            "Encounter sweep",
            encounter.label or encounter.id,
            string.format("L%d, %s", next_config.level, encounter.phase or encounter.kind or "package"),
            encounter.description or "encounter comparison",
            next_config,
            70 + idx
        )
    end

    return encode({
        schema = 1,
        config = config,
        count = #resources,
        resources = resources,
    })
end

return M
