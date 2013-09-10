local EventLog = import "core.ui.EventLog"
local Cooldowns = import "@Cooldowns"

--- Provides implementations of stat contexts.
-- Conceptually, a stat context is any object which has the fields (or getters) 'base' and 'derived'
-- Optionally, 'obj' should be provided for operations that alter
-- 'obj' is to safe to leave out if doing stat modifications, however methods like on_step and on_calculate may cause problems. 

local M = nilprotect {} -- Submodule

--------------------------------------
-- Context creation                 --
--------------------------------------

-- Create a stat context, optionally taking an object.
function M.stat_context_create(stats, --[[Optional]] obj)
    return {
        base = stats,
        derived = table.deep_clone(stats),
        obj = obj
    }
end

--------------------------------------
-- Main stat modification/event API --
--------------------------------------

function M.add_spell(stats, ...)
    stats.base.spells:add_spell(...)
end

function M.can_use_spell(stats, spell_slot)
    return stats.base.spells:can_use_spell(stats, spell_slot)
end

function M.use_spell(stats, spell_slot)
    return stats.base.spells:use_spell(stats, spell_slot)
end

function M.add_item(stats, ...)
    return stats.base.inventory:add_item(...)
end

function M.use_item(stats, ...)
    return stats.base.inventory:use_item(...)
end

function M.get_equipped_item(stats, ...)
    return stats.base.inventory:get_equipped_item(...)
end

function M.get_equipped_items(stats, ...)
    return stats.base.inventory:get_equipped_items(...)
end

function M.equip_item(stats, ...)
    return stats.base.inventory:equip_item(...)
end

function M.deequip_item(stats, ...)
    return stats.base.inventory:deequip_item(...)
end

function M.calculate_proficiency_modifier(stats, ...)
    return stats.base.inventory:calculate_proficiency_modifier(stats, ...) 
end

function M.on_step(context)
    if context.base.hp > 0 then
        M.add_hp(context, context.derived.hp_regen)
    end
    M.add_mp(context, context.derived.mp_regen)
    context.base.hooks:merge_new_hooks()
    M.copy_base_to_derived(context)
    context.base.cooldowns:on_step()
    context.base.hooks:on_step(context)
end

function M.on_calculate(context)
    for skill_slot in values(context.derived.skills) do
        skill_slot.type.on_calculate(skill_slot, context)
    end
    context.derived.inventory:on_calculate(context)
    context.derived.hooks:perform("on_calculate", context)
end

--------------------------------------
-- Utility functions                --
--------------------------------------

--- Sync derived stats with base stats
function M.copy_base_to_derived(context)
    table.deep_copy(context.base, context.derived)
end

--- Add an immunity, possibly permanently.
function M.add_immunity(context, type, --[[Optional]] permanent)
    local B, D = context.base, context.derived
    D.immunities[type] = true
    if permanent then
        B.immunities[type] = true
    end
end

--- Change HP & reflect it in both derived and base stat-sets
-- Ensures that 0 <= hp <= max_hp
function M.add_hp(context, hp)
	local B, D = context.base, context.derived
    local orig_hp = D.hp
	-- NOTE: We can have as much hp as the -derived max hp-
	B.hp = math.min(math.max(0, B.hp + hp), D.max_hp)
	D.hp = math.min(math.max(0, D.hp + hp), D.max_hp)
	return D.hp - orig_hp
end

--- Change MP & reflect it in both derived and base stat-sets
-- Ensures that 0 <= mp <= max_mp
function M.add_mp(context, mp)
    local B, D = context.base, context.derived
    local orig_mp = D.mp
	-- NOTE: We can have as much mp as the -derived max mp-
    B.mp = math.min(math.max(0, B.mp + mp), D.max_mp)
    D.mp = math.min(math.max(0, D.mp + mp), D.max_mp)
    return D.mp - orig_mp
end

