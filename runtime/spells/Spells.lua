local EventLog = require "ui.EventLog"
local GameObject = require "core.GameObject"
local Map = require "core.Map"
local Bresenham = require "core.Bresenham"
local SpellObjects = require "objects.SpellObjects"

-- Effects:
require("spells.Effects")

-- Projectiles:
require("spells.Projectiles")

-- New spells coded in more convenient syntax:
-- TODO figure out long term plan
require("spells.Spells2")

-- REGENERATION

local Regeneration = {
    name = "Regeneration",
    spr_spell = "spr_spells.regeneration",
    description = "Regenerate health at 15x speed for two seconds.",
    mp_cost = 25,
    cooldown = 40,
    can_cast_with_held_key = false,
    fallback_to_melee = false,
    spell_cooldown = 1600
}

function Regeneration.prereq_func(caster)
    return not caster:has_effect("Regeneration") and not caster:has_effect("Exhausted")
end

function Regeneration.autotarget_func(caster)
    return caster.x, caster.y
end

function Regeneration.action_func(caster, x, y)
    caster:add_effect("Regeneration", 60 * 2)
    if caster:is_local_player() then
        EventLog.add("You start to regenerate quickly!", {200,200,255})
    else
        EventLog.add(caster.name .. " starts to regenerate quickly!", {200,200,255})
    end
end

Data.spell_create(Regeneration)


-- BERSERK

local Berserk = {
    name = "Berserk",
    spr_spell = "berserk",
    description = "Initiate frenzy, gaining +2 defence, +5 willpower, +{Level} strength, +60% melee speed, +25% move speed. Killing enemies grants you longer frenzy, and heals a bit every kill. Afterwards, become exhausted with -3 defence, -3 willpower, -25% action speed, -50% move speed.",
    mp_cost = 40,
    cooldown = 30,
    spell_cooldown = 400,
    can_cast_with_held_key = false,
    fallback_to_melee = false
}

function Berserk.prereq_func(caster)
    return not caster:has_effect("Berserk") and not caster:has_effect("Exhausted") and not caster:has_effect("Ice Form")
end

function Berserk.autotarget_func(caster)
    return caster.x, caster.y
end

