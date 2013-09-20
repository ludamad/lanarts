local StatContext = import "@StatContext"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local ItemTraits = import "@items.ItemTraits"
local ProficiencyPenalties = import "@stats.ProficiencyPenalties"
local AttackResolution = import "@AttackResolution"
local LogUtils = import "lanarts.LogUtils"

local AttackableObject = ObjectUtils.type_create()
AttackableObject.ATTACKABLE_TRAIT = "ATTACKABLE_TRAIT"

function AttackableObject.create(args)
    assert(args.base_stats and (args.can_attack ~= nil))
    args.solid = true

    -- Set up type signature
    args.type = args.type or AttackableObject
    args.derived_stats = table.deep_clone(args.base_stats)

    args.traits = args.traits or {}
    table.insert(args.traits, AttackableObject.ATTACKABLE_TRAIT)

    local ret = args.base_create and args.base_create(args) or AttackableObject.base_create( args)
    assert(ret.derived_stats == args.derived_stats)
    StatContext.on_step(ret._context)
    StatContext.on_calculate(ret._context)
    return ret
end

function AttackableObject:on_init()
    -- Internal only property. Provides a stat context of unknown validity.
    self._context = StatContext.stat_context_create(self.base_stats, self.derived_stats, self)
    self._stats_need_calculate = false -- Mark as valid stat context
end

-- Only way to get a StatContext.
-- This provides some guarantee of correctness.
function AttackableObject:stat_context()
    if self._stats_need_calculate then
        StatContext.on_calculate(self._context)
        self._stats_need_calculate = false
    end
    return self._context
end

-- Shortcut for providing a StatContext copy, needed by eg projectile objects
function AttackableObject:stat_context_copy()
    local context = self:stat_context()
    return { obj = self, base = table.deep_clone(context.base), derived = table.deep_clone(context.derived) }
end

function AttackableObject:on_step()
    StatContext.on_step(self._context)
    self._stats_need_calculate = true
end

function AttackableObject:melee_attack()
    if not self.can_attack then return nil end

    local weapon = StatContext.get_equipped_item(self._context, ItemTraits.WEAPON)
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(self._context, weapon)
        return ProficiencyPenalties.apply_attack_modifier(weapon.type.attack, modifier)
    end
    return self.unarmed_attack -- Default
end

function AttackableObject.is_attackable(obj)
    return table.contains(obj.traits, AttackableObject.ATTACKABLE_TRAIT)
end

-- NOTE: Bypassing on_prerequisite
function AttackableObject:apply_attack(attack, target_obj)
    local attacker, target = self:stat_context(), target_obj:stat_context()

    local dmg = attack:on_use(attacker, target)
    LogUtils.resolved_log(attacker.obj, "{The }$You deal{s} " ..dmg .. " damage!", COL_GREEN)
    LogUtils.resolved_log(target.obj, "{The }$You [have]{has} " .. math.ceil(target.base.hp) .. "HP left.", COL_PALE_BLUE)
end

return AttackableObject