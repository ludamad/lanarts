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
    expect(#(catalog.maps or {}) >= 4, "catalog should expose map generators")
    expect(#(catalog.map_algorithms or {}) >= 4, "catalog should expose map algorithms")
    expect(#(catalog.encounters or {}) >= 10, "catalog should expose packaged encounters")
    local has_executioner_package = false
    for _, encounter in ipairs(catalog.encounters or {}) do
        if encounter.id == "executioner-blade-package" then
            has_executioner_package = true
        end
    end
    expect(has_executioner_package, "catalog should expose the Executioner blade package")

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
    for _, rhythm in ipairs(summary.rhythms or {}) do
        if rhythm.id == "rescue" then
            has_rescue_rhythm = true
            expect((rhythm.pullFrames or 0) > 0, "rescue rhythm should expose pull frames")
        end
    end
    expect(has_rescue_rhythm, "summary should expose rescue rhythm metadata")
    expect(summary.args and #summary.args > 0, "summary should expose launch args")
    expect(summary.balanceArgs and #summary.balanceArgs > 0, "summary should expose balance args")

    local balance_args = parse_response("balance args", ContentApi.balance_args_json(config))
    expect(balance_args.args and balance_args.args[1] == "engine.StartBalanceRun", "balance args should target StartBalanceRun")
    local has_package_intensity = false
    for _, arg in ipairs(balance_args.args or {}) do
        if arg == "--package_intensity" then
            has_package_intensity = true
        end
    end
    expect(has_package_intensity, "balance args should preserve package intensity")

    local map_config = [[{
        "className": "Fighter",
        "level": 1,
        "preset": "none",
        "seed": "17300",
        "mapAlgorithm": "overworld-broken-grove"
    }]]
    local map_generation = parse_response("map generation", ContentApi.map_generation_json(map_config))
    expect(map_generation.selectedAlgorithm and map_generation.selectedAlgorithm.mapProfile == "broken_grove", "map generation should resolve overworld profile")
    expect(#(map_generation.previews or {}) >= 3, "map generation should expose profile previews")
    local launch_args = parse_response("launch args", ContentApi.launch_args_json(map_config))
    local has_map_profile = false
    for _, arg in ipairs(launch_args.args or {}) do
        if arg == "--map_profile" then
            has_map_profile = true
        end
    end
    expect(has_map_profile, "launch args should preserve map profile")

    local matrix = parse_response("resource matrix", ContentApi.resource_matrix_json(config))
    expect((matrix.count or 0) == #(matrix.resources or {}), "matrix count should match resources")
    expect((matrix.count or 0) >= 12, "matrix should expose orthogonal resources")

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
