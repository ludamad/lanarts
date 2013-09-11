local Races = import "@Races"

local Spells = import "@Spells"
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

    -- Resolve embedded spell definitions
    -- TODO: Make most of these abilities
    if args.starting_spells then
        for i,spell in ipairs(args.starting_spells) do
            if type(spell) == "table" and not spell.id then
                args.starting_spells[i] = Spells.define(spell)
            end 
        end
    end

    -- Create based off embedded, embedded aptitudes, and starting_spells
    local aptitudes = ContentUtils.resolve_aptitude_bonuses(args)
    function args.on_create(name, team)
        local stats = Stats.stats_create(args)
        stats.name = name
        stats.team = team
        stats.race = args
        if args.starting_spells then
            local context = StatContext.stat_context_create(stats)
            for spell in values(args.starting_spells) do
                StatContext.add_spell(context, spell)
            end
        end
        table.defaulted_addition(aptitudes, stats.aptitudes)
        return stats
    end

    return Races.define(args)
end

return M