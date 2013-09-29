-- Represents an object that takes actions and 

local StatContext = import "@StatContext"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local ItemTraits = import "@items.ItemTraits"
local ProficiencyPenalties = import "@stats.ProficiencyPenalties"
local AttackResolution = import "@AttackResolution"
local LogUtils = import "lanarts.LogUtils"
local Actions = import "@Actions"
local Attacks = import "@Attacks"

local CombatObject = ObjectUtils.type_create()

CombatObject.COMBAT_TRAIT = "COMBAT_TRAIT"

function CombatObject.create(args)
    assert(args.base_stats)
    args.solid = args.solid or true
    -- Set up type signature
    args.type = args.type or CombatObject
    args.derived_stats = table.deep_clone(args.base_stats)

    args.traits = args.traits or {}
    table.insert(args.traits, CombatObject.COMBAT_TRAIT)

    local ret = args.base_create and args.base_create(args) or CombatObject.base_create( args)
    assert(ret.derived_stats == args.derived_stats)
    StatContext.on_step(ret._context)
    StatContext.on_calculate(ret._context)
    return ret
end

function CombatObject:on_init()
    -- Internal only property. Provides a stat context of unknown validity.
    self._context = StatContext.stat_context_create(self.base_stats, self.derived_stats, self)
    self._stats_need_calculate = false -- Mark as valid stat context
    self.team = self.base_stats.team
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
    return { obj = self, base = table.deep_clone(context.base), derived = table.deep_clone(context.derived) }
end

function CombatObject:on_step()
    StatContext.on_step(self._context)
    self._stats_need_calculate = true
end

CombatObject.on_draw = ObjectUtils.draw_sprite_member_if_seen

function CombatObject:weapon_action()
    local weapon = StatContext.get_equipped_item(self._context, ItemTraits.WEAPON)
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(self._context, weapon)
        return ProficiencyPenalties.apply_attack_modifier(weapon.action_wield, modifier), weapon
    end
    return self.unarmed_action, self.race or self -- Default
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
