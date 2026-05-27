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

local function normalized_config(config)
    config = config or {}
    return {
        className = tostring(config.className or config.class or "Fighter"),
        level = math.max(1, math.floor(tonumber(config.level) or 1)),
        seed = tostring(config.seed or "1"),
        preset = tostring(config.preset or config.loadout_preset or "none"),
        intensity = tostring(config.intensity or config.packageIntensity or config.package_intensity or "standard"),
        customLoadout = tostring(config.customLoadout or config.loadout or ""),
        mapDesc = tostring(config.mapDesc or config.start_map_desc or ""),
        encounter = tostring(config.encounter or ""),
        monsters = tostring(config.monsters or ""),
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
    end
    if config.seed ~= "" then
        args[#args + 1] = "--seed"
        args[#args + 1] = config.seed
    end
    return args
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

function M.map_preview_json(raw_json)
    local config = normalized_config(parse_json_object(raw_json))
    local selected = map_lookup()[config.mapDesc or ""]
    local encounter = encounter_lookup()[config.encounter or ""]
    local previews = {}
    for _, map in ipairs(Catalog.maps) do
        if map.id == "" or map.id == config.mapDesc or #previews < 4 then
            previews[#previews + 1] = map
        end
    end
    return encode({
        schema = 1,
        seed = config.seed,
        selected = selected or Catalog.maps[1],
        encounter = encounter,
        previews = previews,
    })
end

return M
