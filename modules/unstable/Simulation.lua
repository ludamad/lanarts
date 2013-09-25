-- OpenGL must be initialized before content loading!
local Display = import "core.Display"
Display.initialize("Lanarts", {640, 640}, false)

local StatContext = import "@StatContext"
local AttackResolution = import "@AttackResolution"
local Attacks = import "@Attacks"
local Stats = import "@Stats"
local AnsiCol = import "core.terminal.AnsiColors"
local RaceType = import "@RaceType"
local ItemType = import "@ItemType"
local SkillType = import "@SkillType"
local SpellType = import "@SpellType"
local ClassType = import "@ClassType"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local ProficiencyPenalties = import "@stats.ProficiencyPenalties"

local Relations = import "lanarts.objects.Relations"
local Apts = import "@stats.AptitudeTypes"
local StatUtils = import "@stats.StatUtils"
local ItemTraits = import "@items.ItemTraits"
local Identification = import "@stats.Identification"

local GameMap = import "core.GameMap"
local GameObject = import "core.GameObject"
local GameState = import "core.GameState"
local PlayerObject = import "@objects.PlayerObject"
local MonsterObject = import "@objects.MonsterObject"

local CooldownTypes = import "@stats.CooldownTypes"

local SimulationMap = import "@SimulationMap"
local Proficiency = import "@Proficiency"
local LogUtils = import "lanarts.LogUtils"
local Keys = import "core.Keyboard"

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
        if not input or input:lower() == 'exit' or input:lower() == 'quit' then os.exit(0) end
        choice = options[tonumber(input)]
        if not choice then
            AnsiCol.println("Sorry, that is not a valid choice.", AnsiCol.RED, AnsiCol.BOLD)
        end
    end

    return choice
end

-- Returns whether performed the spell
local function perform_spell(player, monster)
    local spells = player.base.spells
    local choices = {"Back"}
    local choice2spell = {}
    for spell in spells:values() do
        table.insert(choices, spell.name)
        choice2spell[spell.name] = spell
    end

    local spell = choice2spell[choose_option(unpack(choices))]
    if not spell then return false end

    local used = StatContext.use_spell(player, spell, monster.obj.xy)
    if not used then
        print("You must wait before using this.")
    end

    return used
end

