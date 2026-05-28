local json = require "json"
local Catalog = require "engine.ContentCatalog"

local M = {}

local MONSTER_EVAL_CONFIG_BY_CLASS = {
    Fighter = "BaseFighter",
    Necromancer = "BaseNecromancer",
    Pyrocaster = "BasePyrocaster",
    Stormcaller = "BaseStormcaller",
    Templar = "BaseFighter",
    Rogue = "BaseFighter",
}

local DEFAULT_MONSTER_EVAL_CONFIGS = "BaseFighter,BaseNecromancer,BaseStormcaller,BasePyrocaster"
local action_guide_payload
local telemetry_watch_for

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

local function normalize_bot_profile_name(value)
    local profile_name = tostring(value or "human")
    if profile_name == "" then
        profile_name = "human"
    end
    for _, profile in ipairs(Catalog.bot_profiles or {}) do
        if (profile.id or "") == profile_name then
            return profile_name
        end
    end
    return "human"
end

local COMPONENT_ALIASES = {
    ally = "escort",
    aoe = "pulse",
    add = "summon",
    adds = "summon",
    lane = "sweep",
    lanes = "sweep",
    orbs = "orbit",
    rescue_lane = "rescue",
    sweeps = "sweep",
}

local COMPONENT_ORDER = {"escort", "orbit", "pulse", "sweep", "summon", "rescue"}
local COMPONENT_KEYS = {}
for _, component in ipairs(COMPONENT_ORDER) do
    COMPONENT_KEYS[component] = true
end

local function copy_package_metadata(package)
    local result = {}
    for key, value in pairs(package or {}) do
        if not COMPONENT_KEYS[key] then
            result[key] = value
        end
    end
    return result
end

