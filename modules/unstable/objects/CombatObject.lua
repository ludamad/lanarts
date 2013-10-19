-- Represents an object that takes actions and 

local Map = import "core.Map"
local StatContext = import "@StatContext"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local ItemTraits = import "@items.ItemTraits"
local ProficiencyPenalties = import "@stats.ProficiencyPenalties"
local AttackResolution = import "@AttackResolution"
local LogUtils = import "lanarts.LogUtils"
local Actions = import "@Actions"
local ActionContext = import "@ActionContext"
local Attacks = import "@Attacks"
local StatUtils = import "@stats.StatUtils"

local CombatObject = ObjectUtils.type_create()

CombatObject.COMBAT_TRAIT = "COMBAT_TRAIT"

function CombatObject.create(args)
    assert(args.base_stats and args.action_resolver)
    args.solid = args.solid or true
    -- Set up type signature
    args.type = args.type or CombatObject
    args.derived_stats = table.deep_clone(args.base_stats)

    args.frame = 0
    args.traits = args.traits or {}
    table.insert(args.traits, CombatObject.COMBAT_TRAIT)

    local ret = args.base_create and args.base_create(args) or CombatObject.base_create( args)
    assert(ret.derived_stats == args.derived_stats)
    StatUtils.stat_context_on_step(ret._context)
    StatContext.on_calculate(ret._context)
    return ret
end

function CombatObject:on_init()
    -- Internal only property. Provides a stat context of unknown validity.
    self._context = StatContext.stat_context_create(self.base_stats, self.derived_stats, self)
    self._stats_need_calculate = false -- Mark as valid stat context
    self.action_resolver:on_object_init(self)
end

function CombatObject:on_deinit()
    self.action_resolver:on_object_deinit(self)
end

function CombatObject:gain_xp(xp)
    ExperienceCalculation.gain_xp(self:stat_context(), xp)
end

-- Only way to get a StatContext.
-- This provides some guarantee of correctness.
function CombatObject:stat_context()
    if self._stats_need_calculate then
        StatContext.on_calculate(self._context)
        self._stats_need_calculate = false
    end
    return self._context
end

-- Shortcut for providing a StatContext copy, needed by eg projectile objects
function CombatObject:stat_context_copy()
    local context = self:stat_context()
    local new_base = StatUtils.stat_full_clone(context.base)
    return { obj = self, base = new_base, derived = StatUtils.stat_clone(new_base) }
end

function CombatObject:on_prestep()
    self.action_resolver:on_prestep(self)
end

function CombatObject:use_resolved_action()
    self.action_resolver:use_resolved_action(self)
end

function CombatObject:on_step()
    StatUtils.stat_context_on_step(self._context)
    self._stats_need_calculate = true
    self.frame = self.frame + 0.1

    self:use_resolved_action()
end

-- Paramater for on_draw
local function drawf(O)
    ObjectUtils.screen_draw(O.sprite, O.xy, O.alpha, O.frame, O.direction, O.color)
end

function CombatObject:on_draw()
    if Map.object_visible(self) then 
        local options = {
            sprite = self.sprite, xy = self.xy, direction = self.direction or 0,
            alpha = self.alpha or 1, frame = self.frame, color = self.color or COL_WHITE
        }
        StatContext.on_draw(self:stat_context(), drawf, options)
    end
end

function CombatObject:weapon_action_context(--[[Optional]] weapon)
    local weapon = weapon or StatContext.get_equipped_item(self._context, ItemTraits.WEAPON)
    local action, source = self.unarmed_action, self.race or self -- Default
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(self._context, weapon)
        source = weapon
        action = ProficiencyPenalties.apply_attack_modifier(weapon.action_wield, modifier)
    end
    return ActionContext.action_context_create(action, self:stat_context(), source)
end

function CombatObject:spell_action_context(spell)
    return ActionContext.action_context_create(spell.action_use, self:stat_context(), spell)
end

function CombatObject:speed()
    return self:stat_context().derived.movement_speed
end

function CombatObject.is_combat_object(obj)
    return table.contains(obj.traits, CombatObject.COMBAT_TRAIT)
end

local status_type_define = (import "@stats.StatusTypeUtils").status_type_define
-- EXHAUSTION
local Hurt = status_type_define {
    name = "Hurt",
    time_limited = true,
    hurt_alpha_value = function()
        local t_left, t_durr = self.time_left, self.total_duration
        if t_left < t_durr /2 then
            return t_left / t_durr / 2 * 0.7 + 0.3
        else
            return (t_durr - t_left) / 10 * 0.7 + 0.3
        end
    end,
    on_draw = function(self, stats, drawf, options, ...)
        local r,g,b,a = unpack(options.color or COL_WHITE)
        local s = 1 - self:hurt_alpha_value()
        options.color = {r, g * s, b * s, a}
        StatContext.on_draw_call_collapse(stats, drawf, options, ...)
    end,
    init = function(self, stats, ...)
        self.base.init(self, stats, ...)
    end
}

local HURT_COOLDOWN_DURATION = 30
function CombatObject:on_hostile_action(action_context)
    local action = action_context.derived
    local atk = Actions.get_effect(action, Attacks.AttackEffect)
    if atk then
        StatContext.update_status(self:stat_context(), Hurt, HURT_COOLDOWN_DURATION)
    end
end

return CombatObject
