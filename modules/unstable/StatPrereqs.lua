--- Define stats and cooldown prerequisites. 
-- These are used to help define actions, which consist of a list of prerequisites and effects. 
-- Prerequisites consist of a check() method, which returns true/false as well as the reason for failure (string).

local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

------ GENERIC STAT PREREQ (>= stat prereq) ------------------------
local StatPrereq = newtype()
local function resolve_stat_prereq(prereqs, derived_stats)
    for k,v in prereqs do
        if type(v) == "table" then
            local is_ok, problem = resolve_stat_prereq(v, derived_stats[k])
            if not is_ok then
                return false, problem
            end
        end
        if derived_stats[k] < v then
            return false, "You do not have enough " .. k .. "." -- TODO Fix for awkward cases
        end
    end
    return true
end
function StatPrereq:init(stat_prereqs) 
    -- Takes mapping stat -> prereq. Can take nested tables.
    self.stat_prereqs = stat_prereqs
end
function StatPrereq:check(user) 
    return resolve_stat_prereq(self.stat_prereqs, user.derived)
end
function M.is_stat_prereq(prereq) -- API
    return getmetatable(prereq) == StatPrereq 
end
M.stat_prereq_create = StatPrereq.create -- API

------ GENERIC COOLDOWN REQUIREMENT (<= 0) ------------------------
local CooldownPrereq = newtype()
function CooldownPrereq:init(cooldown_prereq_list) 
    -- Takes list of required cooldowns
    self.cooldown_prereq_list = cooldown_prereq_list
end
function CooldownPrereq:check(user)
    for c in values(self.cooldown_prereq_list) do
        if StatContext.has_cooldown(user, c) then
            return false, "You must wait before doing that!"
        end
    end
    return true
end
function M.is_cooldown_prereq(prereq) -- API
    return getmetatable(prereq) == CooldownPrereq 
end
M.cooldown_prereq_create = CooldownPrereq.create -- API

------ RANGE REQUIREMENT -------------------------
-- Note: Able to take both xy and obj targets!
local RangePrereq = newtype()
function RangePrereq:init(min_range, --[[Optional]] max_range)
    assert(type(max_range) == 'number') 
    -- Takes list of required cooldowns
    self.min_range = min_range
    self.max_range = max_range or math.huge
end
-- TODO: Evaluate if this should be placed in util. If enough time passes without need just remove this comment.
local function is_position(xy_candidate)
    return type(xy_candidate) == "table" and not getmetatable(xy_candidate) and #xy_candidate == 2
end
function RangePrereq:check(user, target)
    local target_xy, target_radius 
    -- Assumption: Target is either a position or a StatContext
    if is_position(target) then 
        target_xy, target_radius = target, 0
    else 
        target_xy, target_radius = target.obj.xy, target.obj.radius 
    end

    -- Compare only at the tip of the object(s)
    local reach = user.obj.radius + target_radius
    local distance = vector_distance(user.obj.xy, target_xy) - reach
    if distance > self.max_range then
        return false, "You are too close!"
    end 
    if distance < self.min_range then
        return false, "You cannot reach!"
    end

    return true
end
function M.is_range_prereq(prereq) -- API
    return getmetatable(prereq) == RangePrereq 
end
M.range_prereq_create = RangePrereq.create -- API

return M