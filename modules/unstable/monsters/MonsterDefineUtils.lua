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

local Traits = import ".MonsterTraits"

local M = nilprotect {} -- Submodule

local function default_on_appear_message(self)
    EventLog.add(self.appear_message, self.appear_color or {255,255,255})
end

local function default_on_die_message(self)
    EventLog.add(self.type.defeat_message, self.defeat_color or {255,255,255})
end

local DEFAULT_MELEE_RANGE = 10
function M.monster_define(t, --[[Optional]] derive_idx)
    t.team = t.team or Relations.TEAM_MONSTER_ROOT
    local stats = ContentUtils.resolve_embedded_stats(t)
    local context = StatContext.stat_context_create(stats)
    local sprite = t.sprite or ContentUtils.derive_sprite(t.name)

    for trait in values(t.traits or {}) do
        Traits.stat_mod_functions[trait](stats)
    end

    t.aptitude_types = t.aptitude_types or {Apts.MELEE}
    local action = t.unarmed_action or t
    action.range = action.range or DEFAULT_MELEE_RANGE

    return MonsterType.define {
        name = t.name,
        team = t.team,
        description = t.description:pack(),
        on_first_appear = t.on_first_appear or default_on_appear_message,
        on_die = t.on_die or default_on_die_message,

        sprite = sprite,
        on_draw = t.on_draw,
        traits = t.traits,

        challenge_rating = t.challenge_rating,
        radius = t.radius,

        base_stats = stats,
        unarmed_action = ActionUtils.derive_action(action)
    }
end

function M.animal_define(t, --[[Optional]] derive_idx)
    t.traits = t.traits or {}
    table.insert(t.traits, Traits.ANIMAL)
    return M.monster_define(t, (derive_idx or 1) + 1)
end

return M