--- Add a cooldown of a certain cooldown type (permanently)
function M.add_cooldown(context, type, amount)
    local A, B = context.base, context.derived
    A.cooldowns:add_cooldown(type, amount)    
    B.cooldowns:add_cooldown(type, amount)    
end

--- Set a cooldown of a certain cooldown type (permanently)
function M.set_cooldown(context, type, amount)
    local A, B = context.base, context.derived
    A.cooldowns:set_cooldown(type, amount)    
    B.cooldowns:set_cooldown(type, amount)    
end

--- Change a stat temporarily. Adds to derived context only.
function M.temporary_add(context, values)
    table.defaulted_addition(values, context.derived)
end

--- Change a stat permanently. Adds to both derived and base contexts.
function M.permanent_add(context, values)
    table.defaulted_addition(values, context.base)
    M.temporary_add(context, values)
end

--- Change an aptitude temporarily.
local function add_aptitude(context, category, type, amount, --[[Optional, default false]] permanent )
    (permanent and M.permanent_add or M.temporary_add)(context, {
        aptitudes = {
            [category] = {
                [type] = amount
            } 
        }
    })
end

--- Change an effectiveness aptitude, defaults to temporary. 
function M.add_effectiveness(context, type, amount, --[[Optional, default false]] permanent)
    add_aptitude(context, "effectiveness", type, amount, permanent)
end

--- Change a damage aptitude, defaults to temporary. 
function M.add_damage(context, type, amount, --[[Optional, default false]] permanent)
    add_aptitude(context, "damage", type, amount, permanent)
end

--- Change a resistance aptitude, defaults to temporary. 
function M.add_resistance(context, type, amount, --[[Optional, default false]] permanent)
    add_aptitude(context, "resistance", type, amount, permanent)
end

--- Change a defence aptitude, defaults to temporary. 
function M.add_defence(context, type, amount, --[[Optional, default false]] permanent)
    add_aptitude(context, "defence", type, amount, permanent)
end

--- Change resistance & defence aptitude of a certain type, defaults to temporary. 
function M.add_defensive_aptitudes(context, type, amounts, --[[Optional, default false]] permanent)
    if _G.type(amounts) == "number" then amounts = {dup(amounts,2)} end
    M.add_resistance(context, type, amounts[1], permanent)
    M.add_defence(context, type, amounts[2], permanent)
end

--- Change effectiveness & damage aptitude of a certain type, defaults to temporary. 
function M.add_offensive_aptitudes(context, type, amounts, --[[Optional, default false]] permanent)
    if _G.type(amounts) == "number" then amounts = {dup(amounts,2)} end
    M.add_effectiveness(context, type, amounts[1], permanent)
    M.add_damage(context, type, amounts[2], permanent)
end

--- Change all aptitude of a certain type, defaults to temporary. 
function M.add_all_aptitudes(context, type, amounts, --[[Optional, default false]] permanent)
    if _G.type(amounts) == "number" then
        -- Resolve if number
        amounts = {dup(amounts,4)}
    end
    local eff,dam,res,def = unpack(amounts)
    M.add_effectiveness(context, type, eff or 0, permanent)
    M.add_damage(context, type, dam or 0, permanent)
    M.add_resistance(context, type, res or 0, permanent)
    M.add_defence(context, type, def or 0, permanent)
end

function M.multiply_cooldown_multiplier(context, type, multiplier, --[[Optional, default false]] permanent)
    context.derived.cooldowns:multiply_cooldown_multiplier(type, multiplier)
    if permanent then
        context.base.cooldowns:multiply_cooldown_multiplier(type, multiplier)
    end
end

function M.multiply_cooldown_rate(context, type, multiplier, --[[Optional, default false]] permanent)
    context.derived.cooldowns:multiply_cooldown_multiplier(type, multiplier)
    if permanent then
        context.base.cooldowns:multiply_cooldown_multiplier(type, multiplier)
    end
end

return M