local function normalize_package_components(value)
    local selected = {}
    for part in tostring(value or ""):gmatch("[^,]+") do
        part = part:gsub("^%s+", ""):gsub("%s+$", ""):lower():gsub("[^a-z_]+", "_")
        part = COMPONENT_ALIASES[part] or part
        if part == "full" or part == "all" then
            return ""
        end
        selected[part] = true
    end
    if selected.rescue then
        selected.escort = true
    end
    local parts = {}
    for _, component in ipairs(COMPONENT_ORDER) do
        if selected[component] then
            parts[#parts + 1] = component
        end
    end
    return table.concat(parts, ",")
end

local function package_component_lookup(value)
    local selected = {}
    for _, component in ipairs(COMPONENT_ORDER) do
        selected[component] = false
    end
    local normalized = normalize_package_components(value)
    if normalized == "" then
        return nil
    end
    for part in normalized:gmatch("[^,]+") do
        selected[part] = true
    end
    return selected
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
        packageComponents = normalize_package_components(config.packageComponents or config.package_components or ""),
        customLoadout = tostring(config.customLoadout or config.loadout or ""),
        mapDesc = map_desc,
        mapProfile = map_profile,
        mapAlgorithm = map_algorithm,
        encounter = tostring(config.encounter or ""),
        drill = tostring(config.drill or config.microDrill or config.micro_drill or ""),
        monsters = tostring(config.monsters or ""),
        goal = tostring(config.goal or config.analysisGoal or config.analysis_goal or "enemy-balance"),
        enemyGroup = tostring(config.enemyGroup or config.enemy_group or ""),
        enemyPack = tostring(config.enemyPack or config.enemy_pack or ""),
        monsterConfigs = tostring(config.monsterConfigs or config.monster_configs or ""),
        monsterLevels = tostring(config.monsterLevels or config.monster_levels or config.levels or ""),
        botProfile = normalize_bot_profile_name(config.botProfile or config.bot_profile or "human"),
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
        if config.packageComponents ~= "" then
            args[#args + 1] = "--package_components"
            args[#args + 1] = config.packageComponents
        end
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
        if config.packageComponents ~= "" then
            args[#args + 1] = "--package_components"
            args[#args + 1] = config.packageComponents
        end
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

local STAT_ROWS = {
    {"hp", "HP"},
    {"mp", "MP"},
    {"strength", "STR"},
    {"defence", "DEF"},
    {"magic", "MAG"},
    {"willpower", "WIL"},
}

local function class_stats_for_level(class_name, level)
    local class_build = class_build_for(class_name)
    local base = class_build.base or {}
    local per_level = class_build.per_level or {}
    local level_index = math.max(0, (tonumber(level) or 1) - 1)
    local values = {}
    local rows = {}
    for _, entry in ipairs(STAT_ROWS) do
        local id, label = entry[1], entry[2]
        local value = (tonumber(base[id]) or 0) + (tonumber(per_level[id]) or 0) * level_index
        value = math.floor(value + 0.5)
        values[id] = value
        rows[#rows + 1] = {
            id = id,
            label = label,
            value = value,
        }
    end
    return {
        level = tonumber(level) or 1,
        values = values,
        rows = rows,
    }
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

local function bot_profile_lookup()
    local lookup = {}
    for _, profile in ipairs(Catalog.bot_profiles or {}) do
        lookup[profile.id or ""] = profile
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

local function selected_bot_profile(config)
    local lookup = bot_profile_lookup()
    return lookup[config.botProfile or ""]
        or lookup.human
        or {
            id = "human",
            label = "Human",
            decisionInterval = 4,
            spellSkipChance = 0.12,
            itemSkipChance = 0.14,
            weaponSkipChance = 0.06,
            movementNoiseChance = 0.07,
        }
end

local function control_model_for(config)
    local class = class_lookup()[config.className] or {}
    local profile = selected_bot_profile(config)
    return {
        className = config.className,
        classRole = class.role or "",
        classRange = class.range or "",
        positioning = class.positioning or "",
        targetDistance = class.targetDistance or (class.range == "far" and 120 or 46),
        notes = class.control_notes or class.balance_focus or "",
        botProfile = profile,
        decisionInterval = profile.decisionInterval or 1,
        spellSkipChance = profile.spellSkipChance or 0,
        itemSkipChance = profile.itemSkipChance or 0,
        weaponSkipChance = profile.weaponSkipChance or 0,
        movementNoiseChance = profile.movementNoiseChance or 0,
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

local function join_names(values)
    return table.concat(values or {}, ",")
end

local function enemy_group_lookup()
    local lookup = {}
    for _, group in ipairs(Catalog.enemy_groups or {}) do
        lookup[group.id or ""] = group
    end
    return lookup
end

local function enemy_pack_lookup()
    local lookup = {}
    for _, pack in ipairs(Catalog.enemy_packs or {}) do
        lookup[pack.id or ""] = pack
    end
    return lookup
end

local function analysis_goal_lookup()
    local lookup = {}
    for _, goal in ipairs(Catalog.analysis_goals or {}) do
        lookup[goal.id or ""] = goal
    end
    return lookup
end

local function selected_enemy_group(config)
    local lookup = enemy_group_lookup()
    if lookup[config.enemyGroup or ""] then
        return lookup[config.enemyGroup]
    end
    return Catalog.enemy_groups and Catalog.enemy_groups[1] or nil
end

local function selected_enemy_pack(config)
    local lookup = enemy_pack_lookup()
    if lookup[config.enemyPack or ""] then
        return lookup[config.enemyPack]
    end
    return nil
end

local function monster_eval_configs_for(config)
    if config.monsterConfigs ~= "" then
        return config.monsterConfigs
    end
    if config.goal == "enemy-balance" or config.goal == "caster-delta" or config.goal == "class-buffer" then
        return DEFAULT_MONSTER_EVAL_CONFIGS
    end
    return MONSTER_EVAL_CONFIG_BY_CLASS[config.className] or DEFAULT_MONSTER_EVAL_CONFIGS
end

local function monster_eval_levels_for(config, fallback_level)
    if config.monsterLevels ~= "" then
        return config.monsterLevels
    end
    return tostring(fallback_level or config.level or 1)
end

local function monster_eval_args(config, selection)
    config = normalized_config(config)
    selection = selection or {}
    local args = {
        "engine.StartMonsterEvals",
        "--steps",
        tostring(math.max(100, math.min(config.steps, 20000))),
        "--runs",
        tostring(config.runs),
        "--seed",
        config.seed,
        "--levels",
        monster_eval_levels_for(config, selection.level),
        "--configs",
        selection.configs or monster_eval_configs_for(config),
    }
    if selection.pack and selection.pack ~= "" then
        args[#args + 1] = "--pack"
        args[#args + 1] = selection.pack
    else
        local monsters = selection.monsters
        if not monsters or monsters == "" then
            monsters = config.monsters ~= "" and config.monsters or "all"
        end
        args[#args + 1] = "--monster"
        args[#args + 1] = monsters
    end
    return args
end

local function enemy_choice_rows(group)
    local rows = {}
    for _, choice in ipairs(group and group.choices or {}) do
        rows[#rows + 1] = {
            name = choice.name,
            role = choice.role or "",
            suggestedLevel = choice.suggestedLevel or 1,
            tags = copy_array(choice.tags or {}),
        }
    end
    return rows
end

local function all_enemy_choices()
    local rows = {}
    local seen = {}
    for _, group in ipairs(Catalog.enemy_groups or {}) do
        for _, choice in ipairs(group.choices or {}) do
            if choice.name and not seen[choice.name] then
                local row = copy_object(choice)
                row.group = group.id
                row.groupLabel = group.label
                rows[#rows + 1] = row
                seen[choice.name] = true
            end
        end
    end
    return rows
end

local function enemy_options_payload(config)
    local group = selected_enemy_group(config)
    local pack = selected_enemy_pack(config)
    local selected_monsters = config.monsters ~= "" and config.monsters
        or pack and join_names(pack.monsters)
        or group and join_names((function()
            local names = {}
            for _, choice in ipairs(group.choices or {}) do
                names[#names + 1] = choice.name
            end
            return names
        end)())
        or "all"
    return {
        schema = 1,
        config = config,
        goals = Catalog.analysis_goals or {},
        selectedGoal = analysis_goal_lookup()[config.goal] or (Catalog.analysis_goals and Catalog.analysis_goals[1]),
        enemyGroups = Catalog.enemy_groups or {},
        enemyPacks = Catalog.enemy_packs or {},
        selectedGroup = group,
        selectedPack = pack,
        choices = enemy_choice_rows(group),
        selectedMonsters = split_csv(selected_monsters),
        monsterEvalArgs = monster_eval_args(config, {
            monsters = pack and nil or selected_monsters,
            pack = pack and pack.pack or nil,
        }),
    }
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

local function tag_lookup_for_items()
    local lookup = {}
    for _, group in ipairs(Catalog.item_choices or {}) do
        for _, choice in ipairs(group.choices or {}) do
            lookup[choice.name or ""] = copy_array(choice.tags or {})
        end
    end
    return lookup
end

local function add_trait(traits, key, amount)
    traits[key] = (traits[key] or 0) + (amount or 1)
end

local function loadout_traits(config, loadout, known_spells)
    local traits = {}
    local tags_by_item = tag_lookup_for_items()
    local specs = {}
    local class_build = class_build_for(config.className)
    if class_build.starting_weapon then
        specs[#specs + 1] = {item = class_build.starting_weapon, amount = 1}
    end
    for _, item in ipairs(class_build.starting_items or {}) do
        specs[#specs + 1] = {item = item, amount = 1}
    end
    for _, spec in ipairs(loadout and loadout.items or {}) do
        specs[#specs + 1] = spec
    end
    for _, spec in ipairs(specs) do
        local item = tostring(spec.item or "")
        local amount = math.max(1, tonumber(spec.amount) or 1)
        local lower = item:lower()
        for _, tag in ipairs(tags_by_item[item] or {}) do
            add_trait(traits, tag, amount)
        end
        if lower:find("potion") then
            if lower:find("mana") then
                add_trait(traits, "casting", amount)
            else
                add_trait(traits, "survival", amount)
            end
            add_trait(traits, "consumable", amount)
        end
        if lower:find("vitality") or lower:find("healing") or lower:find("fortification")
            or lower:find("stone") or lower:find("plate") or lower:find("chainmail")
            or lower:find("armour") then
            add_trait(traits, "survival", amount)
        end
        if lower:find("staff") or lower:find("wizard") or lower:find("magician")
            or lower:find("spells") or lower:find("robe") or lower:find("moonstone")
            or lower:find("wishful") then
            add_trait(traits, "casting", amount)
        end
        if lower:find("pain") then
            add_trait(traits, "regen", amount)
        end
        if lower:find("fear") then
            add_trait(traits, "control", amount)
        end
        if lower:find("scimitar") or lower:find("broadsword") or lower:find("club")
            or lower:find("mace") or lower:find("slaying") or lower:find("vampirism") then
            add_trait(traits, "melee", amount)
        end
        if lower:find("boots") or lower:find("dash") then
            add_trait(traits, "mobility", amount)
        end
    end
    for _, spell in ipairs(known_spells or {}) do
        local role = tostring(spell.role or "")
        if role ~= "" then
            add_trait(traits, role, 1)
        end
        if role == "gap close" or role == "mobility" then
            add_trait(traits, "mobility", 1)
        elseif role == "regen" then
            add_trait(traits, "regen", 1)
        elseif role == "summon" then
            add_trait(traits, "summon", 1)
        elseif role == "control" or role == "melee control" then
            add_trait(traits, "control", 1)
        elseif role == "burst" then
            add_trait(traits, "damage", 1)
        end
    end
    if config.className == "Necromancer" then
        add_trait(traits, "regen", 1)
    elseif config.className == "Fighter" or config.className == "Rogue" or config.className == "Templar" then
        add_trait(traits, "melee", 1)
    else
        add_trait(traits, "casting", 1)
    end
    return traits
end

local function package_for_readiness(config)
    local encounter = encounter_lookup()[config.encounter or ""]
    local package = encounter and encounter.package
    if not package then
        return nil, encounter
    end
    local selected = package_component_lookup(config.packageComponents)
    if not selected then
        return package, encounter
    end
    local filtered = copy_package_metadata(package)
    for _, component in ipairs(COMPONENT_ORDER) do
        if selected[component] and package[component] then
            filtered[component] = package[component]
        end
    end
    return filtered, encounter
end

local function loadout_readiness_for_config(config, loadout, known_spells)
    local traits = loadout_traits(config, loadout, known_spells)
    local package = package_for_readiness(config)
    local stats = class_stats_for_level(config.className, config.level).values or {}
    local control_model = control_model_for(config)
    local score = 45
    local demands = {}
    local strengths = {}
    local gaps = {}
    local notes = {}

    local function demand(key, label)
        demands[#demands + 1] = {id = key, label = label}
    end
    local function strength(text)
        strengths[#strengths + 1] = text
        score = score + 7
    end
    local function gap(text)
        gaps[#gaps + 1] = text
        score = score - 9
    end

    if package and package.rescue then
        demand("rescue", "body-block rescue")
        if (traits.survival or 0) >= 3 or (traits.regen or 0) >= 2 or (stats.hp or 0) >= 175 then
            strength("block survival")
        else
            gap("thin block survival")
        end
        if (traits.mobility or 0) > 0 or (traits.regen or 0) > 0 then
            strength("rescue commit tool")
        else
            gap("no clear rescue commit tool")
        end
    end
    if package and package.summon then
        demand("summon", "add cleanup")
        if (traits.summon or 0) > 0 or (traits.damage or 0) > 0 or (traits.melee or 0) >= 2 or (traits.casting or 0) >= 2 then
            strength("add cleanup")
        else
            gap("weak add cleanup")
        end
    end
    if package and package.pulse then
        demand("pulse", "AOE movement")
        if (traits.mobility or 0) > 0 or (control_model.classRange or "") == "ranged" then
            strength("AOE repositioning")
        else
            notes[#notes + 1] = "AOE reads rely on footwork rather than an explicit mobility tool."
        end
    end
    if package and package.orbit then
        demand("orbit", "orb spacing")
        if (traits.survival or 0) >= 2 or (control_model.classRange or "") == "ranged" then
            strength("orb tolerance")
        else
            notes[#notes + 1] = "Orb hits will quickly reveal whether the build has enough margin."
        end
    end
    if not package then
        demand("general", "map/enemy probe")
        if (traits.survival or 0) > 0 then
            strength("survival margin")
        end
        if (traits.casting or 0) > 0 or (traits.melee or 0) > 0 then
            strength("primary offense")
        end
    end

    if config.className == "Necromancer" and (traits.regen or 0) > 0 then
        notes[#notes + 1] = "Necromancer sustain is regen-adjusted; gross damage alone will mislead."
    end
    if (traits.consumable or 0) >= 4 then
        notes[#notes + 1] = "Consumable-heavy loadout can hide pacing or damage problems in short runs."
    end

    score = math.max(0, math.min(100, score))
    local label = score >= 78 and "ready"
        or score >= 58 and "workable"
        or score >= 38 and "thin"
        or "risky"
    return {
        schema = 1,
        score = score,
        label = label,
        traits = traits,
        demands = demands,
        strengths = strengths,
        gaps = gaps,
        notes = notes,
    }
end

local function build_archetype_by_id(class_name, build_id)
    for _, archetype in ipairs((Catalog.build_archetypes or {})[class_name] or {}) do
        if (archetype.id or "") == (build_id or "") then
            return archetype
        end
    end
    return nil
end

local function apply_build_archetype(config, build_id)
    local next_config = copy_object(config)
    local archetype = build_archetype_by_id(next_config.className, build_id)
    if archetype then
        next_config.preset = archetype.preset or "none"
        next_config.customLoadout = join_names(archetype.items or {})
    end
    return next_config, archetype
end

local function build_archetype_rows(config)
    local rows = {}
    for _, archetype in ipairs((Catalog.build_archetypes or {})[config.className] or {}) do
        local next_config = apply_build_archetype(config, archetype.id)
        next_config.className = config.className
        local known_spells, upcoming_spells = spell_rows_for_level(next_config.className, next_config.level)
        local loadout = loadout_for_config(next_config)
        local readiness = loadout_readiness_for_config(next_config, loadout, known_spells)
        rows[#rows + 1] = {
            id = archetype.id or tostring(config.className):lower(),
            label = archetype.label or archetype.id or config.className,
            role = archetype.role or "",
            preset = next_config.preset,
            customLoadout = next_config.customLoadout,
            tags = copy_array(archetype.tags or {}),
            config = copy_object(next_config),
            loadout = loadout,
            loadoutReadiness = readiness,
            knownSpells = known_spells,
            upcomingSpells = upcoming_spells,
            args = launch_args(next_config),
            balanceArgs = balance_args(next_config),
        }
    end
    return rows
end

local function build_matrix_payload(config)
    local rows = build_archetype_rows(config)
    return {
        schema = 1,
        config = config,
        count = #rows,
        builds = rows,
    }
end

local function build_options_payload(config)
    local known_spells, upcoming_spells = spell_rows_for_level(config.className, config.level)
    local loadout = loadout_for_config(config)
    return {
        schema = 1,
        config = config,
        classBuild = class_build_for(config.className),
        resolvedStats = class_stats_for_level(config.className, config.level),
        controlModel = control_model_for(config),
        knownSpells = known_spells,
        upcomingSpells = upcoming_spells,
        loadout = loadout,
        loadoutReadiness = loadout_readiness_for_config(config, loadout, known_spells),
        actionGuide = action_guide_payload and action_guide_payload(config) or nil,
        buildArchetypes = build_archetype_rows(config),
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

local function scaled_summon_count(summon, key, fallback, intensity)
    local count = tonumber(summon and summon[key])
    if count == nil then
        count = fallback
    end
    if summon and summon.scale_with_intensity == false then
        return math.max(1, math.ceil(count))
    end
    return scaled_count(count, fallback, intensity)
end

local function safe_rate(num, denom)
    denom = tonumber(denom) or 0
    if denom <= 0 then
        return 0
    end
    return (tonumber(num) or 0) / denom
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
    if package.sweep then
        local windup = scaled_frames(package.sweep.windup, 48, intensity)
        local active = scaled_frames(package.sweep.active, 34, intensity)
        rows[#rows + 1] = {
            id = "sweep",
            label = "Sweep",
            offset = scaled_frames(package.sweep.offset, 52, intensity),
            interval = scaled_frames(package.sweep.interval, 180, intensity),
            duration = windup + active,
            activeStart = windup,
            windup = windup,
            active = active,
            width = package.sweep.width,
            length = package.sweep.length,
            travel = package.sweep.travel,
            damage = scaled_damage(package.sweep.damage, 3, intensity),
            affectsEscorts = package.sweep.affects_escorts == true,
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
        local perfect_window = scaled_frames(package.rescue.perfect_window, 6, intensity)
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
            perfectWindow = perfect_window,
            radius = package.rescue.radius,
            blockRadius = package.rescue.block_radius,
            allyDamage = scaled_damage(package.rescue.ally_damage, 10, intensity),
            blockDamage = scaled_damage(package.rescue.block_damage, 4, intensity),
            pullFrames = scaled_frames(package.rescue.pull_frames, 90, intensity),
            clutchCounterDamage = scaled_damage(package.rescue.clutch_counter_damage, 0, intensity),
            perfectCounterDamage = scaled_damage(package.rescue.perfect_counter_damage, 0, intensity),
            clutchCounterStun = scaled_frames(package.rescue.clutch_counter_stun, 0, intensity),
            perfectCounterStun = scaled_frames(package.rescue.perfect_counter_stun, 0, intensity),
        }
    end
    if package.summon then
        local windup = scaled_frames(package.summon.windup, 24, intensity)
        rows[#rows + 1] = {
            id = "summon",
            label = "Summons",
            offset = scaled_frames(package.summon.offset, 0, intensity),
            interval = scaled_frames(package.summon.interval, 1, intensity),
            duration = windup,
            windup = windup,
            monsters = copy_array(package.summon.monsters or {}),
            count = scaled_summon_count(package.summon, "count", 1, intensity),
            maxAlive = scaled_summon_count(package.summon, "max_alive", 4, intensity),
        }
    end
    return rows
end

local function filtered_package(package, components)
    if not package then
        return nil
    end
    local selected = package_component_lookup(components)
    if not selected then
        return package
    end
    local result = copy_package_metadata(package)
    for _, component in ipairs(COMPONENT_ORDER) do
        if selected[component] and package[component] then
            result[component] = package[component]
        end
    end
    return result
end

local function encounter_with_components(encounter, components)
    if not encounter or not encounter.package then
        return encounter
    end
    local selected = package_component_lookup(components)
    if not selected then
        return encounter
    end
    local result = copy_object(encounter)
    result.package = filtered_package(encounter.package, components)
    return result
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
    if selected.encounter and selected.encounter.kind and selected.encounter.kind:find("baseline") then
        warnings[#warnings + 1] = "Baseline duels are control cases; use package variants for encounter tuning."
    end
    if config.packageComponents ~= "" and selected.encounter and selected.encounter.package then
        local components = package_component_lookup(config.packageComponents)
        local has_component = false
        for component, enabled in pairs(components or {}) do
            if enabled and selected.encounter.package[component] then
                has_component = true
            end
        end
        if not has_component then
            warnings[#warnings + 1] = "Selected package parts are not present on this encounter."
        end
    end
    if config.botProfile == "perfect" then
        warnings[#warnings + 1] = "Perfect automation is a ceiling profile; use human/sloppy sweeps for fairness checks."
    elseif config.botProfile == "sloppy" then
        warnings[#warnings + 1] = "Sloppy automation is a lower-bound stress profile for dropped actions."
    end
    if config.className == "Necromancer" then
        warnings[#warnings + 1] = "Necromancer damage taken must be read with regen-adjusted pressure."
    end
    if config.monsters ~= "" and config.encounter ~= "" then
        warnings[#warnings + 1] = "Custom monsters override the encounter's monster list but keep the selected package."
    end
    return warnings
end

local function resource_entry(group, name, meta, goal, config, seed_offset, extras)
    local seed = tonumber(config.seed) or 1
    local resource_config = copy_object(config)
    resource_config.seed = tostring(seed + (seed_offset or 0))
    local entry = {
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
        packageComponents = config.packageComponents,
        botProfile = config.botProfile,
        mapDesc = config.mapDesc,
        mapProfile = config.mapProfile,
        mapAlgorithm = config.mapAlgorithm,
        encounter = config.encounter,
        drill = config.drill,
        monsters = config.monsters,
        analysisGoal = config.goal,
        enemyGroup = config.enemyGroup,
        enemyPack = config.enemyPack,
        seed = resource_config.seed,
        args = launch_args(resource_config),
        balanceArgs = balance_args(resource_config),
    }
    for key, value in pairs(extras or {}) do
        entry[key] = value
    end
    return entry
end

local function append_build_archetype_resources(resources, config, start_offset)
    for idx, build in ipairs(build_archetype_rows(config)) do
        resources[#resources + 1] = resource_entry(
            "Build archetype sweep",
            build.label,
            string.format("L%d, %s", build.config.level or config.level, build.role or "build"),
            "spec comparison against the selected fight",
            build.config,
            (start_offset or 10) + idx,
            {
                buildArchetype = {
                    id = build.id,
                    label = build.label,
                    role = build.role,
                    tags = copy_array(build.tags or {}),
                },
                loadoutReadiness = build.loadoutReadiness,
            }
        )
    end
end

local function build_archetype_resources(config, start_offset)
    local resources = {}
    append_build_archetype_resources(resources, config, start_offset)
    return resources
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

local function encounter_resource_level(encounter)
    local level = tonumber(encounter and (encounter.suggestedLevel or encounter.level))
    if level == nil then
        level = 5
    end
    return math.min(5, math.max(1, math.floor(level)))
end

local function encounter_resource_preset(encounter, current_preset, level)
    if encounter and encounter.suggestedPreset ~= nil then
        return tostring(encounter.suggestedPreset)
    end
    if (current_preset or "") == "none" and (level or 1) >= 5 then
        return "late"
    end
    return current_preset
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

local function hash_text(value)
    local hash = 5381
    local text = tostring(value or "")
    for idx = 1, #text do
        hash = (hash * 33 + text:byte(idx)) % 2147483647
    end
    return hash
end

local function preview_rng(seed)
    local state = math.floor(tonumber(seed) or hash_text(seed))
    state = state % 2147483647
    if state <= 0 then
        state = 1
    end
    return function()
        state = (state * 48271) % 2147483647
        return state / 2147483647
    end
end

local function trim(value)
    return tostring(value or ""):gsub("^%s+", ""):gsub("%s+$", "")
end

local function roll_signature_count(value, random, fallback)
    if type(value) == "number" then
        return math.max(0, math.floor(value + 0.5))
    end
    local text = trim(value)
    if text == "" or text == "mixed" or text == "profiled" then
        return fallback
    end
    local low, high = text:match("^([0-9]+)%s*%-%s*([0-9]+)$")
    if low and high then
        low = tonumber(low) or fallback
        high = tonumber(high) or low
        if high < low then
            low, high = high, low
        end
        return low + math.floor(random() * (high - low + 1))
    end

    local total = 0
    local matched = false
    for term in text:gmatch("[^%+]+") do
        term = trim(term)
        local dice_count, sides = term:match("^([0-9]*)d([0-9]+)$")
        if sides then
            matched = true
            dice_count = tonumber(dice_count) or 1
            sides = tonumber(sides) or 1
            for _ = 1, math.max(1, dice_count) do
                total = total + 1 + math.floor(random() * math.max(1, sides))
            end
        else
            local number = tonumber(term)
            if number then
                matched = true
                total = total + number
            end
        end
    end
    if matched then
        return math.max(0, math.floor(total + 0.5))
    end
    return fallback
end

local function preview_enemy_names(signature, mode)
    local mix = tostring(signature and signature.enemyMix or ""):lower()
    local names = {}
    local function add(label)
        names[#names + 1] = label
    end
    if mix:find("mamba") then
        add("Black Mamba")
    end
    if mix:find("cloud") then
        add("Cloud Elemental")
    end
    if mix:find("mouther") then
        add("Mouther")
    end
    if mix:find("hound") then
        add("Hound")
    end
    if mix:find("skeleton") then
        add("Skeleton")
    end
    if #names == 0 and tostring(mode or ""):find("overworld") then
        names = {"Sheep", "Black Mamba", "Skeleton", "Cloud Elemental"}
    elseif #names == 0 then
        names = {"ambient"}
    end
    return names
end

local MAP_ENEMY_THREAT = {
    ["Sheep"] = {weight = 0.25, contact = 0, role = "ambient"},
    ["Black Mamba"] = {weight = 1.10, contact = 1, predator = 1, role = "fast contact"},
    ["Skeleton"] = {weight = 0.70, contact = 1, role = "contact"},
    ["Cloud Elemental"] = {weight = 1.30, ranged = 1, caster = 1, role = "caster"},
    ["Mouther"] = {weight = 1.00, contact = 1, disruptor = 1, role = "disruptor"},
    ["Hound"] = {weight = 1.20, contact = 1, predator = 1, role = "predator"},
    ambient = {weight = 0.75, contact = 1, role = "ambient"},
}

local function round_to(value, scale)
    scale = scale or 1
    return math.floor((tonumber(value) or 0) * scale + 0.5) / scale
end

local function map_enemy_threat(monster)
    return MAP_ENEMY_THREAT[tostring(monster or "")] or MAP_ENEMY_THREAT.ambient
end

local function pressure_label(score)
    score = tonumber(score) or 0
    if score >= 78 then
        return "overloaded"
    elseif score >= 58 then
        return "dangerous"
    elseif score >= 38 then
        return "active"
    elseif score >= 18 then
        return "readable"
    end
    return "sparse"
end

local function map_threat_profile_for_plan(plan, algorithm)
    local pins = plan.enemyPins or {}
    local stats = plan.stats or {}
    local density = tonumber(plan.density or (algorithm and algorithm.density)) or 0.30
    local counts = {}
    local weighted = 0
    local contact = 0
    local ranged = 0
    local caster = 0
    local predator = 0
    local disruptor = 0

    for _, pin in ipairs(pins) do
        local monster = tostring(pin.monster or "ambient")
        local threat = map_enemy_threat(monster)
        counts[monster] = (counts[monster] or 0) + 1
        weighted = weighted + (threat.weight or 0.75)
        contact = contact + (threat.contact or 0)
        ranged = ranged + (threat.ranged or 0)
        caster = caster + (threat.caster or 0)
        predator = predator + (threat.predator or 0)
        disruptor = disruptor + (threat.disruptor or 0)
    end

    local clustered_pairs = 0
    local stack_pairs = 0
    local tightest
    local nearest_total = 0
    local nearest_count = 0
    for idx, pin in ipairs(pins) do
        local nearest
        for other_idx, other in ipairs(pins) do
            if other_idx ~= idx then
                local dx = (pin.x or 0) - (other.x or 0)
                local dy = (pin.y or 0) - (other.y or 0)
                local dist = math.sqrt(dx * dx + dy * dy)
                if not nearest or dist < nearest then
                    nearest = dist
                end
                if other_idx > idx then
                    if dist <= 18 then
                        clustered_pairs = clustered_pairs + 1
                    end
                    if dist <= 10 then
                        stack_pairs = stack_pairs + 1
                    end
                    if not tightest or dist < tightest then
                        tightest = dist
                    end
                end
            end
        end
        if nearest then
            nearest_total = nearest_total + nearest
            nearest_count = nearest_count + 1
        end
    end

    local avg_nearest = nearest_count > 0 and nearest_total / nearest_count or 0
    local cluster_pressure = clustered_pairs * 3.5 + stack_pairs * 4.5
    if avg_nearest > 0 and avg_nearest < 22 then
        cluster_pressure = cluster_pressure + (22 - avg_nearest) * 0.55
    end
    local score = math.min(100, math.max(0, math.floor(
        weighted * 8
        + density * 18
        + cluster_pressure
        + ranged * 2.5
        + caster * 2.5
        + predator * 2.0
        + disruptor * 1.5
        + 0.5
    )))

    local mix = {}
    for monster, count in pairs(counts) do
        mix[#mix + 1] = {monster = monster, count = count, role = map_enemy_threat(monster).role}
    end
    table.sort(mix, function(a, b)
        if a.count == b.count then
            return a.monster < b.monster
        end
        return a.count > b.count
    end)

    local notes = {}
    if #pins == 0 then
        notes[#notes + 1] = "No ambient enemy pins in this preview."
    elseif #pins >= 9 then
        notes[#notes + 1] = "High ambient count; compare early contact and potion burn."
    end
    if caster > 0 then
        notes[#notes + 1] = "Caster pins can punish slow or cornered classes."
    end
    if predator > 0 then
        notes[#notes + 1] = "Fast predator/contact pins can collapse ranged spacing."
    end
    if clustered_pairs > 0 then
        notes[#notes + 1] = "Enemy pins cluster close enough to create package-like pressure."
    end
    if tostring(stats.enemyMix or "") == "" and #pins > 0 then
        notes[#notes + 1] = "Using default ambient mix for this map family."
    end

    return {
        schema = 1,
        label = pressure_label(score),
        score = score,
        enemyCount = #pins,
        weightedThreat = round_to(weighted, 10),
        density = round_to(density, 100),
        contactCount = contact,
        rangedCount = ranged,
        casterCount = caster,
        predatorCount = predator,
        disruptorCount = disruptor,
        clusteredPairs = clustered_pairs,
        stackPairs = stack_pairs,
        avgNearestDistance = round_to(avg_nearest, 10),
        tightestPairDistance = round_to(tightest or 0, 10),
        mix = mix,
        notes = notes,
    }
end

local function preview_plan_for_algorithm(algorithm, config, seed_offset)
    local width = 112
    local height = 72
    local mode = algorithm.preview_mode or "world"
    local signature = algorithm.signature or {}
    local density = tonumber(algorithm.density) or 0.30
    local seed = (tonumber(config.seed) or hash_text(config.seed)) + hash_text(algorithm.id or algorithm.label or mode) + (seed_offset or 0) * 7919
    local random = preview_rng(seed)
    local room_count = roll_signature_count(signature.rooms, random, 6 + math.floor(random() * 8))
    local region_count = roll_signature_count(signature.regions, random, 5 + math.floor(random() * 5))
    local ambient_count = roll_signature_count(signature.ambientEnemies, random, mode:find("overworld") and 7 or 3)
    local vault_count = roll_signature_count(signature.vaults, random, 1 + math.floor(random() * 3))
    local statue_count = roll_signature_count(signature.statues, random, 1 + math.floor(random() * 4))

    room_count = math.max(3, math.min(16, room_count))
    region_count = math.max(2, math.min(12, region_count))
    ambient_count = math.max(0, math.min(12, ambient_count))
    vault_count = math.max(0, math.min(6, vault_count))
    statue_count = math.max(0, math.min(8, statue_count))

    local rooms = {}
    local max_radius = math.max(7, 18 * (1 - math.min(0.55, density)))
    for idx = 1, room_count do
        local rx = 7 + math.floor(random() * max_radius)
        local ry = 5 + math.floor(random() * math.max(5, max_radius * 0.70))
        rooms[#rooms + 1] = {
            x = 7 + math.floor(random() * (width - 14)),
            y = 6 + math.floor(random() * (height - 12)),
            rx = rx,
            ry = ry,
            rotation = random() * 3.14159,
            fill = random() > 0.45 and "floor" or "floor2",
            shape = random() > 0.82 and "rect" or "ellipse",
        }
    end

    local corridors = {}
    for idx = 1, math.min(#rooms - 1, region_count + 1) do
        local a = rooms[idx]
        local b = rooms[idx + 1]
        corridors[#corridors + 1] = {
            x1 = a.x,
            y1 = a.y,
            x2 = b.x,
            y2 = b.y,
            width = 3 + math.floor(random() * 2),
            color = "floor2",
        }
    end

    local rivers = {}
    if mode == "world" or mode == "overworld" or mode:find("river") or (mode:find("overworld") and random() > 0.55) then
        local y = 9 + math.floor(random() * (height - 18))
        local points = {}
        for x = 0, width, 9 do
            y = math.max(8, math.min(height - 8, y + math.floor(random() * 11) - 5))
            points[#points + 1] = {x = x, y = y}
        end
        rivers[#rivers + 1] = {
            points = points,
            width = mode:find("river") and 4 or 3,
            color = "accent",
        }
    end

    local props = {}
    for idx = 1, vault_count do
        props[#props + 1] = {
            kind = "vault",
            x = 6 + math.floor(random() * (width - 12)),
            y = 6 + math.floor(random() * (height - 12)),
        }
    end
    for idx = 1, statue_count do
        props[#props + 1] = {
            kind = "statue",
            x = 6 + math.floor(random() * (width - 12)),
            y = 6 + math.floor(random() * (height - 12)),
        }
    end

    local enemy_names = preview_enemy_names(signature, mode)
    local enemy_pins = {}
    for idx = 1, ambient_count do
        enemy_pins[#enemy_pins + 1] = {
            monster = enemy_names[1 + math.floor(random() * #enemy_names)],
            x = 6 + math.floor(random() * (width - 12)),
            y = 6 + math.floor(random() * (height - 12)),
        }
    end

    local plan = {
        schema = 1,
        width = width,
        height = height,
        seed = tostring(seed),
        mode = mode,
        density = density,
        palette = copy_object(algorithm.palette or {}),
        rooms = rooms,
        corridors = corridors,
        rivers = rivers,
        props = props,
        enemyPins = enemy_pins,
        stats = {
            regions = region_count,
            rooms = room_count,
            ambientEnemies = ambient_count,
            enemyMix = signature.enemyMix or "",
            vaults = vault_count,
            statues = statue_count,
        },
    }
    plan.threatProfile = map_threat_profile_for_plan(plan, algorithm)
    return plan
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
    local preview_plan = preview_plan_for_algorithm(algorithm, config, seed_offset or 0)
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
        previewPlan = preview_plan,
        threatProfile = preview_plan.threatProfile,
        runConfig = preview_config,
        args = launch_args(preview_config),
        balanceArgs = balance_args(preview_config),
    }
end

local function map_threat_summary(previews)
    local total = 0
    local highest
    for _, preview in ipairs(previews or {}) do
        local threat = preview.threatProfile or (preview.previewPlan and preview.previewPlan.threatProfile)
        local score = tonumber(threat and threat.score) or 0
        total = total + score
        if not highest or score > (highest.score or 0) then
            highest = {
                id = preview.id,
                label = preview.label,
                mapProfile = preview.mapProfile,
                score = score,
                threatLabel = threat and threat.label or pressure_label(score),
            }
        end
    end
    return {
        previewCount = #(previews or {}),
        averageScore = #(previews or {}) > 0 and round_to(total / #previews, 10) or 0,
        highest = highest,
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
        threatSummary = map_threat_summary(previews),
        previews = previews,
    }
end

local function map_resource_extras(algorithm, config, seed_offset)
    local extras = {
        previewMode = algorithm.preview_mode or "world",
        density = algorithm.density or 0.30,
        phase = algorithm.phase or "",
        tags = copy_array(algorithm.tags or {}),
        signature = copy_object(algorithm.signature or {}),
        palette = algorithm.palette,
    }
    if config then
        local preview_config = copy_object(config)
        preview_config.seed = tostring((tonumber(config.seed) or 1) + (seed_offset or 0))
        local plan = preview_plan_for_algorithm(algorithm, preview_config, 0)
        extras.previewStats = copy_object(plan.stats or {})
        extras.mapThreat = plan.threatProfile
    end
    return extras
end

local function config_for_map_algorithm(config, algorithm)
    local next_config = copy_object(config)
    next_config.level = math.min(5, math.max(1, tonumber(next_config.level) or 1))
    next_config.mapDesc = algorithm.mapDesc or ""
    next_config.mapProfile = algorithm.mapProfile or ""
    next_config.mapAlgorithm = algorithm.id or map_algorithm_id_for(next_config.mapDesc, next_config.mapProfile)
    next_config.encounter = ""
    next_config.monsters = ""
    return next_config
end

local function map_probe_resources(config, start_offset)
    local resources = {}
    local offset = start_offset or 40
    local selected_algorithm = selected_map_algorithm(config) or map_algorithm_by_id(config.mapAlgorithm or "")

    if selected_algorithm then
        for idx = 1, 4 do
            local next_config = config_for_map_algorithm(config, selected_algorithm)
            local seed = (tonumber(config.seed) or 1) + idx - 1
            resources[#resources + 1] = resource_entry(
                "Selected map seed sweep",
                string.format("Seed %d", seed),
                selected_algorithm.label or selected_algorithm.id or "selected map",
                "same map algorithm across nearby seeds",
                next_config,
                idx - 1,
                map_resource_extras(selected_algorithm, next_config, idx - 1)
            )
        end
    end

    for idx, algorithm in ipairs(overworld_algorithms()) do
        local next_config = config_for_map_algorithm(config, algorithm)
        resources[#resources + 1] = resource_entry(
            "Overworld algorithm sweep",
            algorithm.label or algorithm.id,
            string.format("L%d, %s", next_config.level, algorithm.phase or "opening"),
            algorithm.description or "initial-map generation comparison",
            next_config,
            offset + idx,
            map_resource_extras(algorithm, next_config, offset + idx)
        )
    end

    for idx, map in ipairs(late_maps()) do
        local algorithm = selected_map_algorithm({
            mapDesc = map.id,
            mapProfile = "",
            mapAlgorithm = map_algorithm_id_for(map.id, ""),
        }) or {
            id = map_algorithm_id_for(map.id, ""),
            label = map.label,
            mapDesc = map.id,
            mapProfile = "",
            preview_mode = map.preview_mode,
            density = map.density,
            phase = map.phase,
            palette = map.palette,
        }
        local next_config = config_for_map_algorithm(config, algorithm)
        next_config.level = 5
        next_config.preset = next_config.preset == "none" and "late" or next_config.preset
        resources[#resources + 1] = resource_entry(
            "Late map sweep",
            map.label or map.id,
            string.format("L%d, %s loadout", next_config.level, next_config.preset),
            "late-map enemy pool and routing comparison",
            next_config,
            offset + 30 + idx,
            map_resource_extras(algorithm, next_config, offset + 30 + idx)
        )
    end

    return resources
end

local function map_matrix_payload(config)
    local resources = map_probe_resources(config, 40)
    return {
        schema = 1,
        config = config,
        generation = map_generation_payload(config),
        count = #resources,
        resources = resources,
    }
end

local function enemy_probe_resources(config, start_offset)
    local resources = {}
    local seed_offset = start_offset or 90
    for idx, choice in ipairs(all_enemy_choices()) do
        local next_config = copy_object(config)
        next_config.level = math.min(5, math.max(1, tonumber(choice.suggestedLevel) or config.level))
        if config.level >= 5 then
            next_config.level = 5
        end
        next_config.preset = next_config.preset == "none" and (next_config.level >= 5 and "late" or "none") or next_config.preset
        next_config.mapDesc = ""
        next_config.mapProfile = ""
        next_config.mapAlgorithm = ""
        next_config.encounter = ""
        next_config.enemyGroup = choice.group or ""
        next_config.enemyPack = ""
        next_config.monsters = choice.name or ""
        local eval_config = copy_object(next_config)
        eval_config.seed = tostring((tonumber(config.seed) or 1) + seed_offset + idx)
        resources[#resources + 1] = resource_entry(
            "Enemy probe",
            choice.name or "Enemy",
            string.format("L%d, %s arena", next_config.level, choice.role or "single enemy"),
            "single-enemy balance and class-counter probe",
            next_config,
            seed_offset + idx,
            {
                enemy = choice,
                monsterEvalArgs = monster_eval_args(eval_config, {
                    monsters = choice.name or "",
                    level = next_config.level,
                }),
            }
        )
    end
    for idx, pack in ipairs(Catalog.enemy_packs or {}) do
        local next_config = copy_object(config)
        next_config.level = pack.phase == "opening" and math.min(3, next_config.level) or math.min(5, math.max(3, next_config.level))
        next_config.preset = next_config.preset == "none" and (next_config.level >= 5 and "late" or "none") or next_config.preset
        next_config.mapDesc = ""
        next_config.mapProfile = ""
        next_config.mapAlgorithm = ""
        next_config.encounter = ""
        next_config.monsters = join_names(pack.monsters or {})
        next_config.enemyPack = pack.id or ""
        local eval_config = copy_object(next_config)
        eval_config.seed = tostring((tonumber(config.seed) or 1) + seed_offset + 40 + idx)
        resources[#resources + 1] = resource_entry(
            "Enemy pack",
            pack.label or pack.id,
            string.format("L%d, %s", next_config.level, pack.phase or "arena"),
            pack.goal or "enemy-pack interaction probe",
            next_config,
            seed_offset + 40 + idx,
            {
                enemyPack = pack.id,
                monsterEvalArgs = monster_eval_args(eval_config, {
                    pack = pack.pack,
                    level = next_config.level,
                }),
            }
        )
    end
    return resources
end

local function append_resources(dest, rows)
    for _, row in ipairs(rows or {}) do
        dest[#dest + 1] = row
    end
end

local function class_sweep_args(config)
    local next_config = copy_object(config)
    local names = {}
    for _, class in ipairs(Catalog.classes or {}) do
        names[#names + 1] = class.id
    end
    next_config.className = table.concat(names, ",")
    return balance_args(next_config)
end

local function analysis_plan_payload(config)
    local enemy_options = enemy_options_payload(config)
    local enemy_matrix = enemy_probe_resources(config, 130)
    return {
        schema = 1,
        config = config,
        goal = analysis_goal_lookup()[config.goal] or (Catalog.analysis_goals and Catalog.analysis_goals[1]),
        commands = {
            launch = launch_args(config),
            balance = balance_args(config),
            classSweep = class_sweep_args(config),
            monsterEval = enemy_options.monsterEvalArgs,
        },
        enemyOptions = enemy_options,
        enemyMatrix = {
            count = #enemy_matrix,
            resources = enemy_matrix,
        },
        mapGeneration = map_generation_payload(config),
    }
end

local function pressure_labels_for_frame(rhythms, frame)
    local labels = {}
    for _, rhythm in ipairs(rhythms or {}) do
        if rhythm.id ~= "escort" then
            local interval = math.max(1, tonumber(rhythm.interval) or 1)
            local offset = tonumber(rhythm.offset) or 0
            local duration = math.max(0, tonumber(rhythm.duration) or 0)
            if duration > 0 and frame >= offset then
                local phase = (frame - offset) % interval
                if phase < duration then
                    labels[#labels + 1] = rhythm.label or rhythm.id
                end
            end
        end
    end
    return labels
end

local function pressure_diagnosis_for(rhythms, preview)
    local ids = {}
    for _, rhythm in ipairs(rhythms or {}) do
        ids[rhythm.id or ""] = true
    end

    local peak = tonumber(preview.peakStack) or 0
    local overlap_rate = tonumber(preview.overlapRate) or 0
    local rhythm_count = #(rhythms or {})
    local label
    local severity
    local notes = {}

    if rhythm_count == 0 then
        label = "No package pressure"
        severity = "none"
        notes[#notes + 1] = "No package rhythms are selected."
    elseif peak >= 4 or overlap_rate >= 0.45 then
        label = "Overloaded"
        severity = "danger"
        notes[#notes + 1] = "Many windows overlap; use this as a stress target, not a default baseline."
    elseif peak >= 3 or overlap_rate >= 0.18 then
        label = "Stacked micro"
        severity = "strong"
        notes[#notes + 1] = "Multiple package decisions overlap often enough to test real micro."
    elseif peak >= 2 and overlap_rate >= 0.06 then
        label = "Readable overlap"
        severity = "ok"
        notes[#notes + 1] = "Package has overlap, but the stack should remain readable."
    else
        label = "Serial timings"
        severity = "light"
        notes[#notes + 1] = "Rhythms rarely overlap; the fight may feel like isolated mechanics."
    end

    if not ids.rescue then
        notes[#notes + 1] = "No rescue lane; this will not test pull/body-block saves."
    end
    if not ids.summon then
        notes[#notes + 1] = "No add wave; this will not test target-swap cleanup pressure."
    end
    if ids.rescue and ids.summon and peak < 2 then
        notes[#notes + 1] = "Rescue and add pressure do not overlap in the preview horizon."
    end

    return {
        label = label,
        severity = severity,
        notes = notes,
    }
end

local function pressure_preview_for(rhythms, horizon_frames)
    rhythms = rhythms or {}
    local horizon = math.max(120, math.floor(tonumber(horizon_frames) or 720))
    local peak_stack = 0
    local first_overlap
    local first_peak
    local overlap_frames = 0
    local windows = {}
    local active_window

    for frame = 0, horizon - 1 do
        local labels = pressure_labels_for_frame(rhythms, frame)
        local stack = #labels
        if stack > peak_stack then
            peak_stack = stack
            first_peak = frame
        end
        if stack >= 2 then
            overlap_frames = overlap_frames + 1
            if not first_overlap then
                first_overlap = frame
            end
            if not active_window then
                active_window = {
                    startFrame = frame,
                    endFrame = frame + 1,
                    peakStack = stack,
                    labels = table.concat(labels, "+"),
                }
            else
                active_window.endFrame = frame + 1
                if stack > (active_window.peakStack or 0) then
                    active_window.peakStack = stack
                    active_window.labels = table.concat(labels, "+")
                end
            end
        elseif active_window then
            active_window.durationFrames = (active_window.endFrame or frame) - (active_window.startFrame or frame)
            windows[#windows + 1] = active_window
            active_window = nil
        end
    end
    if active_window then
        active_window.durationFrames = (active_window.endFrame or horizon) - (active_window.startFrame or horizon)
        windows[#windows + 1] = active_window
    end

    local limited_windows = {}
    for idx, window in ipairs(windows) do
        if idx > 8 then
            break
        end
        limited_windows[#limited_windows + 1] = window
    end

    local preview = {
        horizonFrames = horizon,
        peakStack = peak_stack,
        firstPeakFrame = first_peak,
        overlapFrames = overlap_frames,
        overlapRate = safe_rate(overlap_frames, horizon),
        firstOverlapFrame = first_overlap,
        windows = limited_windows,
    }
    preview.diagnosis = pressure_diagnosis_for(rhythms, preview)
    return preview
end

local function package_summary_from_rhythms(rhythms)
    if #(rhythms or {}) == 0 then
        return nil
    end
    local pressure = pressure_preview_for(rhythms)
    local rhythm_ids = {}
    local first_threat
    local fastest_loop
    local has_rescue = false
    local rescue_clutch_window
    local rescue_perfect_window
    local rescue_pull_frames
    local has_summons = false
    local summon_cap
    local has_sweep = false
    local hits_escorts = false
    for _, rhythm in ipairs(rhythms or {}) do
        rhythm_ids[#rhythm_ids + 1] = rhythm.id
        local offset = tonumber(rhythm.offset) or 0
        if first_threat == nil or offset < first_threat then
            first_threat = offset
        end
        local interval = tonumber(rhythm.interval)
        if interval and interval > 0 and (fastest_loop == nil or interval < fastest_loop) then
            fastest_loop = interval
        end
        if rhythm.id == "rescue" then
            has_rescue = true
            rescue_clutch_window = rhythm.clutchWindow or rhythm.lateWindow
            rescue_perfect_window = rhythm.perfectWindow
            rescue_pull_frames = rhythm.pullFrames
        elseif rhythm.id == "summon" then
            has_summons = true
            summon_cap = rhythm.maxAlive
        elseif rhythm.id == "sweep" then
            has_sweep = true
        end
        if rhythm.affectsEscorts then
            hits_escorts = true
        end
    end
    return {
        rhythmCount = #rhythm_ids,
        rhythmIds = rhythm_ids,
        firstThreatFrame = first_threat or 0,
        fastestLoopFrames = fastest_loop or 0,
        hasRescue = has_rescue,
        rescueClutchWindow = rescue_clutch_window,
        rescuePerfectWindow = rescue_perfect_window,
        rescuePullFrames = rescue_pull_frames,
        hasSummons = has_summons,
        summonCap = summon_cap,
        hasSweep = has_sweep,
        hitsEscorts = hits_escorts,
        predictedPeakStack = pressure.peakStack,
        predictedOverlapFrames = pressure.overlapFrames,
        predictedOverlapRate = pressure.overlapRate,
        predictedFirstOverlapFrame = pressure.firstOverlapFrame,
        predictedPressureLabel = pressure.diagnosis and pressure.diagnosis.label or "",
        predictedPressureSeverity = pressure.diagnosis and pressure.diagnosis.severity or "",
    }
end

local function metric_row(key, label, note)
    return {
        key = key,
        label = label,
        note = note,
    }
end

local function append_metric(rows, key, label, note)
    rows[#rows + 1] = metric_row(key, label, note)
end

local function threat_target_for(rhythm)
    if rhythm.id == "rescue" then
        return "ally line"
    end
    if rhythm.id == "escort" then
        return rhythm.monster or "ally"
    end
    if rhythm.id == "summon" then
        return "arena"
    end
    if rhythm.id == "sweep" then
        return rhythm.affectsEscorts and "route + ally" or "route"
    end
    if rhythm.affectsEscorts then
        return "player + ally"
    end
    return "player"
end

local function threat_read_for(rhythm)
    if rhythm.id == "orbit" then
        return "orbit pathing"
    elseif rhythm.id == "pulse" then
        return "windup escape"
    elseif rhythm.id == "sweep" then
        return "crossing lane"
    elseif rhythm.id == "summon" then
        return "add cleanup"
    elseif rhythm.id == "escort" then
        return "ally spacing"
    elseif rhythm.id == "rescue" then
        return "pull/body-block"
    end
    return "fight rhythm"
end

local function micro_skill_buffer(control_model, rhythm)
    local reaction_tax = math.max(0, (tonumber(control_model.decisionInterval) or 1) - 1)
    local error_chance = math.max(
        tonumber(control_model.spellSkipChance) or 0,
        tonumber(control_model.itemSkipChance) or 0,
        tonumber(control_model.weaponSkipChance) or 0,
        tonumber(control_model.movementNoiseChance) or 0
    )
    local window = rhythm.clutchWindow or rhythm.lateWindow or rhythm.active or rhythm.duration or 0
    return {
        reactionTaxFrames = reaction_tax,
        dropRiskPct = math.floor(error_chance * 100 + 0.5),
        windowFrames = window,
        effectiveWindowFrames = math.max(0, (tonumber(window) or 0) - reaction_tax),
    }
end

local function micro_action_for(rhythm, control_model)
    local class_name = control_model.className or ""
    local class_range = control_model.classRange or ""
    local target_distance = tonumber(control_model.targetDistance) or 0
    local close_class = class_range ~= "far"

    if rhythm.id == "rescue" then
        if class_name == "Necromancer" then
            return {
                priority = 1,
                action = "Hold Pain range until the line warns, then body-block late and let regen offset the block hit.",
                posture = "regen attrition",
                failure = "ally hit before regen-adjusted damage says the trade was worth it",
                telemetry = {"avg_package_rescue_clutch_rate", "avg_package_rescue_pull_rate", "avg_necromancer_regen_offset_pct"},
            }
        elseif close_class then
            return {
                priority = 1,
                action = "Stay close enough to threaten the primary, then cut into the rescue line during the clutch window.",
                posture = "close intercept",
                failure = "primary dies but ally saves collapse under real reaction delay",
                telemetry = {"avg_package_rescue_save_rate", "avg_package_rescue_clutch_rate", "avg_rescue_intercept_frames"},
            }
        end
        return {
            priority = 1,
            action = "Hold keep-away range, then deliberately break spacing to body-block the line before returning to range.",
            posture = "ranged rescue commit",
            failure = "caster wins only when the rescue lane is ignored or the enemy cannot punish the commit",
            telemetry = {"avg_package_rescue_save_rate", "avg_ranged_keepaway_frames", "avg_ranged_close_enemy_frames"},
        }
    elseif rhythm.id == "summon" then
        if close_class then
            return {
                priority = 2,
                action = "Swap off the primary during the portal warning, clear the add before the cap fills, then re-enter contact.",
                posture = "target swap",
                failure = "primary uptime stays perfect while add cap is saturated",
                telemetry = {"avg_max_visible_enemies", "kills_per_1000_combat_frames"},
            }
        end
        return {
            priority = 2,
            action = "Kite the primary around the portal, tag the spawn early, and reset to keep-away range before the next timed threat.",
            posture = "kite cleanup",
            failure = "adds never force lost casting distance",
            telemetry = {"avg_max_visible_enemies", "kills_per_1000_combat_frames", "avg_ranged_keepaway_frames"},
        }
    elseif rhythm.id == "pulse" then
        if close_class then
            return {
                priority = 3,
                action = "Drop contact during windup, drag the circle away from ally lanes, then re-engage after active frames.",
                posture = "melee reset",
                failure = "melee can facetank the pulse without giving up pressure",
                telemetry = {"avg_min_hp_pct", "combat_frame_rate"},
            }
        end
        return {
            priority = 3,
            action = "Keep casting until windup starts, sidestep out, and preserve roughly " .. tostring(target_distance) .. "px spacing.",
            posture = "caster sidestep",
            failure = "AOE never collapses keep-away uptime",
            telemetry = {"avg_ranged_keepaway_frames", "avg_ranged_close_enemy_frames"},
        }
    elseif rhythm.id == "sweep" then
        if close_class then
            return {
                priority = 3,
                action = "Hold contact only until the lane warns, then cross after the sweep passes or commit through with a mobility button.",
                posture = "lane crossing",
                failure = "melee ignores the sweep and keeps perfect boss uptime",
                telemetry = {"avg_min_hp_pct", "combat_frame_rate"},
            }
        end
        return {
            priority = 3,
            action = "Treat the sweep as a hard boundary, preserve casting distance on the safe side, and cross only during recovery.",
            posture = "ranged lane reset",
            failure = "lane pressure never forces a lost cast angle or distance reset",
            telemetry = {"avg_ranged_keepaway_frames", "avg_ranged_close_enemy_frames"},
        }
    elseif rhythm.id == "orbit" then
        if close_class then
            return {
                priority = 4,
                action = "Fight inside the orbit lanes only when the primary is vulnerable; otherwise stutter-step around the rotating path.",
                posture = "close lane read",
                failure = "orb damage is ignorable contact tax",
                telemetry = {"avg_min_hp_pct", "combat_frame_rate"},
            }
        end
        return {
            priority = 4,
            action = "Use the orbit lane as the spacing boundary and punish when the lane rotates off the casting angle.",
            posture = "ranged lane read",
            failure = "orbs do not constrain caster angle or distance",
            telemetry = {"avg_ranged_keepaway_frames", "avg_min_hp_pct"},
        }
    elseif rhythm.id == "escort" then
        return {
            priority = 5,
            action = "Keep the ally inside reachable space without letting it become the primary damage sponge.",
            posture = "ally tether",
            failure = "package clears while ally HP is irrelevant",
            telemetry = {"avg_package_escort_hp_pct"},
        }
    end

    return {
        priority = 9,
        action = "Read the package timing, trade only inside the class posture, and check whether the bot profile is carrying the clear.",
        posture = control_model.positioning or class_range or "class posture",
        failure = "clear rate hides the actual micro requirement",
        telemetry = {"win_rate", "avg_min_hp_pct"},
    }
end

local function micro_plan_for(rhythms, control_model)
    local rows = {}
    for index, rhythm in ipairs(rhythms or {}) do
        local action = micro_action_for(rhythm, control_model or {})
        local buffer = micro_skill_buffer(control_model or {}, rhythm)
        rows[#rows + 1] = {
            id = rhythm.id,
            label = rhythm.label or rhythm.id,
            priority = action.priority or 9,
            order = index,
            firstFrame = rhythm.offset or 0,
            loopFrames = rhythm.interval or 0,
            read = threat_read_for(rhythm),
            target = threat_target_for(rhythm),
            action = action.action,
            posture = action.posture,
            failure = action.failure,
            telemetry = action.telemetry or {},
            reactionTaxFrames = buffer.reactionTaxFrames,
            dropRiskPct = buffer.dropRiskPct,
            windowFrames = buffer.windowFrames,
            effectiveWindowFrames = buffer.effectiveWindowFrames,
            perfectWindowFrames = rhythm.perfectWindow,
        }
    end
    table.sort(rows, function(a, b)
        if a.priority == b.priority then
            return (a.order or 0) < (b.order or 0)
        end
        return (a.priority or 9) < (b.priority or 9)
    end)
    return rows
end

local function threat_windows_for(rhythms)
    local rows = {}
    for _, rhythm in ipairs(rhythms or {}) do
        rows[#rows + 1] = {
            id = rhythm.id,
            label = rhythm.label or rhythm.id,
            firstFrame = rhythm.offset or 0,
            loopFrames = rhythm.interval or 0,
            windowFrames = rhythm.duration or 0,
            windupFrames = rhythm.windup,
            activeFrames = rhythm.active or rhythm.clutchWindow or rhythm.lateWindow,
            clutchFrames = rhythm.clutchWindow or rhythm.lateWindow,
            perfectFrames = rhythm.perfectWindow,
            pullFrames = rhythm.pullFrames,
            damage = rhythm.damage or rhythm.allyDamage,
            blockDamage = rhythm.blockDamage,
            count = rhythm.count,
            maxAlive = rhythm.maxAlive,
            target = threat_target_for(rhythm),
            read = threat_read_for(rhythm),
        }
    end
    return rows
end

local function class_action_guide(class_name)
    return (Catalog.action_guides or {})[class_name or ""] or {}
end

local function action_row(kind, slot, name, label, role, level, spec)
    spec = spec or {}
    return {
        kind = kind,
        slot = slot,
        input = spec.input or slot,
        name = name or label or slot,
        label = label or name or slot,
        role = role or "",
        level = level,
        purpose = spec.purpose or "",
        timing = spec.timing or "",
        tags = copy_array(spec.tags or {}),
        telemetry = copy_array(spec.telemetry or {}),
        beats = copy_object(spec.beats or {}),
    }
end

local function primary_action_row_for(config, guide)
    local class_build = class_build_for(config.className)
    local spec = guide.primary or {}
    local label = spec.label or class_build.starting_weapon or "Weapon"
    return action_row("primary", "weapon", class_build.starting_weapon or "Weapon", label, "primary", 1, spec)
end

local function spell_action_rows_for(class_name, known_spells)
    local guide = class_action_guide(class_name)
    local rows = {}
    for idx, spell in ipairs(known_spells or {}) do
        local spec = guide.spells and guide.spells[spell.name or ""] or {}
        local row = action_row("spell", "spell" .. tostring(idx), spell.name, spell.name, spell.role, spell.level, spec)
        row.input = spec.input or ("Spell " .. tostring(idx))
        rows[#rows + 1] = row
    end
    return rows
end

local function loadout_has_item(loadout, needle)
    needle = tostring(needle or ""):lower()
    for _, spec in ipairs(loadout and loadout.items or {}) do
        local item = tostring(spec.item or ""):lower()
        if item:find(needle, 1, true) then
            return true
        end
    end
    return false
end

local function consumable_action_rows(loadout, package_summary)
    local rows = {}
    if loadout_has_item(loadout, "health potion") then
        rows[#rows + 1] = action_row("item", "item", "Health Potion", "Health Potion", "survival", nil, {
            input = "Item",
            purpose = "Emergency survival buffer; do not let potion use hide enemy damage or failed micro.",
            timing = package_summary and package_summary.hasRescue
                and "After a rescue block if the next timed threat is not already firing."
                or "After a damage window, before re-entering contact.",
            tags = {"survival", "consumable"},
            telemetry = {"avg_min_hp_pct", "death_rate"},
        })
    end
    if loadout_has_item(loadout, "mana potion") then
        rows[#rows + 1] = action_row("item", "item", "Mana Potion", "Mana Potion", "casting", nil, {
            input = "Item",
            purpose = "Casting uptime buffer for long package runs.",
            timing = "Use during low-overlap recovery windows, not during rescue or AOE movement.",
            tags = {"casting", "consumable"},
            telemetry = {"damage_dealt_per_1000_combat_frames"},
        })
    end
    return rows
end

local function beat_action_calls(beat, actions)
    local calls = {}
    for _, action in ipairs(actions or {}) do
        local use = action.beats and action.beats[beat.id or ""]
        if use and use ~= "" then
            calls[#calls + 1] = {
                slot = action.slot,
                input = action.input,
                name = action.name,
                label = action.label,
                kind = action.kind,
                use = use,
                tags = copy_array(action.tags or {}),
                telemetry = copy_array(action.telemetry or {}),
            }
        end
    end
    if #calls == 0 and actions and actions[1] then
        local fallback = actions[1]
        calls[#calls + 1] = {
            slot = fallback.slot,
            input = fallback.input,
            name = fallback.name,
            label = fallback.label,
            kind = fallback.kind,
            use = fallback.timing ~= "" and fallback.timing or fallback.purpose,
            tags = copy_array(fallback.tags or {}),
            telemetry = copy_array(fallback.telemetry or {}),
        }
    end
    return calls
end

local function action_guide_warnings(config, class, package_summary, actions, loadout_readiness)
    local warnings = {}
    if #(actions or {}) <= 1 then
        warnings[#warnings + 1] = "No known spell buttons at this level; this setup mostly tests movement, weapon timing, and gear."
    end
    if package_summary and package_summary.hasRescue and class.range == "far" then
        warnings[#warnings + 1] = "Rescue timings intentionally force ranged classes to break keep-away spacing."
    end
    if config.className == "Necromancer" then
        warnings[#warnings + 1] = "Necromancer reads must use regen-adjusted damage, not raw damage taken."
    end
    for _, gap in ipairs(loadout_readiness and loadout_readiness.gaps or {}) do
        warnings[#warnings + 1] = gap
        if #warnings >= 4 then
            break
        end
    end
    return warnings
end

action_guide_payload = function(config)
    local class = class_lookup()[config.className] or {}
    local guide = class_action_guide(config.className)
    local known_spells, upcoming_spells = spell_rows_for_level(config.className, config.level)
    local loadout = loadout_for_config(config)
    local selected_intensity_row = selected_intensity(config)
    local selected_encounter = encounter_lookup()[config.encounter or ""]
    local encounter = encounter_with_components(selected_encounter, config.packageComponents)
    local rhythms = encounter_rhythms(encounter, selected_intensity_row)
    local package_summary = package_summary_from_rhythms(rhythms)
    local control_model = control_model_for(config)
    local micro_plan = micro_plan_for(rhythms, control_model)
    local pressure_preview = pressure_preview_for(rhythms)
    local loadout_readiness = loadout_readiness_for_config(config, loadout, known_spells)

    local actions = {primary_action_row_for(config, guide)}
    for _, row in ipairs(spell_action_rows_for(config.className, known_spells)) do
        actions[#actions + 1] = row
    end
    for _, row in ipairs(consumable_action_rows(loadout, package_summary)) do
        actions[#actions + 1] = row
    end

    local beat_plan = {}
    for _, beat in ipairs(micro_plan or {}) do
        beat_plan[#beat_plan + 1] = {
            id = beat.id,
            label = beat.label,
            priority = beat.priority,
            firstFrame = beat.firstFrame,
            loopFrames = beat.loopFrames,
            read = beat.read,
            target = beat.target,
            posture = beat.posture,
            action = beat.action,
            failure = beat.failure,
            reactionTaxFrames = beat.reactionTaxFrames,
            effectiveWindowFrames = beat.effectiveWindowFrames,
            perfectWindowFrames = beat.perfectWindowFrames,
            calls = beat_action_calls(beat, actions),
            telemetry = copy_array(beat.telemetry or {}),
        }
    end

    return {
        schema = 1,
        config = config,
        stance = {
            className = config.className,
            label = actions[1] and actions[1].label or class.role or config.className,
            role = class.role or "",
            range = class.range or "",
            positioning = class.positioning or "",
            targetDistance = control_model.targetDistance or 0,
            notes = class.control_notes or class.balance_focus or "",
            botProfile = control_model.botProfile,
        },
        actions = actions,
        beatPlan = beat_plan,
        knownSpells = known_spells,
        upcomingSpells = upcoming_spells,
        loadoutReadiness = loadout_readiness,
        packageSummary = package_summary,
        pressurePreview = pressure_preview,
        threats = threat_windows_for(rhythms),
        telemetry = telemetry_watch_for(config, package_summary),
        warnings = action_guide_warnings(config, class, package_summary, actions, loadout_readiness),
    }
end

local function primary_enemy_for(config, encounter)
    local custom = split_csv(config.monsters)
    if #custom > 0 then
        return custom[1], custom
    end
    if encounter and encounter.package and encounter.package.primary then
        return encounter.package.primary, copy_array(encounter.monsters or {encounter.package.primary})
    end
    if encounter and encounter.monsters and #(encounter.monsters or {}) > 0 then
        return encounter.monsters[1], copy_array(encounter.monsters)
    end
    return "", {}
end

local function package_allies_for(encounter)
    local allies = {}
    local escort = encounter and encounter.package and encounter.package.escort
    if escort then
        allies[#allies + 1] = {
            monster = escort.monster or "ally",
            count = escort.count or 1,
            hp = escort.hp,
        }
    end
    return allies
end

local function fight_objective_for(config, selected, encounter, package_summary)
    local primary, monsters = primary_enemy_for(config, encounter)
    local allies = package_allies_for(encounter)
    if encounter and (encounter.id or "") ~= "" then
        local label
        if package_summary and package_summary.hasRescue then
            label = string.format("Defeat %s while keeping the ally alive", primary ~= "" and primary or "the package")
        elseif encounter.package then
            label = string.format("Defeat %s through the package rhythms", primary ~= "" and primary or "the package")
        elseif encounter.kind and encounter.kind:find("baseline") then
            label = string.format("Baseline duel against %s", primary ~= "" and primary or encounter.label or encounter.id)
        else
            label = string.format("Defeat %s", primary ~= "" and primary or encounter.label or encounter.id)
        end
        return {
            type = encounter.package and "package" or "duel",
            label = label,
            primary = primary,
            monsters = monsters,
            allies = allies,
            arena = copy_object(encounter.arena or {}),
            clear = package_summary and package_summary.hasRescue and "primary down, ally alive" or "hostiles down",
            failure = package_summary and package_summary.hasRescue and "ally down or player death" or "player death",
        }
    end

    local map_label = selected.map and (selected.map.label or selected.map.id) or "Campaign World"
    local algorithm_label = selected.mapAlgorithm and (selected.mapAlgorithm.label or selected.mapAlgorithm.id) or map_label
    return {
        type = "map",
        label = string.format("Run %s", algorithm_label or map_label),
        primary = "",
        monsters = monsters,
        allies = {},
        map = selected.map,
        mapAlgorithm = selected.mapAlgorithm,
        clear = "map survival and pacing",
        failure = "player death",
    }
end

local ENEMY_MATCHUP_TAGS = {
    ["Red Dragon"] = {"red_resistant", "fire_projectile", "boss"},
    ["Purple Dragon"] = {"red_resistant", "fire_projectile", "summon_pressure", "boss"},
    ["Fire Bat"] = {"red_resistant", "fire_projectile", "add"},
    ["Imp"] = {"red_resistant", "fire_projectile", "add"},
    ["Firelord"] = {"red_resistant", "contact"},
    ["Hell Warrior"] = {"red_resistant", "contact"},
    ["Ogre Mage"] = {"red_resistant", "fire_projectile", "caster"},
    ["Cloud Elemental"] = {"white_resistant", "ranged"},
    ["Storm Elemental"] = {"white_resistant", "ranged"},
    ["Hell Storm"] = {"white_resistant", "ranged"},
    ["Crypt Keeper"] = {"black_resistant", "aura", "summon_pressure"},
    ["Mummoner"] = {"black_resistant", "summon_pressure"},
    ["Executioner"] = {"black_resistant", "contact"},
    ["Mana Sapper"] = {"black_resistant", "support"},
}

local function matchup_tags_for_monsters(monsters, encounter)
    local tags = {}
    local function add(tag)
        tags[tag] = (tags[tag] or 0) + 1
    end
    for _, monster in ipairs(monsters or {}) do
        for _, tag in ipairs(ENEMY_MATCHUP_TAGS[monster] or {}) do
            add(tag)
        end
    end
    local package = encounter and encounter.package
    if package and package.summon then
        for _, monster in ipairs(package.summon.monsters or {}) do
            for _, tag in ipairs(ENEMY_MATCHUP_TAGS[monster] or {}) do
                add(tag)
            end
        end
    end
    return tags
end

local function matchup_read_for(config, encounter)
    local _, monsters = primary_enemy_for(config, encounter)
    local tags = matchup_tags_for_monsters(monsters, encounter)
    local risks = {}
    local strengths = {}
    local notes = {}
    local telemetry = {"damage_dealt_per_1000_combat_frames", "avg_objective_damage_total", "avg_min_hp_pct"}
    local severity = "neutral"
    local label = "baseline matchup"

    local function risk(text)
        risks[#risks + 1] = text
        severity = severity == "high" and "high" or "warning"
    end
    local function high_risk(text)
        risks[#risks + 1] = text
        severity = "high"
    end
    local function strength(text)
        strengths[#strengths + 1] = text
    end

    if config.className == "Pyrocaster" then
        if (tags.red_resistant or 0) > 0 then
            high_risk("red/fire resistance can suppress Pyrocaster conversion")
            label = "fire-resist check"
            telemetry[#telemetry + 1] = "avg_pyrocaster_ring_combat_frames"
        end
        if (tags.fire_projectile or 0) > 0 then
            risk("enemy fire/projectile pressure tests ranged spacing while casting")
        end
    elseif config.className == "Stormcaller" then
        if (tags.red_resistant or 0) > 0 then
            strength("white damage avoids the red/fire resistance check")
            label = "caster counter-check"
        end
        if (tags.white_resistant or 0) > 0 then
            high_risk("white-resistant enemies can flatten Stormcaller overperformance")
            label = "white mirror check"
        end
    elseif config.className == "Necromancer" then
        if (tags.black_resistant or 0) > 0 then
            high_risk("black-resistant enemies can make gross regen trades misleading")
            label = "black-resist check"
        end
        telemetry[#telemetry + 1] = "avg_necromancer_regen_offset_pct"
    end

    if (tags.summon_pressure or 0) > 0 then
        notes[#notes + 1] = "Summon pressure should be read with add cleanup and visible-enemy telemetry."
        telemetry[#telemetry + 1] = "kills_per_1000_combat_frames"
    end
    if (tags.contact or 0) > 0 then
        notes[#notes + 1] = "Contact pressure can hide caster damage problems behind survival failures."
    end
    if #risks == 0 and #strengths == 0 then
        notes[#notes + 1] = "No explicit elemental counter read for this class/enemy pairing."
    end

    return {
        schema = 1,
        label = label,
        severity = severity,
        monsters = monsters,
        tags = tags,
        risks = risks,
        strengths = strengths,
        notes = notes,
        telemetry = telemetry,
    }
end

telemetry_watch_for = function(config, package_summary)
    local rows = {}
    append_metric(rows, "win_rate", "Win rate", "overall clear check")
    append_metric(rows, "death_rate", "Death rate", "survival floor")
    append_metric(rows, "avg_frames", "Run frames", "pacing")
    append_metric(rows, "combat_frame_rate", "Combat rate", "enemy contact exposure")
    append_metric(rows, "avg_min_hp_pct", "Min HP", "near-death pressure")

    if package_summary then
        append_metric(rows, "avg_package_frames", "Package time", "package clear pacing")
        append_metric(rows, "avg_package_pressure_max_count", "Peak stack", "simultaneous package demands")
        append_metric(rows, "avg_package_pressure_overlap_rate", "Overlap rate", "share of frames with stacked pressure")
        append_metric(rows, "avg_package_micro_score", "Micro score", "live drill score from dodges, saves, and hits")
        append_metric(rows, "avg_package_micro_best_streak", "Best streak", "longest clean micro streak")
        append_metric(rows, "avg_package_micro_dodges", "Threaded avoids", "targeted or near-miss hazard avoids")
        append_metric(rows, "avg_package_micro_hits", "Mechanic hits", "avoidable hazard failures")
        append_metric(rows, "avg_package_micro_punish_total", "Punish", "boss recovery from avoidable failures")
        append_metric(rows, "avg_package_micro_punish_break_total", "Break", "clutch/perfect save pressure refund")
        if package_summary.hasSweep then
            append_metric(rows, "avg_package_sweep_active_count", "Sweep active", "crossing-lane pressure uptime")
            append_metric(rows, "avg_package_sweep_warning_count", "Sweep warns", "crossing-lane read windows")
        end
        if package_summary.hasSummons then
            append_metric(rows, "avg_max_visible_enemies", "Enemy cap", "add saturation")
            append_metric(rows, "kills_per_1000_combat_frames", "Kill tempo", "add cleanup pace")
        end
        if package_summary.hasRescue then
            append_metric(rows, "avg_package_escort_hp_pct", "Ally HP", "escort survival margin")
            append_metric(rows, "avg_package_rescue_save_rate", "Save rate", "rescue reliability")
            append_metric(rows, "avg_package_rescue_clutch_rate", "Clutch rate", "late-window saves")
            append_metric(rows, "avg_package_rescue_perfect_rate", "Perfect rate", "few-frame saves")
            append_metric(rows, "avg_package_rescue_early_rate", "Early saves", "saves started outside clutch window")
            append_metric(rows, "avg_package_rescue_timing_score", "Timing score", "clutch plus perfect timing quality")
            append_metric(rows, "avg_package_rescue_pull_rate", "Pull rate", "taunt/body-block interaction")
            append_metric(rows, "avg_package_rescue_pull_uptime_rate", "Taunt uptime", "forced target-control share")
            append_metric(rows, "avg_package_rescue_counter_windows", "Counters", "clutch/perfect saves that opened the boss")
            append_metric(rows, "avg_package_rescue_counter_damage_total", "Counter dmg", "boss damage from timed rescue openings")
            append_metric(rows, "avg_package_rescue_counter_stagger_frames", "Stagger", "boss stun frames from timed rescue openings")
            append_metric(rows, "avg_package_rescue_misses", "Misses", "failed rescue windows")
            append_metric(rows, "avg_package_rescue_fatal_misses", "Fatal misses", "missed saves that ended the ally")
            append_metric(rows, "avg_package_rescue_survived_misses", "Buffered misses", "missed saves that left another rescue read")
            append_metric(rows, "avg_rescue_intercept_frames", "Intercept", "body-block movement")
            append_metric(rows, "avg_rescue_hold_frames", "Hold", "frames spent holding the line")
            append_metric(rows, "avg_rescue_late_reaction_frames", "Late reaction", "human skill buffer")
        end
    end

    local class = class_lookup()[config.className] or {}
    if class.range == "far" then
        append_metric(rows, "avg_ranged_keepaway_frames", "Keep-away", "ranged spacing uptime")
        append_metric(rows, "avg_ranged_close_enemy_frames", "Forced close", "enemy access to caster")
        append_metric(rows, "avg_ranged_close_gragh_frames", "Gragh close", "boss contact pressure")
    end
    if config.className == "Necromancer" then
        append_metric(rows, "avg_necromancer_regen_offset_pct", "Regen offset", "gross damage correction")
        append_metric(rows, "avg_regen_adjusted_damage_taken_total", "Adjusted damage", "damage after regen")
        append_metric(rows, "avg_necromancer_regen_recovered_total", "Regen total", "sustain volume")
        append_metric(rows, "avg_necromancer_regen_frames", "Regen frames", "regen uptime")
        append_metric(rows, "avg_necromancer_close_frames", "Close frames", "Pain-range commitment")
    elseif config.className == "Dicebinder" then
        append_metric(rows, "avg_dice_rolls_total", "D6 rolls", "skill dice windows per run")
        append_metric(rows, "avg_dice_dash_effective_refreshes", "Dash refresh", "cooldown refunds that mattered")
        append_metric(rows, "avg_dice_power_effective_refreshes", "Power refresh", "Power Strike refunds that mattered")
        append_metric(rows, "avg_dice_jackpot_rolls", "Jackpots", "roll-six swing windows")
        append_metric(rows, "avg_dice_buff_active_frames", "Buff frames", "skill dice uptime")
    end
    return rows
end

local function fight_setup_payload(config)
    local known_spells = spell_rows_for_level(config.className, config.level)
    local loadout = loadout_for_config(config)
    local selected = {
        class = class_lookup()[config.className],
        level = level_lookup()[config.level],
        map = map_lookup()[config.mapDesc or ""],
        mapAlgorithm = selected_map_algorithm(config),
        encounter = encounter_lookup()[config.encounter or ""],
        intensity = selected_intensity(config),
    }
    local encounter = encounter_with_components(selected.encounter, config.packageComponents)
    local rhythms = encounter_rhythms(encounter, selected.intensity)
    local package_summary = package_summary_from_rhythms(rhythms)
    local control_model = control_model_for(config)
    local micro_plan = micro_plan_for(rhythms, control_model)
    local pressure_preview = pressure_preview_for(rhythms)
    return {
        schema = 1,
        config = config,
        objective = fight_objective_for(config, selected, encounter, package_summary),
        role = {
            className = config.className,
            classRole = selected.class and selected.class.role or "",
            positioning = control_model.positioning or "",
            targetDistance = control_model.targetDistance or 0,
            botProfile = control_model.botProfile,
            decisionInterval = control_model.decisionInterval or 1,
            notes = control_model.notes or "",
        },
        loadout = loadout,
        loadoutReadiness = loadout_readiness_for_config(config, loadout, known_spells),
        matchup = matchup_read_for(config, encounter),
        threats = threat_windows_for(rhythms),
        microPlan = micro_plan,
        actionGuide = action_guide_payload(config),
        pressurePreview = pressure_preview,
        packageSummary = package_summary,
        telemetry = telemetry_watch_for(config, package_summary),
        warnings = warnings_for(config, selected),
        commands = {
            launch = launch_args(config),
            balance = balance_args(config),
        },
    }
end

local function run_summary_payload(config)
    local known_spells, upcoming_spells = spell_rows_for_level(config.className, config.level)
    local loadout = loadout_for_config(config)
    local selected = {
        class = class_lookup()[config.className],
        level = level_lookup()[config.level],
        map = map_lookup()[config.mapDesc or ""],
        mapAlgorithm = selected_map_algorithm(config),
        encounter = encounter_lookup()[config.encounter or ""],
        intensity = selected_intensity(config),
    }
    local encounter = encounter_with_components(selected.encounter, config.packageComponents)
    local rhythms = encounter_rhythms(encounter, selected.intensity)
    local pressure_preview = pressure_preview_for(rhythms)
    return {
        schema = 1,
        config = config,
        class = selected.class,
        level = selected.level or {value = config.level, label = tostring(config.level)},
        classBuild = class_build_for(config.className),
        resolvedStats = class_stats_for_level(config.className, config.level),
        controlModel = control_model_for(config),
        knownSpells = known_spells,
        upcomingSpells = upcoming_spells,
        loadout = loadout,
        loadoutReadiness = loadout_readiness_for_config(config, loadout, known_spells),
        actionGuide = action_guide_payload(config),
        map = selected.map,
        mapAlgorithm = selected.mapAlgorithm,
        encounter = encounter,
        matchup = matchup_read_for(config, encounter),
        intensity = selected.intensity,
        rhythms = rhythms,
        pressurePreview = pressure_preview,
        packageSummary = package_summary_from_rhythms(rhythms),
        botProfile = selected_bot_profile(config),
        warnings = warnings_for(config, selected),
        args = launch_args(config),
        balanceArgs = balance_args(config),
    }
end

local function micro_drill_config(config, drill)
    local next_config = copy_object(config)
    next_config.drill = drill.id or ""
    next_config.className = drill.className or config.className
    next_config.level = tonumber(drill.level) or config.level
    next_config.seed = tostring(drill.seed or config.seed)
    next_config.preset = drill.preset or config.preset
    next_config.customLoadout = drill.customLoadout or ""
    next_config.mapDesc = ""
    next_config.mapProfile = ""
    next_config.mapAlgorithm = ""
    next_config.encounter = drill.encounter or ""
    next_config.monsters = ""
    next_config.intensity = drill.intensity or config.intensity
    next_config.packageComponents = normalize_package_components(drill.packageComponents or "")
    next_config.botProfile = drill.botProfile or config.botProfile
    if drill.buildId and drill.buildId ~= "" then
        next_config = apply_build_archetype(next_config, drill.buildId)
    end
    return normalized_config(next_config)
end

local function list_has(values, needle)
    for _, value in ipairs(values or {}) do
        if value == needle then
            return true
        end
    end
    return false
end

local function drill_scorecard_for(drill, drill_config, package_summary, pressure_preview, matchup)
    local rows = {}
    local telemetry = {}
    local notes = {}
    local seen = {}
    local function watch(metric)
        if metric and metric ~= "" and not seen[metric] then
            telemetry[#telemetry + 1] = metric
            seen[metric] = true
        end
    end
    local function add(row)
        rows[#rows + 1] = row
        watch(row.metric)
    end

    add({
        id = "clear-floor",
        label = "Clear floor",
        metric = "win_rate",
        comparator = ">=",
        threshold = 0.35,
        target = ">=35% human clears",
        reason = "Practice drills should be learnable without perfect automation.",
    })
    add({
        id = "clear-ceiling",
        label = "Ceiling check",
        metric = "win_rate",
        comparator = "<=",
        threshold = 0.90,
        target = "<=90% human clears",
        reason = "A primitive human-profile bot clearing every run means the drill is too forgiving.",
    })
    add({
        id = "near-death",
        label = "Pressure floor",
        metric = "avg_min_hp_pct",
        comparator = "<=",
        threshold = 0.55,
        target = "<=55% minimum HP",
        reason = "The drill should produce real danger instead of cosmetic package timings.",
    })
    if package_summary then
        add({
            id = "micro-clean",
            label = "Micro execution",
            metric = "avg_package_micro_score",
            comparator = ">=",
            threshold = 0,
            target = ">=0 micro score",
            reason = "A clear should not count as healthy when the bot wins while eating avoidable package mechanics.",
        })
    end

    if package_summary and package_summary.hasRescue then
        add({
            id = "rescue-save",
            label = "Save reliability",
            metric = "avg_package_rescue_save_rate",
            comparator = ">=",
            threshold = 0.60,
            target = ">=60% saves",
            reason = "The core read is protecting the ally, not merely killing the boss.",
        })
        add({
            id = "rescue-perfect",
            label = "Perfect saves",
            metric = "avg_package_rescue_perfect_rate",
            comparator = ">=",
            threshold = list_has(drill.tags, "perfect") and 0.12 or 0.05,
            target = list_has(drill.tags, "perfect") and ">=12% perfect saves" or ">=5% perfect saves",
            reason = "Few-frame saves are the intended high-skill moment.",
        })
        add({
            id = "early-parking",
            label = "Early parking",
            metric = "avg_package_rescue_early_rate",
            comparator = "<=",
            threshold = list_has(drill.tags, "perfect") and 0.40 or 0.55,
            target = list_has(drill.tags, "perfect") and "<=40% early saves" or "<=55% early saves",
            reason = "The rescue should be solved by late body-block reads, not camping the line.",
        })
        add({
            id = "timing-quality",
            label = "Timing quality",
            metric = "avg_package_rescue_timing_score",
            comparator = ">=",
            threshold = list_has(drill.tags, "perfect") and 0.35 or 0.20,
            target = list_has(drill.tags, "perfect") and ">=0.35 timing score" or ">=0.20 timing score",
            reason = "Clutch and perfect saves should appear often enough to prove few-frame micro.",
        })
        add({
            id = "taunt-cheese",
            label = "Taunt reliance",
            metric = "avg_package_rescue_pull_uptime_rate",
            comparator = "<=",
            threshold = 0.35,
            target = "<=35% pull uptime",
            reason = "High forced-target uptime means taunt is carrying the rescue lane.",
        })
        add({
            id = "ally-margin",
            label = "Ally margin",
            metric = "avg_package_escort_hp_pct",
            comparator = ">=",
            threshold = 0.20,
            target = ">=20% ally HP",
            reason = "Successful clears should leave evidence that the ally was actually protected.",
        })
    else
        notes[#notes + 1] = "No rescue lane in this drill; rescue-save metrics are intentionally absent."
    end

    if package_summary and package_summary.hasSummons then
        add({
            id = "add-tempo",
            label = "Add cleanup",
            metric = "kills_per_1000_combat_frames",
            comparator = ">=",
            threshold = 2.5,
            target = ">=2.5 kills / 1000 combat frames",
            reason = "Summon drills should fail when adds are ignored or cleanup is too slow.",
        })
        add({
            id = "enemy-cap",
            label = "Enemy cap",
            metric = "avg_max_visible_enemies",
            comparator = "<=",
            threshold = (package_summary.summonCap or 4) + 1,
            target = "<=summon cap + 1",
            reason = "Add pressure should saturate but not become unreadable clutter.",
        })
    end

    if pressure_preview and (pressure_preview.peakStack or 0) >= 2 then
        add({
            id = "stack-present",
            label = "Stack present",
            metric = "avg_package_pressure_max_count",
            comparator = ">=",
            threshold = pressure_preview.peakStack or 2,
            target = ">=" .. tostring(pressure_preview.peakStack or 2) .. " active/warning stack",
            reason = "The drill should preserve the package overlap advertised by the preview.",
        })
        add({
            id = "overlap-rate",
            label = "Overlap rate",
            metric = "avg_package_pressure_overlap_rate",
            comparator = ">=",
            threshold = math.max(0.05, tonumber(pressure_preview.overlapRate) or 0.05),
            target = "overlap visible in samples",
            reason = "Without overlap this becomes serial pressure rather than micro layering.",
        })
    end

    local class = class_lookup()[drill_config.className] or {}
    if class.range == "far" or list_has(drill.tags, "ranged") then
        add({
            id = "keepaway",
            label = "Keep-away",
            metric = "avg_ranged_keepaway_frames",
            comparator = ">",
            threshold = 0,
            target = ">0 keep-away frames",
            reason = "Ranged drills should prove the class is maintaining space rather than face-tanking.",
        })
        add({
            id = "forced-close",
            label = "Forced close",
            metric = "avg_ranged_close_enemy_frames",
            comparator = ">",
            threshold = 0,
            target = ">0 forced-close frames",
            reason = "The enemy package should still create access windows against ranged classes.",
        })
    end

    if drill_config.className == "Necromancer" then
        add({
            id = "regen-offset",
            label = "Regen offset",
            metric = "avg_necromancer_regen_offset_pct",
            comparator = ">",
            threshold = 0,
            target = ">0 regen offset",
            reason = "Necromancer pressure must be read after regen correction.",
        })
    end

    if matchup and matchup.severity == "high" then
        notes[#notes + 1] = "This drill includes a high-severity class/enemy matchup check."
        for _, metric in ipairs(matchup.telemetry or {}) do
            watch(metric)
        end
    end

    return {
        schema = 1,
        label = "Drill scorecard",
        summary = drill.focus or drill.role or "",
        rows = rows,
        telemetry = telemetry,
        notes = notes,
    }
end

local function micro_drill_rows(config)
    local rows = {}
    local encounters = encounter_lookup()
    for _, drill in ipairs(Catalog.micro_drills or {}) do
        local drill_config = micro_drill_config(config, drill)
        local encounter = encounter_with_components(encounters[drill_config.encounter or ""], drill_config.packageComponents)
        local rhythms = encounter_rhythms(encounter, selected_intensity(drill_config))
        local package_summary = package_summary_from_rhythms(rhythms)
        local known_spells, upcoming_spells = spell_rows_for_level(drill_config.className, drill_config.level)
        local loadout = loadout_for_config(drill_config)
        local build = build_archetype_by_id(drill_config.className, drill.buildId)
        local control_model = control_model_for(drill_config)
        local pressure_preview = pressure_preview_for(rhythms)
        local matchup = matchup_read_for(drill_config, encounter)
        local scorecard = drill_scorecard_for(drill, drill_config, package_summary, pressure_preview, matchup)
        rows[#rows + 1] = {
            id = drill.id or "",
            label = drill.label or drill.id or "",
            role = drill.role or "",
            focus = drill.focus or "",
            tags = copy_array(drill.tags or {}),
            config = drill_config,
            buildArchetype = build and {
                id = build.id,
                label = build.label,
                role = build.role,
                tags = copy_array(build.tags or {}),
            } or nil,
            loadout = loadout,
            loadoutReadiness = loadout_readiness_for_config(drill_config, loadout, known_spells),
            matchup = matchup,
            knownSpells = known_spells,
            upcomingSpells = upcoming_spells,
            encounter = encounter,
            rhythms = rhythms,
            packageSummary = package_summary,
            pressurePreview = pressure_preview,
            scorecard = scorecard,
            microPlan = micro_plan_for(rhythms, control_model),
            actionGuide = action_guide_payload(drill_config),
            telemetry = telemetry_watch_for(drill_config, package_summary),
            args = launch_args(drill_config),
            balanceArgs = balance_args(drill_config),
        }
    end
    return rows
end

local function micro_drills_payload(config)
    local rows = micro_drill_rows(config)
    return {
        schema = 1,
        config = config,
        count = #rows,
        drills = rows,
    }
end

local function micro_drill_resources(config, start_offset)
    local resources = {}
    for idx, drill in ipairs(micro_drill_rows(config)) do
        resources[#resources + 1] = resource_entry(
            "Micro drill sweep",
            drill.label,
            string.format("%s, %s", drill.config.className, drill.role or "micro"),
            drill.focus or "micro drill",
            drill.config,
            (start_offset or 120) + idx,
            {
                microDrill = {
                    id = drill.id,
                    label = drill.label,
                    role = drill.role,
                    focus = drill.focus,
                    tags = copy_array(drill.tags or {}),
                },
                buildArchetype = drill.buildArchetype,
                loadoutReadiness = drill.loadoutReadiness,
                matchup = drill.matchup,
                packageSummary = drill.packageSummary,
                pressurePreview = drill.pressurePreview,
                scorecard = drill.scorecard,
            }
        )
    end
    return resources
end

local function selected_package_encounter(config)
    local selected_encounter = encounter_lookup()[config.encounter or ""]
    if selected_encounter and selected_encounter.package then
        return selected_encounter
    end
    for _, encounter in ipairs(package_encounters()) do
        if encounter.id == "gragh-rescue-package" then
            return encounter
        end
    end
    return package_encounters()[1]
end

local function package_resource_entry(group, name, meta, goal, config, encounter, seed_offset, extras)
    local intensity = selected_intensity(config)
    local component_encounter = encounter_with_components(encounter, config.packageComponents)
    local rhythms = encounter_rhythms(component_encounter, intensity)
    local package_summary = package_summary_from_rhythms(rhythms)
    local extra = copy_object(extras or {})
    extra.encounterLabel = encounter and encounter.label or config.encounter
    extra.encounterKind = encounter and encounter.kind or ""
    extra.encounterPhase = encounter and encounter.phase or ""
    extra.suggestedLevel = encounter and encounter.suggestedLevel or nil
    extra.suggestedPreset = encounter and encounter.suggestedPreset or nil
    extra.rhythms = rhythms
    extra.packageSummary = package_summary
    return resource_entry(group, name, meta, goal, config, seed_offset, extra)
end

local function package_base_config(config, encounter)
    local next_config = copy_object(config)
    next_config.level = encounter_resource_level(encounter)
    next_config.preset = encounter_resource_preset(encounter, next_config.preset, next_config.level)
    next_config.mapDesc = ""
    next_config.mapProfile = ""
    next_config.mapAlgorithm = ""
    next_config.encounter = encounter.id or ""
    next_config.monsters = ""
    return next_config
end

local function package_has_any_component(package, components)
    if not package then
        return false
    end
    local selected = package_component_lookup(components)
    if not selected then
        return true
    end
    for component, enabled in pairs(selected) do
        if enabled and package[component] then
            return true
        end
    end
    return false
end

local function package_matrix_payload(config)
    local resources = {}
    local packages = package_encounters()
    local selected_package = selected_package_encounter(config)

    for idx, encounter in ipairs(packages) do
        local next_config = package_base_config(config, encounter)
        next_config.intensity = "standard"
        resources[#resources + 1] = package_resource_entry(
            "Package baselines",
            encounter.label or encounter.id,
            string.format("L%d, %s, %s", next_config.level, next_config.preset, encounter.phase or encounter.kind or "package"),
            "declared package baseline at its suggested level and loadout",
            next_config,
            encounter,
            idx,
            {
                packageRole = "baseline",
            }
        )
    end

    if selected_package then
        for idx, class in ipairs(Catalog.classes or {}) do
            local next_config = package_base_config(config, selected_package)
            next_config.className = class.id
            resources[#resources + 1] = package_resource_entry(
                "Selected package class sweep",
                class.label or class.id,
                string.format("%s, L%d, %s", selected_package.label or selected_package.id, next_config.level, next_config.intensity),
                class.balance_focus or "class buffer against selected package",
                next_config,
                selected_package,
                30 + idx,
                {
                    packageRole = "class-sweep",
                }
            )
        end

        for idx, intensity in ipairs(Catalog.intensities or {}) do
            local next_config = package_base_config(config, selected_package)
            next_config.intensity = intensity.id or "standard"
            resources[#resources + 1] = package_resource_entry(
                "Selected package pressure sweep",
                intensity.label or intensity.id,
                string.format("%s, %.2fx tempo", selected_package.label or selected_package.id, tonumber(intensity.tempo) or 1),
                intensity.description or "package pressure comparison",
                next_config,
                selected_package,
                50 + idx,
                {
                    packageRole = "pressure-sweep",
                }
            )
        end

        for idx, profile in ipairs(Catalog.bot_profiles or {}) do
            local next_config = package_base_config(config, selected_package)
            next_config.botProfile = profile.id or "human"
            resources[#resources + 1] = package_resource_entry(
                "Selected package skill buffer",
                profile.label or profile.id,
                string.format("%s, %s automation", selected_package.label or selected_package.id, profile.id or "human"),
                profile.description or "automated-player skill buffer comparison",
                next_config,
                selected_package,
                60 + idx,
                {
                    packageRole = "skill-buffer",
                    controlModel = control_model_for(next_config),
                }
            )
        end

        for idx, component_set in ipairs(Catalog.package_component_sets or {}) do
            local components = component_set.components or component_set.id or ""
            if components ~= "" and package_has_any_component(selected_package.package, components) then
                local next_config = package_base_config(config, selected_package)
                next_config.packageComponents = normalize_package_components(components)
                resources[#resources + 1] = package_resource_entry(
                    "Selected package component probes",
                    component_set.label or component_set.id,
                    string.format("%s, %s", selected_package.label or selected_package.id, component_set.label or components),
                    component_set.description or "package component isolation probe",
                    next_config,
                    selected_package,
                    80 + idx,
                    {
                        packageRole = "component-probe",
                        packageComponents = next_config.packageComponents,
                    }
                )
            end
        end
    end

    local rescue_classes = {"Fighter", "Necromancer", "Stormcaller"}
    local class_by_id = class_lookup()
    local rescue_offset = 70
    for _, encounter in ipairs(packages) do
        if encounter.package and encounter.package.rescue then
            for _, class_name in ipairs(rescue_classes) do
                local next_config = package_base_config(config, encounter)
                next_config.className = class_name
                next_config.intensity = "high"
                rescue_offset = rescue_offset + 1
                local class = class_by_id[class_name] or {label = class_name}
                resources[#resources + 1] = package_resource_entry(
                    "Rescue package checks",
                    string.format("%s %s", class.label or class_name, encounter.label or encounter.id),
                    string.format("L%d, high pressure, rescue timing", next_config.level),
                    "body-block rescue read under class-specific positioning pressure",
                    next_config,
                    encounter,
                    rescue_offset,
                    {
                        packageRole = "rescue-check",
                    }
                )
            end
        end
    end

    return {
        schema = 1,
        config = config,
        selectedPackage = selected_package,
        count = #resources,
        resources = resources,
    }
end

local function enemy_matrix_payload(config, start_offset)
    local resources = enemy_probe_resources(config, start_offset or 90)
    return {
        schema = 1,
        config = config,
        count = #resources,
        options = enemy_options_payload(config),
        resources = resources,
    }
end

local function append_caster_matchup_resources(resources, config, start_offset)
    local specs = {
        {
            name = "Pyro Red Dragon resist",
            meta = "L5, late loadout, red package",
            goal = "fire-resistance conversion check",
            className = "Pyrocaster",
            encounter = "red-dragon-duel",
            level = 5,
            preset = "late",
            intensity = "high",
        },
        {
            name = "Stormcaller Red Dragon counter",
            meta = "L5, late loadout, red package",
            goal = "same red package without fire-resist penalty",
            className = "Stormcaller",
            encounter = "red-dragon-duel",
            level = 5,
            preset = "late",
            intensity = "high",
        },
        {
            name = "Stormcaller white mirror",
            meta = "L3, Cloud Elemental duel",
            goal = "white-resistance overperformance check",
            className = "Stormcaller",
            monsters = "Cloud Elemental",
            level = 3,
            preset = "none",
            intensity = "standard",
        },
        {
            name = "Necro Crypt resist",
            meta = "L5, late loadout, black package",
            goal = "regen-adjusted black-resistance check",
            className = "Necromancer",
            encounter = "crypt-keeper-aura-package",
            level = 5,
            preset = "late",
            intensity = "high",
        },
    }
    local encounters = encounter_lookup()
    for idx, spec in ipairs(specs) do
        local next_config = copy_object(config)
        next_config.className = spec.className
        next_config.level = spec.level
        next_config.preset = spec.preset
        next_config.intensity = spec.intensity
        next_config.mapDesc = ""
        next_config.mapProfile = ""
        next_config.mapAlgorithm = ""
        next_config.encounter = spec.encounter or ""
        next_config.monsters = spec.monsters or ""
        local encounter = encounters[next_config.encounter or ""]
        resources[#resources + 1] = resource_entry(
            "Caster matchup sweep",
            spec.name,
            spec.meta,
            spec.goal,
            next_config,
            (start_offset or 110) + idx,
            {
                matchup = matchup_read_for(next_config, encounter),
            }
        )
    end
end

local function resource_matrix_payload(config)
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

    for idx, preset_name in ipairs(Catalog.loadout_order or {}) do
        local preset = Catalog.loadout_presets and Catalog.loadout_presets[preset_name]
        if preset then
            local next_config = {}
            for key, value in pairs(config) do
                next_config[key] = value
            end
            next_config.preset = preset_name
            local label = preset.label or preset_name
            local custom = next_config.customLoadout ~= "" and " + custom" or ""
            resources[#resources + 1] = resource_entry(
                "Loadout sweep",
                label,
                string.format("L%d, %s%s, %s", next_config.level, label, custom, selected_target_label),
                preset.description or "loadout comparison",
                next_config,
                12 + idx
            )
        end
    end

    for idx, profile in ipairs(Catalog.bot_profiles or {}) do
        local next_config = {}
        for key, value in pairs(config) do
            next_config[key] = value
        end
        next_config.botProfile = profile.id or "human"
        resources[#resources + 1] = resource_entry(
            "Skill buffer sweep",
            profile.label or profile.id,
            string.format("%s automation, %s", profile.id or "human", selected_target_label),
            profile.description or "automated-player skill buffer comparison",
            next_config,
            30 + idx,
            {
                controlModel = control_model_for(next_config),
            }
        )
    end

    append_build_archetype_resources(resources, config, 36)

    append_caster_matchup_resources(resources, config, 110)

    append_resources(resources, micro_drill_resources(config, 120))

    append_resources(resources, map_probe_resources(config, 40))

    for idx, encounter in ipairs(package_encounters()) do
        local next_config = {}
        for key, value in pairs(config) do
            next_config[key] = value
        end
        next_config.level = encounter_resource_level(encounter)
        next_config.preset = encounter_resource_preset(encounter, next_config.preset, next_config.level)
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

    append_resources(resources, enemy_probe_resources(config, 90))

    return {
        schema = 1,
        config = config,
        count = #resources,
        resources = resources,
    }
end

local function shell_arg(value)
    local text = tostring(value == nil and "" or value)
    if text == "" then
        return "''"
    end
    if text:match("^[A-Za-z0-9_./:=@%%+%-]+$") then
        return text
    end
    return "'" .. text:gsub("'", "'\\''") .. "'"
end

local function command_text(args)
    local parts = {}
    for _, arg in ipairs(args or {}) do
        parts[#parts + 1] = shell_arg(arg)
    end
    return table.concat(parts, " ")
end

local function option_rows(entries, value_key, label_key)
    local rows = {}
    for _, entry in ipairs(entries or {}) do
        local value = entry[value_key or "id"]
        rows[#rows + 1] = {
            value = value == nil and "" or tostring(value),
            label = entry[label_key or "label"] or entry[value_key or "id"] or "",
            phase = entry.phase,
            description = entry.description,
            primary = entry.primary == true,
        }
    end
    return rows
end

local function controls_payload(config)
    local map_algorithms = algorithms_for_map(config.mapDesc or "")
    if #map_algorithms == 0 then
        map_algorithms = Catalog.map_algorithms or {}
    end
    return {
        {
            id = "className",
            label = "Class",
            value = config.className,
            options = option_rows(Catalog.classes or {}, "id", "label"),
        },
        {
            id = "level",
            label = "Level",
            value = tostring(config.level),
            options = option_rows(Catalog.levels or {}, "value", "label"),
        },
        {
            id = "preset",
            label = "Loadout",
            value = config.preset,
            options = (function()
                local rows = {}
                for _, name in ipairs(Catalog.loadout_order or {}) do
                    local preset = Catalog.loadout_presets and Catalog.loadout_presets[name] or {}
                    rows[#rows + 1] = {
                        value = name,
                        label = preset.label or name,
                        description = preset.description or "",
                    }
                end
                return rows
            end)(),
        },
        {
            id = "mapDesc",
            label = "Map generator",
            value = config.mapDesc,
            options = option_rows(Catalog.maps or {}, "id", "label"),
        },
        {
            id = "mapAlgorithm",
            label = "Map algorithm",
            value = config.mapAlgorithm,
            options = option_rows(map_algorithms, "id", "label"),
        },
        {
            id = "encounter",
            label = "Encounter",
            value = config.encounter,
            options = option_rows(Catalog.encounters or {}, "id", "label"),
        },
        {
            id = "intensity",
            label = "Pressure",
            value = config.intensity,
            options = option_rows(Catalog.intensities or {}, "id", "label"),
        },
        {
            id = "packageComponents",
            label = "Package parts",
            value = config.packageComponents,
            options = option_rows(Catalog.package_component_sets or {}, "components", "label"),
        },
        {
            id = "drill",
            label = "Drill",
            value = config.drill,
            options = (function()
                local rows = {{value = "", label = "None"}}
                for _, drill in ipairs(Catalog.micro_drills or {}) do
                    rows[#rows + 1] = {
                        value = drill.id or "",
                        label = drill.label or drill.id or "",
                        description = drill.focus or drill.role or "",
                    }
                end
                return rows
            end)(),
        },
        {
            id = "enemyGroup",
            label = "Enemy group",
            value = config.enemyGroup,
            options = option_rows(Catalog.enemy_groups or {}, "id", "label"),
        },
        {
            id = "enemyPack",
            label = "Enemy pack",
            value = config.enemyPack,
            options = (function()
                local rows = {{value = "", label = "None"}}
                for _, pack in ipairs(Catalog.enemy_packs or {}) do
                    rows[#rows + 1] = {
                        value = pack.id or "",
                        label = pack.label or pack.id or "",
                        description = pack.goal or pack.description or "",
                    }
                end
                return rows
            end)(),
        },
        {
            id = "goal",
            label = "Goal",
            value = config.goal,
            options = option_rows(Catalog.analysis_goals or {}, "id", "label"),
        },
        {
            id = "botProfile",
            label = "Bot skill",
            value = config.botProfile,
            options = option_rows(Catalog.bot_profiles or {}, "id", "label"),
        },
    }
end

local function command_preview(args)
    return {
        args = args or {},
        text = command_text(args or {}),
    }
end

local function resource_summary_chips(resource)
    local chips = {}
    local function add(value)
        if value ~= nil and tostring(value) ~= "" then
            chips[#chips + 1] = tostring(value)
        end
    end
    add(resource.className)
    add("L" .. tostring(resource.level or 1))
    if resource.preset and resource.preset ~= "none" then
        add(resource.preset)
    end
    if resource.intensity and resource.intensity ~= "standard" then
        add(resource.intensity)
    end
    if resource.botProfile and resource.botProfile ~= "human" then
        add(resource.botProfile)
    end
    if resource.packageComponents and resource.packageComponents ~= "" then
        add(resource.packageComponents)
    end
    if resource.microDrill then
        add(resource.microDrill.role)
        for _, tag in ipairs(resource.microDrill.tags or {}) do
            add(tag)
        end
    end
    if resource.scorecard then
        add(resource.scorecard.label or "scorecard")
        for _, row in ipairs(resource.scorecard.rows or {}) do
            add(row.label)
            if #chips >= 12 then
                break
            end
        end
    end
    if resource.mapProfile and resource.mapProfile ~= "" then
        add(resource.mapProfile)
    end
    if resource.monsters and resource.monsters ~= "" then
        add(resource.monsters)
    end
    if resource.packageSummary then
        local summary = resource.packageSummary
        if summary.fastestLoopFrames and summary.fastestLoopFrames > 0 then
            add(tostring(summary.fastestLoopFrames) .. "f loop")
        end
        if summary.predictedPeakStack and summary.predictedPeakStack > 0 then
            add("stack " .. tostring(summary.predictedPeakStack))
        end
        if summary.predictedOverlapRate and summary.predictedOverlapRate > 0 then
            add("overlap " .. tostring(math.floor(summary.predictedOverlapRate * 100 + 0.5)) .. "%")
        end
        if summary.predictedPressureLabel and summary.predictedPressureLabel ~= "" then
            add(summary.predictedPressureLabel)
        end
        if summary.hasRescue then
            add("rescue")
        end
        if summary.hasSummons then
            add("adds")
        end
    end
    if resource.signature and resource.signature.enemyMix then
        add(resource.signature.enemyMix)
    elseif resource.signature and resource.signature.ambientEnemies then
        add(tostring(resource.signature.ambientEnemies) .. " ambient")
    end
    if resource.mapThreat then
        add(tostring(resource.mapThreat.label or "threat") .. " " .. tostring(resource.mapThreat.score or 0))
        if (resource.mapThreat.clusteredPairs or 0) > 0 then
            add(tostring(resource.mapThreat.clusteredPairs) .. " clusters")
        end
    end
    if resource.loadoutReadiness then
        add("fit " .. tostring(resource.loadoutReadiness.label or "read"))
        if resource.loadoutReadiness.score then
            add(tostring(resource.loadoutReadiness.score) .. " build")
        end
    end
    if resource.matchup then
        add(resource.matchup.label)
        if resource.matchup.severity and resource.matchup.severity ~= "neutral" then
            add(resource.matchup.severity)
        end
    end
    return chips
end

local function resource_card(resource)
    return {
        id = resource.id,
        group = resource.group,
        title = resource.name,
        subtitle = resource.meta or resource.goal or "",
        goal = resource.goal or "",
        chips = resource_summary_chips(resource),
        config = {
            className = resource.className,
            level = resource.level,
            preset = resource.preset,
            intensity = resource.intensity,
            packageComponents = resource.packageComponents,
            botProfile = resource.botProfile,
            customLoadout = resource.customLoadout,
            mapDesc = resource.mapDesc,
            mapProfile = resource.mapProfile,
            mapAlgorithm = resource.mapAlgorithm,
            encounter = resource.encounter,
            drill = resource.drill or (resource.microDrill and resource.microDrill.id) or "",
            monsters = resource.monsters,
            enemyGroup = resource.enemyGroup,
            enemyPack = resource.enemyPack,
            goal = resource.analysisGoal,
            seed = resource.seed,
        },
        command = command_preview(resource.args),
        balance = command_preview(resource.balanceArgs),
        packageSummary = resource.packageSummary,
        pressurePreview = resource.pressurePreview,
        microDrill = resource.microDrill,
        scorecard = resource.scorecard,
        buildArchetype = resource.buildArchetype,
        loadoutReadiness = resource.loadoutReadiness,
        previewMode = resource.previewMode,
        signature = resource.signature,
        mapThreat = resource.mapThreat,
        matchup = resource.matchup,
        previewStats = resource.previewStats,
        monsterEval = resource.monsterEvalArgs and command_preview(resource.monsterEvalArgs) or nil,
    }
end

local function resource_cards(resources)
    local cards = {}
    for _, resource in ipairs(resources or {}) do
        cards[#cards + 1] = resource_card(resource)
    end
    return cards
end

local function preview_card(preview)
    local stats = preview.previewPlan and preview.previewPlan.stats or {}
    local chips = {}
    local function add(value)
        if value ~= nil and tostring(value) ~= "" then
            chips[#chips + 1] = tostring(value)
        end
    end
    add(preview.mapProfile)
    add(preview.phase)
    if stats.rooms then
        add(tostring(stats.rooms) .. " rooms")
    end
    if stats.ambientEnemies then
        add(tostring(stats.ambientEnemies) .. " ambient")
    end
    if stats.enemyMix and stats.enemyMix ~= "" then
        add(stats.enemyMix)
    end
    local threat = preview.threatProfile or (preview.previewPlan and preview.previewPlan.threatProfile)
    if threat then
        add(tostring(threat.label or "threat") .. " " .. tostring(threat.score or 0))
        if (threat.clusteredPairs or 0) > 0 then
            add(tostring(threat.clusteredPairs) .. " clusters")
        end
        if (threat.casterCount or 0) > 0 then
            add(tostring(threat.casterCount) .. " caster")
        end
    end
    return {
        id = preview.id,
        title = preview.label or preview.id,
        subtitle = preview.description or "",
        chips = chips,
        mapDesc = preview.mapDesc,
        mapProfile = preview.mapProfile,
        mapAlgorithm = preview.mapAlgorithm,
        previewMode = preview.preview_mode,
        previewPlan = preview.previewPlan,
        threatProfile = threat,
        command = command_preview(preview.args),
        balance = command_preview(preview.balanceArgs),
    }
end

local function preview_cards(map_generation)
    local cards = {}
    for _, preview in ipairs(map_generation and map_generation.previews or {}) do
        cards[#cards + 1] = preview_card(preview)
    end
    return cards
end

local function selected_summary(config, run_summary, fight_setup)
    local objective = fight_setup and fight_setup.objective or {}
    local map = run_summary and run_summary.map or nil
    local map_algorithm = run_summary and run_summary.mapAlgorithm or nil
    local encounter = run_summary and run_summary.encounter or nil
    local loadout = run_summary and run_summary.loadout or nil
    local class = run_summary and run_summary.class or nil
    local bot_profile = run_summary and run_summary.botProfile or selected_bot_profile(config)
    return {
        title = objective.label
            or (encounter and (encounter.label or encounter.id))
            or (map_algorithm and (map_algorithm.label or map_algorithm.id))
            or (map and (map.label or map.id))
            or "Lanarts content lab",
        subtitle = string.format(
            "%s L%d, %s loadout, %s bot",
            config.className,
            tonumber(config.level) or 1,
            loadout and (loadout.label or loadout.id) or config.preset,
            bot_profile and (bot_profile.label or bot_profile.id) or config.botProfile
        ),
        class = class,
        loadout = loadout,
        map = map,
        mapAlgorithm = map_algorithm,
        encounter = encounter,
        objective = objective,
        botProfile = bot_profile,
        controlModel = run_summary and run_summary.controlModel or control_model_for(config),
        matchup = run_summary and run_summary.matchup or nil,
    }
end

local function presentation_payload(config)
    local run_summary = run_summary_payload(config)
    local fight_setup = fight_setup_payload(config)
    local map_generation = map_generation_payload(config)
    local map_matrix = map_matrix_payload(config)
    local package_matrix = package_matrix_payload(config)
    local resource_matrix = resource_matrix_payload(config)
    local enemy_matrix = enemy_matrix_payload(config, 90)
    local analysis_plan = analysis_plan_payload(config)
    local build_matrix = build_matrix_payload(config)
    local micro_drills = micro_drills_payload(config)
    local action_guide = action_guide_payload(config)

    return {
        schema = 1,
        config = config,
        selected = selected_summary(config, run_summary, fight_setup),
        controls = controls_payload(config),
        commandPreviews = {
            launch = command_preview(run_summary.args),
            balance = command_preview(run_summary.balanceArgs),
            classSweep = command_preview(analysis_plan.commands and analysis_plan.commands.classSweep),
            monsterEval = command_preview(analysis_plan.commands and analysis_plan.commands.monsterEval),
        },
        buildArchetypes = build_matrix.builds or {},
        microDrills = micro_drills.drills or {},
        actionGuide = action_guide,
        mapPreviews = preview_cards(map_generation),
        resourceGroups = {
            {
                id = "build",
                label = "Build archetypes",
                count = build_matrix.count or 0,
                cards = resource_cards(build_archetype_resources(config, 0)),
            },
            {
                id = "drill",
                label = "Micro drills",
                count = micro_drills.count or 0,
                cards = resource_cards(micro_drill_resources(config, 0)),
            },
            {
                id = "map",
                label = "Map matrix",
                count = map_matrix.count or 0,
                cards = resource_cards(map_matrix.resources),
            },
            {
                id = "package",
                label = "Package matrix",
                count = package_matrix.count or 0,
                cards = resource_cards(package_matrix.resources),
            },
            {
                id = "enemy",
                label = "Enemy probes",
                count = enemy_matrix.count or 0,
                cards = resource_cards(enemy_matrix.resources),
            },
            {
                id = "orthogonal",
                label = "Orthogonal resources",
                count = resource_matrix.count or 0,
                cards = resource_cards(resource_matrix.resources),
            },
        },
        telemetry = fight_setup.telemetry or {},
        threats = fight_setup.threats or {},
        microPlan = fight_setup.microPlan or {},
        beatPlan = action_guide.beatPlan or {},
        matchup = run_summary.matchup or fight_setup.matchup,
        pressurePreview = fight_setup.pressurePreview or {},
        warnings = run_summary.warnings or {},
    }
end

local function workspace_payload(config)
    local analysis_plan = analysis_plan_payload(config)
    return {
        schema = 1,
        config = config,
        loadoutOptions = build_options_payload(config),
        actionGuide = action_guide_payload(config),
        buildMatrix = build_matrix_payload(config),
        microDrills = micro_drills_payload(config),
        runSummary = run_summary_payload(config),
        enemyOptions = enemy_options_payload(config),
        mapGeneration = map_generation_payload(config),
        mapMatrix = map_matrix_payload(config),
        fightSetup = fight_setup_payload(config),
        analysisPlan = analysis_plan,
        packageMatrix = package_matrix_payload(config),
        resourceMatrix = resource_matrix_payload(config),
        botProfiles = Catalog.bot_profiles or {},
        controlModel = control_model_for(config),
        commands = analysis_plan.commands,
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

function M.build_matrix_json(raw_json)
    return encode(build_matrix_payload(normalized_config(parse_json_object(raw_json))))
end

function M.micro_drills_json(raw_json)
    return encode(micro_drills_payload(normalized_config(parse_json_object(raw_json))))
end

function M.action_guide_json(raw_json)
    return encode(action_guide_payload(normalized_config(parse_json_object(raw_json))))
end

function M.run_summary_json(raw_json)
    return encode(run_summary_payload(normalized_config(parse_json_object(raw_json))))
end

function M.fight_setup_json(raw_json)
    return encode(fight_setup_payload(normalized_config(parse_json_object(raw_json))))
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

function M.map_matrix_json(raw_json)
    return encode(map_matrix_payload(normalized_config(parse_json_object(raw_json))))
end

function M.enemy_options_json(raw_json)
    return encode(enemy_options_payload(normalized_config(parse_json_object(raw_json))))
end

function M.enemy_matrix_json(raw_json)
    return encode(enemy_matrix_payload(normalized_config(parse_json_object(raw_json)), 90))
end

function M.analysis_plan_json(raw_json)
    return encode(analysis_plan_payload(normalized_config(parse_json_object(raw_json))))
end

function M.resource_matrix_json(raw_json)
    return encode(resource_matrix_payload(normalized_config(parse_json_object(raw_json))))
end

function M.package_matrix_json(raw_json)
    return encode(package_matrix_payload(normalized_config(parse_json_object(raw_json))))
end

function M.workspace_json(raw_json)
    return encode(workspace_payload(normalized_config(parse_json_object(raw_json))))
end

function M.presentation_json(raw_json)
    return encode(presentation_payload(normalized_config(parse_json_object(raw_json))))
end

return M
