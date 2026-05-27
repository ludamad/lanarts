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
    expect(#(catalog.encounters or {}) >= 4, "catalog should expose encounters")

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
        end
    end
    expect(has_rescue_rhythm, "summary should expose rescue rhythm metadata")
    expect(summary.args and #summary.args > 0, "summary should expose launch args")

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
