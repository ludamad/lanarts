local StatContext = import "@StatContext"
local AttackResolution = import "@AttackResolution"
local Attacks = import "@Attacks"
local Stats = import "@Stats"
local AnsiCol = import "core.terminal.AnsiColors"
local Races = import "@Races"
local ItemType = import "@ItemType"
local SkillType = import "@SkillType"
local Spells = import "@Spells"
local ExperienceCalculation = import "@stats.ExperienceCalculation"

local Relations = import "lanarts.objects.Relations"
local Apts = import "@stats.AptitudeTypes"
local StatUtils = import "@stats.StatUtils"
local ItemTraits = import "@items.ItemTraits"
local ItemProficiency = import "@items.ItemProficiency"
local LogUtils = import "lanarts.LogUtils"

local function choose_option(...)
    local options = {...}
    local prompt = {}
    for idx, option in ipairs(options) do
        table.insert(prompt, AnsiCol.WHITE(idx .. ') ', AnsiCol.BOLD)  .. AnsiCol.YELLOW(option, AnsiCol.BOLD))
    end

    print(("\n"):join(prompt))
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

-- Returns whether performed the spell
local function perform_spell(player, enemy)
    local spells = player.base.spells
    local choices = {"Back"}
    local choice2spell = {}
    for spell in spells:values() do
        table.insert(choices, spell.name)
        choice2spell[spell.name] = spell
    end

    local spell = choice2spell[choose_option(unpack(choices))]
    if not spell then return false end

    local used = StatContext.use_spell(player, spell)
    if not used then
        print("You must wait before using this.")
    end

    return used
end

local function choose_skills(player, xp_to_spend)
    while xp_to_spend > 0 do
        -- Pick skill
        AnsiCol.println("You have "..xp_to_spend.."XP to spend.", AnsiCol.GREEN, AnsiCol.BOLD)
        local choices = {}
        local choice2skill = {}
        for skill in values(SkillType.list) do
            table.insert(choices, skill.name)
            choice2skill[skill.name] = skill
        end
        table.insert(choices, "Finish")
    
        local skill = choice2skill[choose_option(unpack(choices))]
        if not skill then break end

        -- Pick experience gain
        while true do
            AnsiCol.println("You have "..xp_to_spend.."XP to spend.", AnsiCol.GREEN, AnsiCol.BOLD)
            AnsiCol.println(
                ("Spend how much on %s%s"):format(
                    AnsiCol.WHITE(skill.name, AnsiCol.BOLD), 
                    AnsiCol.GREEN('?', AnsiCol.BOLD)), 
                AnsiCol.GREEN, AnsiCol.BOLD
            )
            local skill_slot = Stats.get_skill(player.base, skill)
    
            local function xp2level(xp)
                return ExperienceCalculation.skill_level_from_experience(skill.experience_cost_multiplier, xp)
            end
            local function level2xp(xp_level)
                return ExperienceCalculation.experience_from_skill_level(skill.experience_cost_multiplier, xp_level)
            end
            local function needed(gain)
                return level2xp(skill_slot.level + gain) - skill_slot.experience 
            end
    
            local max = xp2level(skill_slot.experience + xp_to_spend)
            local amounts = {
                ["Max (Spend ".. xp_to_spend .. " to become "..max..")"] = xp_to_spend
            }
            if needed(1.0) <= xp_to_spend then
                amounts[("+1 (Spend %s to become %s)"):format(needed(1.0), skill_slot.level+1)] = needed(1.0)
            end
            if needed(0.1) <= xp_to_spend then
                amounts[("+0.1 (Spend %s to become %s)"):format(needed(0.1), skill_slot.level+0.1)] = needed(0.1)
            end
            local choices = {"Back"}
            for k,v in pairs(amounts) do table.insert(choices, k) end
            local amount = amounts[choose_option(unpack(choices))]
            if not amount then break end

            xp_to_spend = xp_to_spend - amount
            skill_slot.experience = skill_slot.experience + amount
            skill_slot.level = xp2level(skill_slot.experience)
        end
    end
end

-- Returns whether used item
local function use_item(player)
    local equip_msg = AnsiCol.BLUE(" [Equipped]", AnsiCol.BOLD)

    local inventory = player.base.inventory
    local choices = {"Back"}
    local choice2item = {}
    for item in inventory:values() do
        local entry = item.name .. (item.equipped and equip_msg or "")
        table.insert(choices, entry)
        choice2item[entry] = item
    end

    local item = choice2item[choose_option(unpack(choices))]
    if not item then return false end

    inventory:use_item(player, item)
    return true