local function choose_skills(player, SP)
    while SP > 0 do
        StatContext.on_step(player)
        StatContext.on_calculate(player)
        print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
        -- Pick skill
        AnsiCol.println("You have "..SP.."SP to spend.", AnsiCol.GREEN, AnsiCol.BOLD)
        local choices = {}
        local choice2skill = {}
        for skill in values(SkillType.list) do
            table.insert(choices, skill.name)
            choice2skill[skill.name] = skill
        end
        table.insert(choices, "Finish")
    
        local skill = choice2skill[choose_option(unpack(choices))]
        if not skill then break end

        -- Pick skill points gain
        while SP > 0 do
            StatContext.on_step(player)
            StatContext.on_calculate(player)
            print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
    
            AnsiCol.println("You have "..SP.."SP to spend.", AnsiCol.GREEN, AnsiCol.BOLD)
            AnsiCol.println(
                ("Spend how much on %s%s"):format(
                    AnsiCol.WHITE(skill.name, AnsiCol.BOLD), 
                    AnsiCol.GREEN('?', AnsiCol.BOLD)), 
                AnsiCol.GREEN, AnsiCol.BOLD
            )
            local skill_slot = Stats.get_skill(player.base, skill)
    
            local function xp2level(xp)
                return ExperienceCalculation.skill_level_from_cost(skill_slot.cost_multiplier, xp)
            end
            local function level2xp(xp_level)
                return ExperienceCalculation.cost_from_skill_level(skill_slot.cost_multiplier, xp_level)
            end
            local function needed(gain)
                return math.ceil(level2xp(skill_slot.level + gain) - skill_slot.skill_points) 
            end
    
            local need5, need1, need0_1 = needed(5.0), needed(1.0), needed(0.1)

            local choices, amounts = {"Back"}, {}
            local function reg(k,xp) table.insert(choices, k) ; amounts[k] = xp end

            if need0_1 <= SP then
                reg(("+0.1 (Spend %s to become %s)"):format(need0_1, skill_slot.level+0.1), need0_1)
            end
            if need1 <= SP then
                reg(("+1 (Spend %s to become %s)"):format(need1, skill_slot.level+1), need1)
            end
            if need5 <= SP then
                reg(("+5 (Spend %s to become %s)"):format(need5, skill_slot.level+5), need5)
            end
            local max = xp2level(skill_slot.skill_points + SP)
            reg("Max (Spend ".. SP .. " to become "..max..")", SP)

            local amount = amounts[choose_option(unpack(choices))]
            if not amount then break end

            SP = SP - amount
            skill_slot:on_spend_skill_points(amount)
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
        local name = Identification.name_and_description(player, item)
        local modifier = StatContext.calculate_proficiency_modifier(player, item)
        local entry
        if not Identification.is_identified(player, item) then
            entry = AnsiCol.YELLOW(name) .. AnsiCol.WHITE(' [Unidentified]', AnsiCol.FAINT)
        else
            entry = modifier > 0 and AnsiCol.WHITE(name) or AnsiCol.WHITE(name, AnsiCol.BOLD)
            if modifier > 0 then
                if modifier <= 0.05 then entry = entry .. AnsiCol.MAGENTA(" [Easy]", AnsiCol.BOLD)
                elseif modifier <= 0.10 then entry = entry .. AnsiCol.MAGENTA(" [Tricky]", AnsiCol.BOLD)
                elseif modifier <= 0.15 then entry = entry .. AnsiCol.MAGENTA(" [Hard]", AnsiCol.BOLD)
                elseif modifier <= 0.75 then entry = entry .. AnsiCol.MAGENTA(" [Very Hard]", AnsiCol.BOLD) 
                else entry = AnsiCol.MAGENTA(name) .. AnsiCol.RED(" [Untrained]", AnsiCol.BOLD) end
            end
        end
        entry = entry .. AnsiCol.YELLOW(item.equipped and equip_msg or "")
        table.insert(choices, entry)
        choice2item[entry] = item
    end

    local item = choice2item[choose_option(unpack(choices))]
    if not item then return false end

    inventory:use_item(player, item)
    return true
end

local function track_identification(stats)
    stats.base.identifications = stats.base.identifications or {}
    table.clear(stats.base.identifications)
    for item in stats.base.inventory:values() do
        stats.base.identifications[item] = Identification.is_identified(stats, item)
    end
end

local function WHITE_BOLD(s) return AnsiCol.WHITE(s, AnsiCol.BOLD) end
local function WHITE_FAINT(s) return AnsiCol.WHITE(s, AnsiCol.FAINT) end

local function report_new_identifications(stats)
    stats.base.identifications = stats.base.identifications or {}
    for item in stats.base.inventory:values() do
        if (stats.base.identifications[item] == false) and Identification.is_identified(stats, item) then
            print(WHITE_BOLD("You identify the ".. item.name .. " (Was ") .. WHITE_FAINT(item.unidentified_name) .. WHITE_BOLD(")"))
        end
    end
end
local frame = 1

local function dump_stats(player, monster)
    print("Player Before\t", StatUtils.stats_to_string(player.base, --[[Color]] true))
    print("Player After\t", StatUtils.stats_to_string(player.derived, --[[Color]] true))
    print("Monster Before\t", StatUtils.stats_to_string(monster.base, --[[Color]] true))
    print("Monster After\t", StatUtils.stats_to_string(monster.derived, --[[Color]] true))
end

local function give_all_items(player)
    for item in values(ItemType.list) do
        local IT = ItemTraits
        local function is(trait) return table.contains(item.traits, trait) end
 
        if table.contains(item.traits, ItemTraits.WEAPON) then
            StatContext.add_item(player, {type = item, effectiveness_bonus = random(-2,4), damage_bonus = random(-2,4)})
        elseif is(IT.BODY_ARMOUR) or is(IT.BRACERS) or is(IT.BOOTS) or is(IT.HEADGEAR) or is(IT.GLOVES) then
            StatContext.add_item(player, {type = item, bonus = random(1,3)})
        else 
            StatContext.add_item(player, {type = item, bonus = random(1,5)})
        end
    end
end

local frame, WAIT_UNTIL = 0, 0

local function can_continue(player)
    if frame < WAIT_UNTIL then return false end
    return true--not StatContext.has_cooldown(player, CooldownTypes.ALL_ACTIONS)
