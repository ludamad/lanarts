local StatContext = import "@StatContext"
local StatCalc = import "@stat_calc"
local Stats = import "@Stats"
local AnsiCol = import "core.terminal.ansi_colors"

local function choose_option(...)
    local options = {...}
    local prompt = {}
    for idx, option in ipairs(options) do
        table.insert(prompt, idx .. ') ' .. option)
    end

    AnsiCol.println(("\n"):join(prompt), AnsiCol.YELLOW, AnsiCol.BOLD)
    local choice
    while not choice do
        local input = io.read("*line")
        choice = options[tonumber(input)]
        if not choice then
            AnsiCol.println("Sorry, that is not a valid choice.", AnsiCol.RED, AnsiCol.BOLD)
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
        
        AnsiCol.println(
            "The " .. enemy.obj.name .. " has " .. enemy.base.hp .. "HP left", 
            AnsiCol.BLUE, AnsiCol.ITALIC .. AnsiCol.BOLD
        )

        choose_option("Attack", "Spell", "Item")

        AnsiCol.println(
            "You strike for " .. dmg .. " damage!", 
            AnsiCol.GREEN, AnsiCol.BOLD
        )

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
player.name = "You"

local enemy = table.deep_clone(player)
enemy.name = "Wild Gazebo"

local scmake = StatContext.game_object_stat_context_create
battle(scmake(player), scmake(enemy))