function Berserk.action_func(caster, x, y)
    caster:add_effect("Berserk", 130 + math.min(3, caster.stats.level) * 30)
    if caster:is_local_player() then
        play_sound "sound/berserk.ogg"
        EventLog.add("You enter a powerful rage!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " enters a powerful rage!", {200,200,255})
    end
end

Data.spell_create(Berserk)

-- BLINK

local Blink = {
    name = "Blink",
    spr_spell = "blink",
    description = "Takes you instantly across a short distance.",
    mp_cost = 50,
    cooldown = 50,
    autotarget_func = spell_choose_safest_square,
    can_cast_with_held_key = false,
    can_cast_with_cooldown = true,
    fallback_to_melee = false
}

function Blink.prereq_func(caster, x, y)
    local xy = {x,y}
    return Map.radius_visible(xy, caster.radius) and not Map.object_collision_check(caster, xy)
end

function Blink.action_func(caster, x, y)
    caster.xy = {x,y}
end

Data.spell_create(Blink)

-- POWER STRIKE

local PowerStrike = {
    name = "Power Strike",
    description = "A powerful charged strike. Deals stunning blows to all surrounding enemies, knocking them back.",
    can_cast_with_held_key = true,
    spr_spell = "chargestrike",
    can_cast_with_cooldown = false,
    mp_cost = 40,
    spell_cooldown = 800,
    cooldown = 0, -- Uses cooldown of weapon, favours 40 cooldown weapons
    fallback_to_melee = true,
}

local function ChargeCallback(_, caster)
    local num = 0
    for mon in values(Map.enemies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + 30 + caster.stats.strength then
            num = num + 1
            caster:melee(mon)
            --            local chance = math.max(25, 100 - num * 20)
            --if rand_range(0, 100) < chance then -- decreasing chance of knockback
            local str_diff = math.max(0, caster.stats.strength - mon.stats.strength)
                local thrown = mon:add_effect("Thrown", 10 + 10 * str_diff)
                thrown.angle = vector_direction(caster.xy, mon.xy)
                if caster:is_local_player() then
                    EventLog.add("The " .. mon.name .." is thrown back!", {200,200,255})
                end
            --end
        end
    end
end

function PowerStrike.action_func(caster)
    caster:apply_melee_cooldown()
    if caster:is_local_player() then
        EventLog.add("You strike wildly in all directions!", {200,200,255})
        play_sound "sound/knockback.ogg"
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " strikes wildly in all directions!", {200,200,255})
        play_sound "sound/knockback.ogg"
    end

    caster:add_effect("Charge", 8).callback = ChargeCallback
end

function PowerStrike.prereq_func(caster)
    if caster:has_ranged_weapon() then
        return false
    end
    for mon in values(Map.enemies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + 30 then
            return true
        end
    end
    return false
end

function PowerStrike.autotarget_func(caster)
    return caster.x, caster.y
end

Data.spell_create(PowerStrike)

-- PAIN --

local Pain = {
    name = "Pain",
    description = "Instantly damage a nearby enemy, but hurt yourself in the process. If you kill the enemy, you gain back one third its life.",
    can_cast_with_held_key = true,
    spr_spell = "spr_spells.pain",
    can_cast_with_cooldown = false,
    mp_cost = 0,
    cooldown = 30,
    fallback_to_melee = true,
    range = 50
}

function Pain.action_func(caster, x, y, target)
    local num = 0
    local eff_range = Pain.range
    if not target or vector_distance({target.x, target.y}, {caster.x, caster.y}) > target.target_radius + eff_range then
        local least_dist = math.huge
        for mon in values(Map.enemies_list(caster)) do
            local dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
            if dist < mon.target_radius + eff_range and least_dist > dist then
                least_dist = dist
                target = mon
            end
        end
    end
    if not target then
        return
    end
    local stats = caster:effective_stats()
    target:add_effect("Pained", 50)
    local aura = caster:add_effect("Pain Aura", 100)
    aura.animation_only = true
    aura.range = eff_range
    play_pained_sound()
    if target:damage(random(1,5) + stats.magic, random(2,5) + stats.magic * 0.3, 1, 0.9) then
        play_sound "sound/painkill.ogg"
        caster:gain_xp_from(target)
        if caster:has_effect("AmuletGreatPain") then
            caster:heal_hp(target:effective_stats().max_hp * 2 / 3)
        else
            caster:heal_hp(target:effective_stats().max_hp / 3)
        end
    else
        caster:direct_damage(20)
    end
    caster:add_effect("Pained", 50)
    if caster:is_local_player() then
        EventLog.add("You attack your enemy's life force directly!", {200,200,255})
    else
        EventLog.add(caster.name .. " attacks their enemy's life force directly!", {200,200,255})
    end
end

function Pain.prereq_func(caster)
    if caster.stats.hp < 35 then
        if caster:is_local_player() then
            EventLog.add("You require more health!", {200,200,255})
        end
        return false
    end
    for mon in values(Map.enemies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + Pain.range then
            return true
        end
    end
    return false
end

function Pain.autotarget_func(caster)
    return caster.x, caster.y
end

Data.spell_create(Pain)

-- HEAL AURA --

local HealAura = {
    name = "Healing Aura",
    description = "Greatly boosts the healing of nearby allies.",
    can_cast_with_held_key = false,
    spr_spell = "spr_amulets.healing",
    can_cast_with_cooldown = false,
    mp_cost = 25,
    cooldown = 25,
    spell_cooldown = 800,
    fallback_to_melee = false,
    range = 80
}

function HealAura.action_func(caster, x, y, target)
    local stats = caster:effective_stats()
    caster:add_effect("Healing Aura", 100).range = HealAura.range
    if caster:is_local_player() then
        EventLog.add("You release a healing radiance!", {200,200,255})
    else
        EventLog.add(caster.name .. " releases a healing radiance!", {200,200,255})
    end
end

function HealAura.prereq_func(caster)
    for mon in values(Map.allies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + HealAura.range then
            return true
        end
    end
    EventLog.add("No allies in sight!", COL_PALE_RED)
    return false
end

HealAura.autotarget_func = Pain.autotarget_func

Data.spell_create(HealAura)

-- DAZE AURA --

local Luminos = {
    name = "Luminos",
    description = "Dazes nearby enemies.",
    can_cast_with_held_key = false,
    spr_spell = "spr_amulets.light",
    can_cast_with_cooldown = false,
    mp_cost = 0,
    cooldown = 25,
    spell_cooldown = 1600,
    fallback_to_melee = false,
    range = 80
}

function Luminos.action_func(caster, x, y, target)
    local stats = caster:effective_stats()
    caster:add_effect("Daze Aura", 200).range = Luminos.range
    if caster:is_local_player() then
        EventLog.add("You daze nearby enemies!", {200,200,255})
    else
        EventLog.add(caster.name .. " dazes nearby enemies!", {200,200,255})
    end
end

function Luminos.prereq_func(caster)
    for mon in values(Map.enemies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + Luminos.range then
            return true
        end
    end
    return false
end

Luminos.autotarget_func = Pain.autotarget_func

Data.spell_create(Luminos)


-- GREATER PAIN --

local GreaterPain = {
    name = "Greater Pain",
    description = "Instantly damage all nearby enemies, but hurting yourself in the process. Killing enemies brings back some health.",
    can_cast_with_held_key = true,
    spr_spell = "spr_spells.greaterpain",
    can_cast_with_cooldown = false,
    mp_cost = 0,
    cooldown = 25,
    spell_cooldown = 800,
    fallback_to_melee = true,
    range = 65
}

function GreaterPain.action_func(caster, x, y, target)
    local stats = caster:effective_stats()
    caster:direct_damage(40)
    caster:add_effect("Pained", 50)
    caster:add_effect("Pain Aura", 100).range = GreaterPain.range + caster.stats.level * 5
    if caster:is_local_player() then
        EventLog.add("You attack nearby enemies life force directly!", {200,200,255})
    else
        EventLog.add(caster.name .. " attacks nearby enemies life force directly!", {200,200,255})
    end
end

function GreaterPain.prereq_func(caster)
    if caster.stats.hp < 55 then
        if caster:is_local_player() then
            EventLog.add("You do not have enough health!", {255,200,200})
        end
        return false
    end
    for mon in values(Map.enemies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + Pain.range then
            return true
        end
    end
    return false
end

GreaterPain.autotarget_func = Pain.autotarget_func

Data.spell_create(GreaterPain)

-- FEAR STRIKE --

local FearStrike = {
    name = "Fear Strike",
    description = "Strike an enemy, magically instilling the fear of death within them.",
    can_cast_with_held_key = true,
    spr_spell = "spr_spells.fear_strike",
    can_cast_with_cooldown = false,
    mp_cost = 5,
    cooldown = 0, -- Uses cooldown of weapon
    spell_cooldown = 100,
    fallback_to_melee = true,
}

function FearStrike.action_func(caster, x, y)
    caster:apply_melee_cooldown()
    local num = 0
    local closest_mon, least_dist = nil, math.huge
    for mon in values(Map.enemies_list(caster)) do
        local dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
        if dist < mon.target_radius + caster.target_radius + caster.weapon_range + 8 and least_dist > dist then
            least_dist = dist
            closest_mon = mon
        end
    end
    if not closest_mon then
        return
    end
    caster:melee(closest_mon)
    closest_mon:add_effect("Fear", 100)
    if caster:is_local_player() then
        EventLog.add("You strike into the soul of your enemy!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " strikes into the soul of their enemy!", {200,200,255})
    end
end

function FearStrike.prereq_func(caster)
    if caster:has_ranged_weapon() then
        return false
    end
    for mon in values(Map.enemies_list(caster)) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + caster.weapon_range + 8 then
            return true
        end
    end
    return false
end

function FearStrike.autotarget_func(caster)
    return caster.x, caster.y
end

Data.spell_create(FearStrike)

-- EXPEDITE --

local Expedite = {
    name = "Expedite",
    description = "Run 25% faster for a short duration, with 50% faster rate of fire.",
    can_cast_with_held_key = false,
    spr_spell = "expedite",
    can_cast_with_cooldown = false,
    mp_cost = 0,
    cooldown = 30,
    spell_cooldown = 1600,
    fallback_to_melee = false,
}

function Expedite.prereq_func(caster)
    return not caster:has_effect("Expedited")
end

function Expedite.autotarget_func(caster)
    return caster.x, caster.y
end

function Expedite.action_func(caster, x, y)
    caster:add_effect("Expedited", 600)
    if caster:is_local_player() then
        EventLog.add("You feel expedient!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " seems expedient.", {200,200,255})
    end
end

Data.spell_create(Expedite)

-- CallSpikes --

local CallSpikes = {
    name = "Call Spikes",
    description = "Calls forth spikes.",
    can_cast_with_held_key = false,
    spr_spell = "spr_spells.arrow",
    can_cast_with_cooldown = false,
    mp_cost = 40,
    cooldown = 45,
    spell_cooldown = 1600,
    fallback_to_melee = true,
}

function CallSpikes.prereq_func(caster)
    return true
end

function CallSpikes.autotarget_func(caster)
    if caster.target then
        return caster.target.x, caster.target.y
    end
    return nil
end

function CallSpikes.action_func(caster, x, y)
    play_sound("sounds/slash.ogg")
    if caster:is_local_player() then
        EventLog.add("You create spikes!", {200,200,255})
    else 
        EventLog.add(caster.name .. " creates spikes!", {200,200,255})
    end

    local dx, dy = x - caster.x, y - caster.y
    local mag = math.sqrt(dx*dx+dy*dy)
    dx, dy = dx / mag, dy / mag
    local start = {math.floor(caster.x / 32), math.floor(caster.y / 32)}
    local finish = {math.floor(caster.x / 32 + dx * 7), math.floor(caster.y / 32 + dy * 7)}
    local points = Bresenham.line_evaluate(start, finish)
    for i=1,#points do
        for j=1,2 do
            points[i][j] = points[i][j] * 32 + 16
        end
    end
    local tx, ty = math.round((caster.x+dx*24) / 32) * 32 + 16, math.round((caster.y+dy*24) / 32) * 32 + 16
    for i=2,#points do
        local obj = SpellObjects.SpellSpikes.create { points = points, point_index = i, caster = caster, duration = 500}
        GameObject.add_to_level(obj)
    end
end

Data.spell_create(CallSpikes)

-- Wallanthor --

local Wallanthor = {
    name = "Wallanthor",
    description = "Creates a temporary wall of pure energy in a line.",
    can_cast_with_held_key = false,
    spr_spell = "spr_spells.spell-wall",
    can_cast_with_cooldown = false,
    mp_cost = 10,
    cooldown = 25,
    spell_cooldown = 400,
    fallback_to_melee = false,
}

function Wallanthor.prereq_func(caster)
    return true
end

function Wallanthor.autotarget_func(caster)
    local dx, dy = unpack(caster.last_moved_direction)
    return caster.x + dx * 32, caster.y + dy * 32
end

function Wallanthor.action_func(caster, x, y)
    if caster:is_local_player() then
        EventLog.add("You create a wall of pure energy!", {200,200,255})
    else 
        EventLog.add(caster.name .. " creates a wall of pure energy!", {200,200,255})
    end

    local dx, dy = x - caster.x, y - caster.y
    local mag = math.sqrt(dx*dx+dy*dy)
    dx, dy = dx / mag, dy / mag
    local start = {math.floor(caster.x / 32), math.floor(caster.y / 32)}
    local finish = {math.floor(caster.x / 32 + dx * 7), math.floor(caster.y / 32 + dy * 7)}
    local points = Bresenham.line_evaluate(start, finish)
    for i=1,#points do
        for j=1,2 do
            points[i][j] = points[i][j] * 32 + 16
        end
    end
    local tx, ty = math.round((caster.x+dx*24) / 32) * 32 + 16, math.round((caster.y+dy*24) / 32) * 32 + 16
    for i=2,#points do
        local obj = SpellObjects.SpellWall.create { points = points, point_index = i }
        GameObject.add_to_level(obj)
    end
end

Data.spell_create(Wallanthor)
