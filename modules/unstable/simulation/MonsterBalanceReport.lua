local MonsterType = import "@MonsterType"
local ClassType = import "@ClassType"
local PlayerObject = import "@objects.PlayerObject"
local RaceType = import "@RaceType"
local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

local function player_configurations(level, race)
    local class_types = {}
    table.insert(class_types, ClassType.lookup("Archer"):on_create {})
    table.insert(class_types, ClassType.lookup("Knight"):on_create {weapon_skill = "Slashing Weapons"})
    for v in values{"Force", "Fire", "Water", "Light", "Dark"} do
        table.insert(class_types, ClassType.lookup("Mage"):on_create {magic_skill = v, weapon_skill = "Slashing Weapons"})
    end

    local stat_list = {}
    for class in values(class_types) do
        local stats = PlayerObject.player_stats_create(RaceType.resolve(race), --[[Can-be-nil]] class, "Tester" .. race)
        table.insert(stat_list,stats)
    end
    return stat_list
end

local function calculate_attack_damage(stats, m)
    
end

local function generate_entry(stats)
    for m in values(MonsterType.list) do
--        StatContext.
    end
end

function M.generate_report(args)
    local calc_ttk = args.calculate_time_to_kill or false
    local file = io.open(assert(args.filename),"w")

    local confs = player_configurations(args.level or 1, args.race or "Human")
    
end

return M