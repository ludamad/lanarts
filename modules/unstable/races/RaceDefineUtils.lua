local Races = import "@Races"

local Stats = import "@Stats"
local StatContext = import "@StatContext"
local ContentUtils = import "@stats.ContentUtils"

local M = nilprotect {} -- Submodule

-- A more convenient race_define
function M.races_define(args)
    args.description = args.description:pack()
    if args.on_create then -- Full custom
        return Races.define(args)
    end

    -- Create based off embedded stats, aptitudes & spells
    local stat_template = ContentUtils.resolve_embedded_stats(args)
    function args.on_create(name, team)
        local stats = Stats.stats_create(stat_template)
        stats.name = name
        stats.team = team
        stats.race = args
        return stats
    end

    return Races.define(args)
end

return M