end

local function query_player(player, monster)
    frame = frame + 1
    local attack = player.obj:melee_attack()
    if not can_continue(player) then return end
    local moved = false
    while not moved do
        AnsiCol.println("Frame: " .. frame, AnsiCol.YELLOW, AnsiCol.BOLD)
        print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
        print(StatUtils.attack_to_string(attack, --[[Color]] true))    
        local action = choose_option("Attack", "Spell", "Item", "Wait")
        if action == "Attack" then
            local able, problem = player.obj:can_attack(attack, monster.obj)
            if able then
                player.obj:apply_attack(attack, monster.obj)
                moved = true
            else
                print(problem)
            end
        elseif action == "Spell" then
            moved = perform_spell(player, monster)
        elseif action == "Item" then
            moved = use_item(player)
        elseif action == "Wait" then
            WAIT_UNTIL = frame + 10
            moved = true
        end
    end
end

local function print_sample_stats(race, --[[Optional]] class)
    local stats = PlayerObject.player_stats_create(race, class, "Level 1 " .. race.name)
    local context = StatContext.stat_context_create(stats)
    StatContext.on_step(context)
    StatContext.on_calculate(context)
    print(StatUtils.stats_to_string(context.derived, --[[Color]] true, --[[New lines]] true, stats.name))
    return stats
end

local function choose_race()
    local C = AnsiCol
    local idx = 1
    while true do
        local race = RaceType.list[idx]
        print(C.YELLOW("Race ".. idx .. ") ", C.BOLD).. C.WHITE(race.name))
        print(C.YELLOW(race.description))
        print_sample_stats(race)
        print(C.BLUE("Do you wish to play as a " .. race.name .. "?", C.BOLD))
        if choose_option("No", "Yes") == "Yes" then
            return race
        end
        idx = (idx % #RaceType.list) + 1 
    end
end

local function choose_class(race)
    local class_types = {}
    table.insert(class_types, ClassType.lookup("Knight"))
    for v in values{"Force"} do
        table.insert(class_types, ClassType.lookup("Mage"):on_create {magic_skill = v, weapon_skill = "Slashing Weapons"})
    end

    local C = AnsiCol
    local idx = 1
    while true do
        local class = class_types[idx]
        print(C.YELLOW("Class ".. idx .. ") ", C.BOLD).. C.WHITE(class.name))
        print(C.YELLOW(class.description))
        print_sample_stats(race, class:on_create{weapon_skill="Slashing Weapons"})
        print(C.BLUE("Do you wish to play as a " .. race.name .. " " .. class.name .. "?", C.BOLD))
        if choose_option("No", "Yes") == "Yes" then
            print("Which specialization?")
            local skill = choose_option("Piercing Weapons", "Slashing Weapons", "Blunt Weapons")
            return class:on_create{magic_skill = class.magic_skill, weapon_skill=skill}
        end
        idx = (idx % #class_types) + 1 
    end
end

local function main()
    -- Load game content
    import "@DefineAll"

    -- Make sure EventLog.add prints to console instead
    local EventLog = import "core.ui.EventLog"
    EventLog.add = function(msg, color)
        AnsiCol.println(msg, AnsiCol.from_rgb(color or COL_WHITE))
    end

    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()

    local SM = SimulationMap.create()
    local race = choose_race()
    local class = choose_class(race)
    local player = SM:add_player("Tester", race, class)
    local monster = SM:add_monster("Giant Rat")

--    local TimeToKillCalculation = import "@simulation.TimeToKillCalculation"
--    local kill_time = TimeToKillCalculation.calculate_time_to_kill(
--        player:stat_context_copy(), player:melee_attack(), monster:stat_context_copy()
--    )
--    print("To kill the giant rat, you need " .. kill_time .. " steps with melee.")
    assert((import "lanarts.objects.Relations").is_hostile(player, monster))

    while GameState.input_capture() and not Keys.key_pressed(Keys.ESCAPE) do
        SM:step()
        Display.draw_start()
        SM:draw()
        Display.draw_finish()
        GameState.wait(5)
        report_new_identifications(player:stat_context())
        track_identification(player:stat_context())
        query_player(player:stat_context(), monster:stat_context())
    end
end

main()