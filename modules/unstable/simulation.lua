local StatContext = import "@StatContext"
local StatCalc = import "@stat_calc"
local Stats = import "@Stats"

local function choose_option(options)
    local prompt = {}
    for idx, option in ipairs(options) do
        table.insert(prompt, idx .. ') ' .. option)
    end

    print(("\n"):join(prompt))
    local choice
    while not choice do
        local input = io.read("*line")
        choice = options[tonumber(input)]
        if not choice then
            print("Sorry, that is not a valid choice.")
        end
    end

    return choice
end

local function battle(player, enemy)
    local attack = {
        resistance_types = {melee = 1}, 
        effectiveness_types = {melee = 1},
        damage_types = {melee = 1}, 
        defence_types = {melee = 1}
    }

    while enemy.base.hp > 0 do
        local dmg = StatCalc.damage_calc(attack, player, enemy)
        StatContext.hp_add(enemy, -dmg)
    end
end

local player = {
    base_stats = Stats.stats_create {
        hp = 100,
        aptitudes = {
            damage = {
                melee = 1
            }
        }
    }
}
player.derived_stats = table.deep_clone(player.base_stats)

local enemy = table.deep_clone(player)

local scmake = StatContext.game_object_stat_context_create
battle(scmake(player), scmake(enemy))