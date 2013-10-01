local LogUtils = import "lanarts.LogUtils"

local M = nilprotect {} -- Submodule

M.SKILL_POINT_START_AMOUNT = 500
M.SKILL_POINT_COST_RATE = 100

M.EXPERIENCE_EXPONENT = 2.7
M.EXPERIENCE_COST_RATE = 75
M.EXPERIENCE_COST_BASE = 125

local function average_kills_per_level(rating)
    return 10 + 5 * rating
end

function M.challenge_rating_to_xp_gain(user_level, rating)
    local gain = M.level_experience_needed(rating) / average_kills_per_level(rating)
    -- For every CR point lower than user_level - 1, remove 25%
    local multiplier = math.max(0, 1 - (user_level-1 - rating) / 4)
    multiplier = math.min(multiplier, 1.0)
    return multiplier * gain
end

-- xp_level is the current level, and the amount of experience needed for the next level is returned.
function M.level_experience_needed(xp_level)
    return math.round(xp_level ^ M.EXPERIENCE_EXPONENT * M.EXPERIENCE_COST_RATE) + M.EXPERIENCE_COST_BASE
end

local function skill_cost_increment(level)
    return level * M.SKILL_POINT_COST_RATE
end

-- Determines the amount of skill points received at each level-up
function M.skill_points_at_level_up(xp_level)
    return skill_cost_increment(xp_level+1)
end

local function skill_point_cost(multiplier, xp_level)
    return skill_cost_increment(math.floor(xp_level * (xp_level + 1) / 2 * multiplier))
end

function M.cost_from_skill_level(multiplier, xp_level)
    local f = math.floor(xp_level)
    local rem = xp_level - f
    local xp = (1-rem) * skill_point_cost(multiplier, f) + rem * skill_cost_increment(f+1);
    return math.ceil(xp)
end

function M.skill_level_from_cost(multiplier, xp)
    for lvl=0,math.huge do
        local xp_base = skill_point_cost(multiplier, lvl)
        local incr = skill_cost_increment(lvl + 1) 
        if xp_base + incr > xp then
           local true_level = lvl + (xp - xp_base) / incr
           return math.floor(true_level * 10) / 10
        end
    end
    assert(false)
end

function M.level_progress(xp, level)
    local pre_xp_cost = 0
    if level > 1 then 
        pre_xp_cost = M.level_experience_needed(level - 1)
    end
    local xp_cost = M.level_experience_needed(level)
    return (xp - pre_xp_cost) / xp_cost
end

-- Gain skill points in 'SKILL_POINT_INTERVALS' intervals
local SKILL_POINT_INTERVALS = 10

local function amount_needed(xp, level)
    return M.level_experience_needed(level) - xp
end

function M.gain_xp(context, xp)
    local B = context.base

    assert(xp >= 0, "Cannot gain negative experience!")
    local old_hp, old_mp = B.max_hp, B.max_mp
    local old_skill_points, old_level = B.skill_points, B.level

    -- Loop if we have enough XP to levelup.
    -- Guarantees correctness for large XP gains.
    while xp > 0 do
        local old_xp, new_xp = B.xp,B.xp+xp

        local intervals_prev = math.floor(M.level_progress(old_xp, B.level) * SKILL_POINT_INTERVALS)
        local intervals_new = math.floor(M.level_progress(new_xp, B.level) * SKILL_POINT_INTERVALS)
        intervals_new = math.min(intervals_new, SKILL_POINT_INTERVALS)

        -- Gain skill points at regular intervals:
        for i=intervals_prev+1,intervals_new do
            B.skill_points = B.skill_points + M.skill_points_at_level_up(B.level) / SKILL_POINT_INTERVALS 
        end

        local xp_spent = math.min(xp, amount_needed(old_xp, B.level))
        xp = xp - xp_spent
        B.xp = B.xp + xp_spent
 
        if intervals_new == SKILL_POINT_INTERVALS then
            -- Levelup!
            B.level = B.level + 1
        end
    end

    local skill_points_gained, levels_gained = B.skill_points - old_skill_points, B.level - old_level
    if skill_points_gained > 0 then
        LogUtils.event_log_resolved(context.obj, ("<The >$You gain{s} %d skill points!"):format(skill_points_gained), COL_BABY_BLUE)
    end
    if levels_gained > 0 then
        for i=1,levels_gained do
            assert(context.obj.class, "Non-player trying to gain level!")
            context.obj.class:on_level_gain(context)
        end
        local level_str = (levels_gained == 1) and ("a level") or (levels_gained.. " levels")
        LogUtils.event_log_resolved(context.obj, ("<The >$You gain{s} %s! $You {is}[are] now level %d!"):format(level_str, B.level), COL_YELLOW)
    end

    -- Log any changes to MP or HP
    local hp_gained, mp_gained = B.max_hp - old_hp, B.max_mp - old_mp
    if mp_gained > 0 then
        LogUtils.event_log_player(context.obj, "$You gain{s} "..mp_gained.."MP!", COL_BABY_BLUE)
    end
    if hp_gained > 0 then
        LogUtils.event_log_player(context.obj, "$You gain{s} "..hp_gained.."HP!", COL_PALE_GREEN)
    end
end

return M