-- Represents an object that takes actions and 

local GameMap = import "core.Map"
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
    if GameMap.object_visible(self) then 
        local options = {
            sprite = self.sprite, xy = self.xy, direction = self.direction or 0,
            alpha = self.alpha or 1, frame = self.frame, color = self.color or COL_WHITE
        }
        StatContext.on_draw(self:stat_context(), drawf, options)
    end
end

function CombatObject:weapon_action()
    local weapon = StatContext.get_equipped_item(self._context, ItemTraits.WEAPON)
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(self._context, weapon)
        return ProficiencyPenalties.apply_attack_modifier(weapon.action_wield, modifier), weapon
    end
    return self.unarmed_action, self.race or self -- Default
end

function CombatObject:weapon_action_context()
    local action, source = self:weapon_action()
    return ActionContext.action_context_create(action, self:stat_context(), source)
end
 
function CombatObject.is_combat_object(obj)
    return table.contains(obj.traits, CombatObject.COMBAT_TRAIT)
end

function CombatObject:use_action(action, target, source)
    assert(self:can_use_action(action, target, source))
    assert(target)
    assert(source)
    Actions.use_action(self:stat_context(), action, target, source)
end

function CombatObject:can_use_action(action, target, source)
    assert(target)
    assert(source)
    return Actions.can_use_action(self:stat_context(), action, target, source)
end

return CombatObject
