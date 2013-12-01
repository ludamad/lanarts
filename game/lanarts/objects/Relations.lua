--- Functions for determining and manipulating object relationships
-- Each team stores information about its relation to other teams.
-- When resolving team relationship, first the relation table is checked, then all of the team's parents are checked against the children,
-- and then finally the default relationship is returned if all queries fail.
-- 
-- With conflicting relationships, the resolution order is NEUTRAL > HOSTILE > FRIENDLY.
-- 
-- The worst-case relationship resolution performance is a bit bad, however it is intended for ~3 depth teams for which it should do fine.

local M = nilprotect {} -- Submodule

--- Relationship constants, part of public API
M.FRIENDLY, M.NEUTRAL, M.HOSTILE = 1, 2, 3

-- Team-to-team relationship queries and operations.
-- Argument order does not matter.

--- Returns a container for a team of CombatObjects. Each CombatObject stores a reference to its team.
function M.team_create(--[[Optional]] default_relationship, --[[Optional]] parent_team)
    return {
        parent_team = parent_team or nil,
        default_relationship = default_relationship or M.HOSTILE,
        -- Relation table is weak to not prevent garbage collection of teams 
        relation_table = setmetatable({}, {__mode = "k"})
    }
end

function M.team_set_relationship(team1, team2, relationship)
    team1.relation_table[team2] = relationship
    team2.relation_table[team1] = relationship
end

local function resolve_asymmetric_relations(relation1, relation2)
    if relation1 == M.NEUTRAL or relation2 == M.NEUTRAL then
        return M.NEUTRAL 
    end
    if relation1 == M.HOSTILE or relation2 == M.HOSTILE then
        return M.HOSTILE
    end
    return M.FRIENDLY
end

-- Query for explicit relationship settings in any parent teams
local function query_chain(team, root)
    while root do
        -- Same-team is friendly
        if team == root then return M.FRIENDLY end
        local relation = team.relation_table[root]
        if relation ~= nil then 
            return relation 
        end
        root = root.parent_team
    end
    return nil
end 

function M.team_get_relationship(team1, team2)
    -- Same-team is friendly
    if team1 == team2 then return M.FRIENDLY end

    local default1, default2 = team1.default_relationship, team2.default_relationship
    while team1 and team2 do
         local result = query_chain(team1, team2)
         if result then return result end
         -- Go up team1's parent chain
         team1 = team1.parent_team

         result = query_chain(team2, team1)
         if result then return result end
         -- Go up team2's parent chain
         team2 = team2.parent_team
    end

    return resolve_asymmetric_relations(default1, default2)
end

--- Determines if an object belongs to a team. Returns true if the team is a parent team.
function M.belongs_to(obj, team)
    local obj_team = obj.team
    while obj_team ~= nil do
        if obj_team == team then
            return true
        end
        obj_team = obj_team.parent_team
    end
    return false
end

-- Object-to-object relationship queries
-- Argument order does not matter

function M.get_relationship(obj1, obj2)
    perf.timing_begin("Relations.get_relationship")
    assert(obj1.team and obj2.team) -- All combat objects have a team
    local relationship = M.team_get_relationship(obj1.team, obj2.team)
    perf.timing_end("Relations.get_relationship")
    return relationship
end

function M.is_hostile(obj1, obj2)
    return M.get_relationship(obj1, obj2) == M.HOSTILE 
end

function M.is_friendly(obj1, obj2)
    return M.get_relationship(obj1, obj2) == M.FRIENDLY 
end

function M.is_neutral(obj1, obj2)
    return M.get_relationship(obj1, obj2) == M.NEUTRAL 
end

M.TEAM_MONSTER_ROOT = data_load("TEAM_MONSTER_ROOT", M.team_create(M.HOSTILE))
M.TEAM_PLAYER_DEFAULT = data_load("TEAM_PLAYER_DEFAULT", M.team_create(M.FRIENDLY))

return M