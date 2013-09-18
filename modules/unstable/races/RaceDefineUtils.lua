local RaceType = import "@RaceType"

local Apts = import "@stats.AptitudeTypes"
local Stats = import "@Stats"
local StatContext = import "@StatContext"
local ContentUtils = import "@stats.ContentUtils"
local CooldownUtils = import "@stats.CooldownUtils"

local M = nilprotect {} -- Submodule

-- A more convenient race_define
function M.races_define(args)
    args.description = args.description:pack()
    if args.on_create then -- Full custom
        return RaceType.define(args)
    end

    args.types = args.types or {Apts.BLUNT, Apts.MELEE}
    args.attack = CooldownUtils.derive_attack_with_cooldown(args)
    -- Create based off embedded stats, aptitudes & spells
    local stat_template = ContentUtils.resolve_embedded_stats(args)
    function args.on_create(name, team)
        local stats = Stats.stats_create(stat_template)
        stats.name = name
        stats.team = team
        stats.race = args
        return stats
    end

    return RaceType.define(args)
end

return M