end

local function get_attack(attacker)
    local weapon = StatContext.get_equipped_item(attacker, ItemTraits.WEAPON)
    if weapon then
        local proficiency = ItemProficiency.calculate_proficiency(weapon.type.proficiency_requirements[1], attacker)
        print("Proficiency", proficiency) 
    end
    return (weapon ~= nil) and weapon.type.attack or attacker.obj.unarmed_attack
end

local function damage(attacker, target)

    local dmg = AttackResolution.damage_calc(get_attack(attacker), attacker, target)

    AnsiCol.println(
        LogUtils.resolve_conditional_message(attacker.obj, "{The }$You deal{s} " ..dmg .. " damage!"),
        AnsiCol.GREEN, AnsiCol.BOLD
    )

    StatContext.add_hp(target, -random_round(dmg))

    AnsiCol.println(
        LogUtils.resolve_conditional_message(target.obj, "{The }$You [have]{has} " .. target.base.hp .. "HP left"), 
        AnsiCol.BLUE, AnsiCol.ITALIC .. AnsiCol.BOLD
    )

end

local frame = 1

local function step(context)
    for i=1,50 do
        StatContext.on_step(context)
        StatContext.on_calculate(context)
        frame = frame + 1
    end
end

local function dump_stats(player, monster)
    print("Player Before\t", StatUtils.stats_to_string(player.base, --[[Color]] true))
    print("Player After\t", StatUtils.stats_to_string(player.derived, --[[Color]] true))
    print("Monster Before\t", StatUtils.stats_to_string(monster.base, --[[Color]] true))
    print("Monster After\t", StatUtils.stats_to_string(monster.derived, --[[Color]] true))
end

local function battle(player, enemy)
    local spells = import "@stats.Spells"

    for item_name in values{"Health Potion", "Dagger"} do
        StatContext.add_item(player, ItemType.lookup(item_name))
    end
    StatContext.add_spell(player, Spells.lookup("Berserk"))
    choose_skills(player, 1000)
    step(player)
    while enemy.base.hp > 0 do
        print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
        print(StatUtils.attack_to_string(get_attack(player), --[[Color]] true))    
        local action = choose_option("Attack", "Spell", "Item")
        local moved = true
        if action == "Attack" then
            damage(player, enemy) 
        elseif action == "Spell" then
            moved = perform_spell(player, enemy)
        elseif action == "Item" then
            moved = use_item(player)
        end
        if moved then
            step(enemy)
            damage(enemy, player)
            step(player)
        end
    end
end

local function replace_event_log_with_print()
    local EventLog = import "core.ui.EventLog"
    EventLog.add = function(msg, color)
        AnsiCol.println(msg, AnsiCol.from_rgb(color or COL_WHITE))
    end
end

local function create_player_stats(race, name, --[[Optional]] team)
    local meta = {__copy = function(t1,t2) 
        table.copy(t1,t2, --[[Do not invoke meta]] false)
    end}

    local stats = race.on_create(name, team or Relations.TEAM_PLAYER_DEFAULT)
    for skill in values(SkillType.list) do
        local slot = {type = skill, level = 0, experience = 0}
        table.insert(stats.skills, setmetatable(slot, meta))
    end
    return stats
end

local function main()
    local animals = import "@monsters.DefineAnimals"
    -- Load stats
    import "@items.DefineConsumables"
    import "@items.DefineWeapons"
    import "@races.DefineRaces"
    import "@stats.DefineSkills"

    replace_event_log_with_print()

    -- Choose race
    local options = {}
    for race in values(Races.list) do
        table.insert(options, race.name)
    end

    local race = Races.lookup( choose_option(unpack(options)) )
    local stats = create_player_stats(race, "Tester")

    -- Create pseudo-objects
    local player = {
        is_local_player = function() return true end,
        base_stats = stats,
        traits = {"player"},
        derived_stats = table.deep_clone(stats),
        unarmed_attack = Attacks.attack_create(0, 5, Apts.MELEE),
        name = "TesterMan"
    }

    local monster_stats = table.deep_clone(animals.rat.stats)
    local monster = {
        traits = {"monster"},
        is_local_player = function() return false end,
        base_stats = monster_stats,
        derived_stats = table.deep_clone(monster_stats),
        name = animals.rat.name,
        unarmed_attack = animals.rat.unarmed_attack
    }

    local scmake = StatContext.game_object_stat_context_create
    battle(scmake(player), scmake(monster))
end

main()
