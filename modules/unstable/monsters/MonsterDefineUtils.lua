local EventLog = import "core.ui.EventLog"

local Apts = import "@stats.AptitudeTypes"
local StatContext = import "@StatContext"
local MonsterType = import "@MonsterType"
local Stats = import "@Stats"
local Attacks = import "@Attacks"

local Relations = import "lanarts.objects.Relations"
local AptitudeTypes = import "@stats.AptitudeTypes"
local ContentUtils = import "@stats.ContentUtils"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local ActionUtils = import "@stats.ActionUtils"

local M = nilprotect {} -- Submodule

M.stat_mod_functions = {
    ANIMAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_defensive_aptitudes(context, Apts.SLASHING, -2, --[[Permanent]] true)
    end,
    WATER_ELEMENTAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_defensive_aptitudes(context, Apts.FIRE, -3, --[[Permanent]] true)
    end,
    AIR_ELEMENTAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_defensive_aptitudes(context, Apts.EARTH, -3, --[[Permanent]] true)
    end,
    EARTH_ELEMENTAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_defensive_aptitudes(context, Apts.AIR, -3, --[[Permanent]] true)
    end,
    FIRE_ELEMENTAL = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_defensive_aptitudes(context, Apts.WATER, -3, --[[Permanent]] true)
    end,
    UNDEAD = function(stats)
        local context = StatContext.stat_context_create(stats)
        StatContext.add_defensive_aptitudes(context, Apts.POISON, 20, --[[Permanent]] true)
        StatContext.add_all_aptitudes(context, Apts.DARK, 2, --[[Permanent]] true)
        StatContext.add_all_aptitudes(context, Apts.LIGHT, -2, --[[Permanent]] true)
    end,
    HUMANOID = do_nothing,
}

-- Export keys as traits
for k,v in pairs(M.stat_mod_functions) do
    M[k] = k
end

local function default_on_appear_message(self)
    EventLog.add(self.appear_message, self.appear_color or {255,255,255})
end

local function default_on_die_message(self)
    EventLog.add(self.type.defeat_message, self.defeat_color or {255,255,255})
end

local DEFAULT_MELEE_RANGE = 10
function M.monster_define(t)
    local stats = ContentUtils.resolve_embedded_stats(t)
    local context = StatContext.stat_context_create(stats)

    t.team = t.team or Relations.TEAM_MONSTER_ROOT
    t.sprite = ContentUtils.resolve_sprite(t, --[[Absolute paths]] false, --[[Assume animation]] true)
    t.traits = t.traits or {}
    if t.damage then
        t.aptitude_types = t.aptitude_types or {Apts.MELEE}
    end
    if t.monster_kind then
        table.insert(t.traits, t.monster_kind)
        M.stat_mod_functions[t.monster_kind](stats)
    end

    local action = t.unarmed_action or t
    action.range = action.range or DEFAULT_MELEE_RANGE

    return MonsterType.define {
        name = t.name,
        team = t.team,
        description = t.description:pack(),
        on_first_appear = t.on_first_appear or default_on_appear_message,
        on_die = t.on_die or default_on_die_message,

        sprite = t.sprite,
        on_draw = t.on_draw,
        traits = t.traits,

        challenge_rating = t.challenge_rating,
        radius = t.radius,

        base_stats = stats,
        unarmed_action = ActionUtils.derive_action(action)
    }
end

return M