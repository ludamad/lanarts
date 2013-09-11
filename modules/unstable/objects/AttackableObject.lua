local StatContext = import "@StatContext"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local ItemTraits = import "@items.ItemTraits"
local ProficiencyPenalties = import "@stats.ProficiencyPenalties"

local AttackableObject = ObjectUtils.type_create()
AttackableObject.ATTACKABLE_TRAIT = "ATTACKABLE_TRAIT"

function AttackableObject.create(args)
    assert(args.base_stats and (args.can_attack ~= nil))

    -- Set up type signature
    args.type = args.type or AttackableObject
    args.base_stats = args.base_stats -- Retains provided stats object
    args.derived_stats = table.deep_clone(args.base_stats)

    args.traits = args.traits or {}
    table.insert(args.traits, AttackableObject.ATTACKABLE_TRAIT)

    return AttackableObject._base_create(AttackableObject, args)
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

function AttackableObject:on_step()
    StatContext.on_step(self._context)
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

return AttackableObject
