--- Provides implementations of stat contexts.
-- Conceptually, a stat context is any object which has the fields (or getters) 'base' and 'derived'
-- Optionally, 'obj' should be provided for operations that alter 

local M = nilprotect {} -- Submodule

-- Context creation                                                                --

--- Simply forwards the 'base' and 'derived' parts of the game object
local GameObjectStatContext = newtype()
function GameObjectStatContext:init(obj)     
	self.obj = obj
end

function GameObjectStatContext.get:base()
	return self.obj.base_stats
end

function GameObjectStatContext.get:derived() 
    return self.obj.derived_stats
end

M.game_object_stat_context_create = GameObjectStatContext.create 

-- Utility creation

--- Sync derived stats with base stats
function M.copy_base_to_derived(context)
    table.deep_copy(context.derived, context.base)
end

--- Change HP & reflect it in both derived and base stat-sets
-- Ensures that 0 <= hp <= max_hp
function M.hp_add(context, hp)
	local A, B = context.base, context.derived
	A.hp = math.min(math.max(0, A.hp + hp), A.max_hp)
	B.hp = math.min(math.max(0, B.hp + hp), B.max_hp)
end

--- Change MP & reflect it in both derived and base stat-sets
-- Ensures that 0 <= mp <= max_mp
function M.mp_add(context, mp)
    local A, B = context.base, context.derived
    A.mp = math.min(math.max(0, A.mp + mp), A.max_mp)
    B.mp = math.min(math.max(0, B.mp + mp), B.max_mp)
end

--- Change a stat temporarily. Adds to derived context only.
function M.temporary_add(context, values)
    table.defaulted_addition(context.derived, values)
end

--- Change a stat permanently. Adds to both derived and base contexts.
function M.permanent_add(context, values)
    table.defaulted_addition(context.base, values)
    M.temporary_add(context, values)
end

--- Change an aptitude temporarily.
local function aptitude_add(context, category, type, amount )
    StatContext.temporary_add(context, {
        aptitudes = {
            [category] = {
                [type] = amount
            } 
        }
    })
end

--- Change an effectiveness aptitude temporarily. 
function M.effectiveness_add(context, type, amount)
    aptitude_add(context, "effectiveness", type, amount)
end

--- Change a damage aptitude temporarily. 
function M.damage_add(context, type, amount)
    aptitude_add(context, "damage", type, amount)
end

--- Change a resistance aptitude temporarily. 
function M.resistance_add(context, type, amount)
    aptitude_add(context, "resistance", type, amount)
end

--- Change a defence aptitude temporarily. 
function M.defence_add(context, type, amount)
    aptitude_add(context, "defence", type, amount)
end

return M