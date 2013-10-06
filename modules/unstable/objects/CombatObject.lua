-- Represents an object that takes actions and 

local GameMap = import "core.GameMap"
local StatContext = import "@StatContext"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
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
    assert(args.base_stats and args.action_resolver)
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
    return { obj = self, base = table.deep_clone(context.base), derived = table.deep_clone(context.derived) }
end

function CombatObject:on_prestep()
    self.action_resolver:on_prestep(self)
end

function CombatObject:use_resolved_action()
    self.action_resolver:use_resolved_action(self)
    if not R then return end

    local new_xy = {self.x + (R.vx or 0), self.y + (R.vy or 0)}
    if not GameMap.object_solid_check(self, new_xy) then
        self.xy = new_xy
    end
    if R.action then
        self:use_action(R.action, R.target, R.source)
    end

    return R
end

function CombatObject:on_step()
    local cgroup = self.collision_avoidance_group
    StatContext.on_step(self._context)
    self._stats_need_calculate = true

    local close_to_wall = GameMap.radius_tile_check(self.xy, self.radius + 10)
    local prev_xy = self.xy
    if not close_to_wall then
        cgroup:object_copy_xy(self.sim_id, self)
    end

    if close_to_wall then
        local dx,dy = unpack(self.preferred_velocity)
        local new_xy = {self.x+dx,self.y+dy}
        if not GameMap.radius_tile_check(new_xy, self.radius) then
            self.xy = new_xy
        else
            new_xy[1], new_xy[2] = self.x+dx, self.y
            if not GameMap.radius_tile_check(new_xy, self.radius) then self.xy = new_xy
            else
                new_xy[1], new_xy[2] = self.x, self.y+dy
                if not GameMap.radius_tile_check(new_xy, self.radius) then self.xy = new_xy end
            end
        end
    end

    local R = self.action_resolver:resolve(self)
    self:on_resolve_action(R)
end

-- Paramater for on_draw
local function drawf(O)
    ObjectUtils.screen_draw(O.sprite, O.xy, O.alpha, O.frame, O.direction, O.color)
end

function CombatObject:on_draw()
    if GameMap.object_visible(self) then 
        local options = {
            sprite = self.sprite, xy = self.xy, direction = self.direction or 0,
            alpha = self.alpha or 1, frame = self.frame or 0, color = self.color or COL_WHITE
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
