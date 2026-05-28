local json = require "json"
local ContentApi = require "engine.ContentApi"

local function parse_response(label, text)
    local ok, parsed = json.parse(text)
    if not ok then
        error(label .. " returned invalid JSON.")
    end
    if parsed and parsed.error then
        error(label .. " failed: " .. tostring(parsed.error))
    end
    return parsed
end

local function expect(condition, message)
    if not condition then
        error(message)
    end
end

local function run_content_api_smoke()
    local catalog = parse_response("catalog", ContentApi.catalog_json())
    expect(catalog.schema == 1, "catalog schema mismatch")
    expect(#(catalog.classes or {}) >= 4, "catalog should expose classes")
    expect(catalog.class_builds and catalog.class_builds.Fighter, "catalog should expose class builds")
    expect(#(catalog.item_choices or {}) >= 4, "catalog should expose curated item choices")
    expect(#(catalog.bot_profiles or {}) >= 4, "catalog should expose bot skill profiles")
    expect(#(catalog.analysis_goals or {}) >= 3, "catalog should expose analysis goals")
    expect(#(catalog.enemy_groups or {}) >= 3, "catalog should expose enemy groups")
    expect(#(catalog.enemy_packs or {}) >= 3, "catalog should expose enemy packs")
    expect(#(catalog.maps or {}) >= 4, "catalog should expose map generators")
    expect(#(catalog.map_algorithms or {}) >= 7, "catalog should expose map algorithms")
    expect(#(catalog.encounters or {}) >= 10, "catalog should expose packaged encounters")
    local has_giant_chicken_package = false
    local has_mummoner_package = false
    local has_executioner_package = false
    local has_executioner_rescue_package = false
    local has_human_bot_profile = false
    local has_pyro_keepaway_model = false
    local has_executioner_baseline = false
    local has_predator_thicket = false
    local has_ruin_crossroads = false
    for _, algorithm in ipairs(catalog.map_algorithms or {}) do
        if algorithm.id == "overworld-predator-thicket" then
            has_predator_thicket = true
        elseif algorithm.id == "overworld-ruin-crossroads" then
            has_ruin_crossroads = true
        end
    end
    for _, profile in ipairs(catalog.bot_profiles or {}) do
        if profile.id == "human" and (profile.decisionInterval or 0) > 1 then
            has_human_bot_profile = true
        end
    end
    for _, class in ipairs(catalog.classes or {}) do
        if class.id == "Pyrocaster" and class.range == "far" and (class.targetDistance or 0) >= 120 then
            has_pyro_keepaway_model = true
        end
    end
    for _, encounter in ipairs(catalog.encounters or {}) do
        if encounter.id == "giant-chicken-duel" and encounter.package then
            has_giant_chicken_package = true
        elseif encounter.id == "mummoner-duel" and encounter.package then
            has_mummoner_package = true
        elseif encounter.id == "executioner-duel" and encounter.kind == "baseline duel" then
            has_executioner_baseline = true
        elseif encounter.id == "executioner-blade-package" then
            has_executioner_package = true
        elseif encounter.id == "executioner-rescue-package" then
            has_executioner_rescue_package = true
        end
    end
    expect(has_giant_chicken_package, "catalog should expose Giant Chicken as a package")
    expect(has_mummoner_package, "catalog should expose Mummoner as a package")
    expect(has_executioner_baseline, "catalog should mark the plain Executioner duel as a baseline")
    expect(has_executioner_package, "catalog should expose the Executioner blade package")
    expect(has_executioner_rescue_package, "catalog should expose the Executioner rescue package")
    expect(has_human_bot_profile, "catalog should expose the human bot profile")
    expect(has_pyro_keepaway_model, "catalog should expose ranged class control metadata")
    expect(has_predator_thicket, "catalog should expose the predator thicket overworld profile")
    expect(has_ruin_crossroads, "catalog should expose the ruin crossroads overworld profile")

    local config = [[{
        "className": "Necromancer",
        "level": 5,
        "preset": "late",
        "seed": "22450",
        "encounter": "gragh-rescue-package",
        "intensity": "high"
    }]]
    local summary = parse_response("run summary", ContentApi.run_summary_json(config))
    expect(summary.config.className == "Necromancer", "summary should preserve class")
    expect(summary.loadout and #(summary.loadout.items or {}) > 0, "summary should resolve loadout")
    expect(summary.rhythms and #summary.rhythms >= 2, "summary should expose package rhythms")
    local has_rescue_rhythm = false
    local has_rescue_summon = false
    for _, rhythm in ipairs(summary.rhythms or {}) do
        if rhythm.id == "rescue" then
            has_rescue_rhythm = true
            expect((rhythm.pullFrames or 0) > 0, "rescue rhythm should expose pull frames")
        elseif rhythm.id == "summon" then
            has_rescue_summon = true
        end
    end
    expect(has_rescue_rhythm, "summary should expose rescue rhythm metadata")
    expect(has_rescue_summon, "Gragh rescue should expose add rhythm metadata")
    expect(summary.packageSummary and summary.packageSummary.hasRescue, "summary should expose package rescue summary")
    expect(summary.packageSummary and summary.packageSummary.hasSummons, "summary should expose package summon summary")
    expect(summary.packageSummary and summary.packageSummary.hitsEscorts, "summary should expose ally-risk package summary")
    expect(summary.controlModel and summary.controlModel.botProfile and summary.controlModel.botProfile.id == "human", "summary should expose selected human control model")
    expect(summary.controlModel and summary.controlModel.classRange == "close", "summary should expose class positioning model")

    local executioner_baseline_config = [[{
        "className": "Templar",
        "level": 5,
        "preset": "late",
        "seed": "22440",
        "encounter": "executioner-duel",
        "intensity": "standard"
    }]]
    local executioner_baseline_summary = parse_response("executioner baseline summary", ContentApi.run_summary_json(executioner_baseline_config))
    local has_baseline_warning = false
    for _, warning in ipairs(executioner_baseline_summary.warnings or {}) do
        if tostring(warning):find("control cases") then
            has_baseline_warning = true
        end
    end
    expect(executioner_baseline_summary.encounter and executioner_baseline_summary.encounter.kind == "baseline duel", "executioner duel should resolve as a baseline")
    expect(has_baseline_warning, "baseline duel summary should steer tuning toward package variants")
    expect(summary.args and #summary.args > 0, "summary should expose launch args")
    expect(summary.balanceArgs and #summary.balanceArgs > 0, "summary should expose balance args")

    local balance_args = parse_response("balance args", ContentApi.balance_args_json(config))
    expect(balance_args.args and balance_args.args[1] == "engine.StartBalanceRun", "balance args should target StartBalanceRun")
    local has_package_intensity = false
    local has_bot_profile_arg = false
    for _, arg in ipairs(balance_args.args or {}) do
        if arg == "--package_intensity" then
            has_package_intensity = true
        elseif arg == "--bot_profile" then
            has_bot_profile_arg = true
        end
    end
    expect(has_package_intensity, "balance args should preserve package intensity")
    expect(has_bot_profile_arg, "balance args should preserve bot profile")

    local loadout_options = parse_response("loadout options", ContentApi.loadout_options_json(config))
    expect(loadout_options.classBuild and loadout_options.classBuild.spells, "loadout options should expose class build")
    expect(#(loadout_options.knownSpells or {}) >= 1, "loadout options should expose known spells")
    expect(#(loadout_options.itemChoices or {}) >= 3, "loadout options should expose item picker groups")
    local has_necro_amulet = false
    for _, group in ipairs(loadout_options.itemChoices or {}) do
        for _, choice in ipairs(group.choices or {}) do
            if choice.name == "Amulet of Great Pain" then
                has_necro_amulet = true
            end
        end
    end
    expect(has_necro_amulet, "loadout options should expose class-relevant item choices")

    local map_config = [[{
        "className": "Fighter",
        "level": 1,
        "preset": "none",
        "seed": "17300",
        "mapAlgorithm": "overworld-broken-grove"
    }]]
    local map_generation = parse_response("map generation", ContentApi.map_generation_json(map_config))
    expect(map_generation.selectedAlgorithm and map_generation.selectedAlgorithm.mapProfile == "broken_grove", "map generation should resolve overworld profile")
    expect(#(map_generation.previews or {}) >= 6, "map generation should expose profile previews")
    local has_river_preview = false
    local has_predator_preview = false
    local has_ruin_preview = false
    for _, preview in ipairs(map_generation.previews or {}) do
        if preview.mapProfile == "river_road" then
            has_river_preview = true
        elseif preview.mapProfile == "predator_thicket" then
            has_predator_preview = true
        elseif preview.mapProfile == "ruin_crossroads" then
            has_ruin_preview = true
        end
    end
    expect(has_river_preview and has_predator_preview and has_ruin_preview, "map generation should expose new overworld profile previews")
    local launch_args = parse_response("launch args", ContentApi.launch_args_json(map_config))
    local has_map_profile = false
    for _, arg in ipairs(launch_args.args or {}) do
        if arg == "--map_profile" then
            has_map_profile = true
        end
    end
    expect(has_map_profile, "launch args should preserve map profile")

    local red_dragon_config = [[{
        "className": "Pyrocaster",
        "level": 5,
        "preset": "late",
        "seed": "22480",
        "encounter": "red-dragon-duel",
        "intensity": "high"
    }]]
    local red_dragon_summary = parse_response("red dragon summary", ContentApi.run_summary_json(red_dragon_config))
    local red_rhythms = {}
    for _, rhythm in ipairs(red_dragon_summary.rhythms or {}) do
        red_rhythms[rhythm.id] = true
    end
    expect(red_rhythms.orbit and red_rhythms.pulse and red_rhythms.summon, "red dragon should expose full package rhythms")
    expect(red_dragon_summary.encounter and red_dragon_summary.encounter.suggestedLevel == 5, "red dragon should expose late-game suggested level")
    expect(red_dragon_summary.encounter and red_dragon_summary.encounter.suggestedPreset == "late", "red dragon should expose late-game suggested loadout")

    local chicken_config = [[{
        "className": "Fighter",
        "level": 2,
        "preset": "none",
        "seed": "22405",
        "encounter": "giant-chicken-duel",
        "intensity": "standard"
    }]]
    local chicken_summary = parse_response("giant chicken summary", ContentApi.run_summary_json(chicken_config))
    local chicken_rhythms = {}
    for _, rhythm in ipairs(chicken_summary.rhythms or {}) do
        chicken_rhythms[rhythm.id] = true
    end
    expect(chicken_rhythms.orbit and chicken_rhythms.pulse and chicken_rhythms.summon, "giant chicken should expose early package rhythms")
    expect(chicken_summary.encounter and chicken_summary.encounter.suggestedLevel == 2, "giant chicken should expose its suggested level")

    local mummoner_config = [[{
        "className": "Fighter",
        "level": 3,
        "preset": "none",
        "seed": "22410",
        "encounter": "mummoner-duel",
        "intensity": "standard"
    }]]
    local mummoner_summary = parse_response("mummoner summary", ContentApi.run_summary_json(mummoner_config))
    local mummoner_rhythms = {}
    for _, rhythm in ipairs(mummoner_summary.rhythms or {}) do
        mummoner_rhythms[rhythm.id] = true
    end
    expect(mummoner_rhythms.orbit and mummoner_rhythms.pulse and mummoner_rhythms.summon, "mummoner should expose full package rhythms")
    expect(mummoner_summary.encounter and mummoner_summary.encounter.suggestedLevel == 3, "mummoner should expose its suggested level")

    local gragh_config = [[{
        "className": "Fighter",
        "level": 5,
        "preset": "late",
        "seed": "22420",
        "encounter": "gragh-duel",
        "intensity": "high"
    }]]
    local gragh_summary = parse_response("gragh summary", ContentApi.run_summary_json(gragh_config))
    local gragh_rhythms = {}
    for _, rhythm in ipairs(gragh_summary.rhythms or {}) do
        gragh_rhythms[rhythm.id] = true
    end
    expect(gragh_rhythms.orbit and gragh_rhythms.pulse and gragh_rhythms.summon, "gragh should expose full package rhythms")
    expect(gragh_summary.encounter and gragh_summary.encounter.suggestedLevel == 5, "gragh should expose late-game suggested level")
    expect(gragh_summary.encounter and gragh_summary.encounter.suggestedPreset == "late", "gragh should expose late-game suggested loadout")

    local hell_pair_config = [[{
        "className": "Rogue",
        "level": 5,
        "preset": "late",
        "seed": "22430",
        "encounter": "hell-pressure-pair",
        "intensity": "high"
    }]]
    local hell_pair_summary = parse_response("hell pressure pair summary", ContentApi.run_summary_json(hell_pair_config))
    local hell_pair_rhythms = {}
    for _, rhythm in ipairs(hell_pair_summary.rhythms or {}) do
        hell_pair_rhythms[rhythm.id] = true
    end
    expect(hell_pair_rhythms.orbit and hell_pair_rhythms.pulse and hell_pair_rhythms.summon, "hell pressure pair should expose full package rhythms")

    local matrix = parse_response("resource matrix", ContentApi.resource_matrix_json(config))
    expect((matrix.count or 0) == #(matrix.resources or {}), "matrix count should match resources")
    expect((matrix.count or 0) >= 30, "matrix should expose orthogonal resources")
    local has_chicken_resource = false
    local has_mummoner_resource = false
    local has_none_loadout = false
    local has_gragh_loadout = false
    local has_late_loadout = false
    local has_perfect_skill_buffer = false
    local has_sloppy_skill_buffer = false
    for _, resource in ipairs(matrix.resources or {}) do
        if resource.group == "Encounter sweep" and resource.encounter == "giant-chicken-duel" then
            has_chicken_resource = true
            expect(resource.level == 2, "giant chicken resource should use its suggested level")
            expect(resource.preset == "none", "giant chicken resource should use its suggested loadout")
        elseif resource.group == "Encounter sweep" and resource.encounter == "mummoner-duel" then
            has_mummoner_resource = true
            expect(resource.level == 3, "mummoner resource should use its suggested level")
            expect(resource.preset == "none", "mummoner resource should use its suggested loadout")
        elseif resource.group == "Loadout sweep" and resource.encounter == "gragh-rescue-package" then
            has_none_loadout = has_none_loadout or resource.preset == "none"
            has_gragh_loadout = has_gragh_loadout or resource.preset == "gragh"
            has_late_loadout = has_late_loadout or resource.preset == "late"
        elseif resource.group == "Skill buffer sweep" and resource.encounter == "gragh-rescue-package" then
            has_perfect_skill_buffer = has_perfect_skill_buffer or resource.botProfile == "perfect"
            has_sloppy_skill_buffer = has_sloppy_skill_buffer or resource.botProfile == "sloppy"
        end
    end
    expect(has_chicken_resource, "matrix should include Giant Chicken package coverage")
    expect(has_mummoner_resource, "matrix should include Mummoner package coverage")
    expect(has_none_loadout and has_gragh_loadout and has_late_loadout, "matrix should include loadout sweep coverage")
    expect(has_perfect_skill_buffer and has_sloppy_skill_buffer, "matrix should include skill-buffer sweep coverage")

    local package_matrix = parse_response("package matrix", ContentApi.package_matrix_json(config))
    expect((package_matrix.count or 0) == #(package_matrix.resources or {}), "package matrix count should match resources")
    expect((package_matrix.count or 0) >= 20, "package matrix should expose package-focused resources")
    expect(package_matrix.selectedPackage and package_matrix.selectedPackage.id == "gragh-rescue-package", "package matrix should preserve selected package")
    local has_package_baseline = false
    local has_package_class_sweep = false
    local has_package_pressure_sweep = false
    local has_package_skill_buffer = false
    local has_rescue_check = false
    for _, resource in ipairs(package_matrix.resources or {}) do
        if resource.group == "Package baselines" and resource.encounter == "gragh-rescue-package" then
            has_package_baseline = resource.packageSummary and resource.packageSummary.hasRescue
        elseif resource.group == "Selected package class sweep" and resource.className == "Fighter" then
            has_package_class_sweep = resource.packageSummary and resource.packageSummary.hasSummons
        elseif resource.group == "Selected package pressure sweep" and resource.intensity == "brutal" then
            has_package_pressure_sweep = resource.packageSummary and resource.packageSummary.fastestLoopFrames > 0
        elseif resource.group == "Selected package skill buffer" and resource.botProfile == "sloppy" then
            has_package_skill_buffer = resource.controlModel and resource.controlModel.botProfile and resource.controlModel.botProfile.id == "sloppy"
        elseif resource.group == "Rescue package checks" and resource.className == "Necromancer" then
            has_rescue_check = resource.packageSummary and resource.packageSummary.hitsEscorts
        end
    end
    expect(has_package_baseline, "package matrix should include package baselines with summaries")
    expect(has_package_class_sweep, "package matrix should include selected-package class sweeps")
    expect(has_package_pressure_sweep, "package matrix should include selected-package pressure sweeps")
    expect(has_package_skill_buffer, "package matrix should include selected-package skill-buffer sweeps")
    expect(has_rescue_check, "package matrix should include rescue package checks")

    local enemy_options = parse_response("enemy options", ContentApi.enemy_options_json(config))
    expect(enemy_options.enemyGroups and #enemy_options.enemyGroups >= 3, "enemy options should expose enemy groups")
    expect(enemy_options.monsterEvalArgs and enemy_options.monsterEvalArgs[1] == "engine.StartMonsterEvals", "enemy options should expose monster eval args")

    local enemy_matrix = parse_response("enemy matrix", ContentApi.enemy_matrix_json(config))
    expect((enemy_matrix.count or 0) == #(enemy_matrix.resources or {}), "enemy matrix count should match resources")
    expect((enemy_matrix.count or 0) >= 12, "enemy matrix should expose enemy probes")
    local has_gragh_probe = false
    for _, resource in ipairs(enemy_matrix.resources or {}) do
        if resource.monsters == "Gragh" and resource.monsterEvalArgs then
            has_gragh_probe = true
        end
    end
    expect(has_gragh_probe, "enemy matrix should include Gragh monster-eval coverage")

    local analysis_plan = parse_response("analysis plan", ContentApi.analysis_plan_json(config))
    expect(analysis_plan.commands and analysis_plan.commands.classSweep, "analysis plan should expose class sweep args")
    expect(analysis_plan.enemyMatrix and (analysis_plan.enemyMatrix.count or 0) >= 12, "analysis plan should expose enemy resources")

    local workspace = parse_response("workspace", ContentApi.workspace_json(config))
    expect(workspace.runSummary and workspace.runSummary.config.className == "Necromancer", "workspace should expose run summary")
    expect(workspace.loadoutOptions and #(workspace.loadoutOptions.knownSpells or {}) >= 1, "workspace should expose loadout options")
    expect(workspace.enemyOptions and workspace.enemyOptions.monsterEvalArgs, "workspace should expose enemy options")
    expect(workspace.mapGeneration and #(workspace.mapGeneration.previews or {}) >= 3, "workspace should expose map previews")
    expect(workspace.analysisPlan and workspace.analysisPlan.commands and workspace.analysisPlan.commands.classSweep, "workspace should expose analysis commands")
    expect(workspace.commands and workspace.commands.monsterEval, "workspace should expose top-level commands")
    expect(workspace.botProfiles and #(workspace.botProfiles or {}) >= 4, "workspace should expose bot profiles")
    expect(workspace.controlModel and workspace.controlModel.botProfile and workspace.controlModel.botProfile.id == "human", "workspace should expose selected control model")
    expect(workspace.packageMatrix and (workspace.packageMatrix.count or 0) >= 20, "workspace should expose package matrix resources")
    expect(workspace.resourceMatrix and (workspace.resourceMatrix.count or 0) == #(workspace.resourceMatrix.resources or {}), "workspace should expose generated resources")

    print(string.format(
        "Content API smoke complete: %d classes, %d maps, %d encounters, %d generated resources.",
        #(catalog.classes or {}),
        #(catalog.maps or {}),
        #(catalog.encounters or {}),
        matrix.count or 0
    ))
    return nil
end

return run_content_api_smoke
