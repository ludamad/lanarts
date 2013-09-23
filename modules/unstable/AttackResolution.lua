local LogUtils = import "lanarts.LogUtils"

local M = nilprotect {} -- Submodule

-- Helper for finding highest defence multiplier of a certain type
local function find_highest_multiplier_index(A, DM, type)
    local highest_multiplier, idx = 0, nil
    for i=1,#A do
        local dm = (DM[i][type] or 0)
        if A[i] > 0 and dm > highest_multiplier then
            highest_multiplier, idx = dm, i
        end
    end
    return idx
end

-- Helper for finding the damage potential of a type of damage
local function damage_potential(A, DM, type)
    local sum = 0
    for i=1,#A do
        sum = sum + A[i] * (DM[i][type] or 0)
    end
    return sum
end

-- Helper for finding the best overloaded type to swap with
local function execute_best_overloaded(A, DM, D, DA, type1)
    local best_type, best_trade, idx = nil, 0, nil
    local rA, rD1, rD2 = 0,0,0
    for type2,def2 in pairs(D) do
        local dpot = damage_potential(A, DM, type2) 
        if type1 ~= type2 and dpot > 0 and def2 == 0 then
            for i=1,#A do
                local allocated = (DA[i][type2] or 0)
                local dm1,dm2= DM[i][type1] or 0,DM[i][type2] or 0
                local tradeable = math.min(D[type1] * dm1, allocated * dm2)
                if tradeable > 0 and dm1 > best_trade then
                    idx = i
                    best_type = type2
                    best_trade = dm1
                    rA = tradeable
                    rD1 = tradeable / dm1
                    rD2 = tradeable / dm2
                end
            end
        end
    end
    if idx then
        A[idx] = A[idx] - rA
        D[type1] = D[type1] - rD1
        D[best_type] = D[best_type] + rD2
        DA[idx][best_type] = DA[idx][best_type] - rD2
        return true
    end
    return false
end

-- Allocate defence onto the various attacks such that we minimize damage done
function M.resolve_damage(attack_damages, defence_multipliers, defences)
    local A, DM, D = table.clone(attack_damages), defence_multipliers, table.clone(defences)

    -- Defence allocation
    local DA = {}
    for i=1,#A do
        DA[i] = {}
    end

    -- First greedily 'spend' the defence of each type
    for type,def in pairs(D) do
        while def > 0 do
            local idx = find_highest_multiplier_index(A, DM, type)
            if not idx then
                break
            end
            -- There is somewhere to spend, so spend.
            local dm = DM[idx][type]
            local damage_offset = math.min(def * dm, A[idx])
            A[idx] = A[idx] - damage_offset
            -- Update remaining * allocated defence
            local spent = damage_offset / dm
            def = def - spent
            D[type] = D[type] - spent
            DA[idx][type] = (DA[idx][type] or 0) + spent
        end
    end

    -- Next, do swaps until we cannot decrease damage done anymore
    local changed = true
    while changed do
        changed = false
        for type,def in pairs(D) do
            -- Free, unspent defence
            if def > 0 then
                if execute_best_overloaded(A, DM, D, DA, type) then
                    changed = true
                end
            end
        end
    end

    return vector_sum(A)
end

local RESISTANCE_MULTIPLE_INTERVAL = 20

local function resolve_subattack(s, damage_multiplier, AApt, TApt)
    local resistance = table.dot_product(s.resistance_multipliers, TApt.resistance)
    local defence = table.dot_product(s.defence_multipliers, TApt.defence)
    local effectiveness = s.base_effectiveness + table.dot_product(s.effectiveness_multipliers, AApt.effectiveness)
    local damage = table.dot_product(s.damage_multipliers, AApt.damage)

    local percentage = math.max(0, 1.0 + (effectiveness - resistance) / RESISTANCE_MULTIPLE_INTERVAL)
    local resolved_damage = math.max(0, s.base_damage + (damage - defence) * damage_multiplier)

    LogUtils.debug_log("Damage: ",s.base_damage, " Base Dam + (", damage," Dam vs ", defence, " Def) * ", damage_multiplier*100, "% multiplier => ", resolved_damage, " damage.")
    LogUtils.debug_log("Effectiveness: 100% + (", effectiveness," Eff vs ", resistance, " Res) / ",RESISTANCE_MULTIPLE_INTERVAL," => ", math.ceil(percentage*1000)/10, "%.")

    return resolved_damage, percentage
end

--- Determine the damage done by an attack
-- @param attacker the attacker StatContext
-- @param target the target StatContext
function M.damage_calc(attack, attacker, target)
    local AApt = attacker.derived.aptitudes
    local TApt = target.derived.aptitudes

    local A,DM = {}, {}

    for s in values(attack.sub_attacks) do
        local damage, percentage = resolve_subattack(s, attack.damage_multiplier, AApt, TApt)
        LogUtils.debug_log("Attack hits for ", damage, " with ", math.ceil(percentage*1000)/10 .. "% effectiveness.")
        table.insert(A, damage * percentage)
        table.insert(DM, table.scaled(s.defence_multipliers, percentage))
    end

    return M.resolve_damage(A, DM, TApt.defence)
end

return M