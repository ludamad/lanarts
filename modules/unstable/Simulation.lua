local StatContext = import "@StatContext"
local AttackResolution = import "@AttackResolution"
local Attacks = import "@Attacks"
local Stats = import "@Stats"
local AnsiCol = import "core.terminal.AnsiColors"
local Races = import "@Races"
local ItemType = import "@ItemType"
local SkillType = import "@SkillType"
local SpellType = import "@SpellType"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local ProficiencyPenalties = import "@stats.ProficiencyPenalties"

local Relations = import "lanarts.objects.Relations"
local Apts = import "@stats.AptitudeTypes"
local StatUtils = import "@stats.StatUtils"
local ItemTraits = import "@items.ItemTraits"
local Identification = import "@stats.Identification"

local GameMap = import "core.GameMap"
local GameObject = import "core.GameObject"
local PlayerObject = import "@objects.PlayerObject"
local MonsterObject = import "@objects.MonsterObject"

local SimulationMap = import "@SimulationMap"
local Proficiency = import "@Proficiency"
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

    local used = StatContext.use_spell(player, spell)
    if not used then
        print("You must wait before using this.")
    end

    return used
end

local function choose_skills(player, xp_to_spend)
    while xp_to_spend > 0 do
        StatContext.on_step(player)
        StatContext.on_calculate(player)
        print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
        -- Pick skill
        AnsiCol.println("You have "..xp_to_spend.."SP to spend.", AnsiCol.GREEN, AnsiCol.BOLD)
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
        while xp_to_spend > 0 do
            StatContext.on_step(player)
            StatContext.on_calculate(player)
            print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
    
            AnsiCol.println("You have "..xp_to_spend.."SP to spend.", AnsiCol.GREEN, AnsiCol.BOLD)
            AnsiCol.println(
                ("Spend how much on %s%s"):format(
                    AnsiCol.WHITE(skill.name, AnsiCol.BOLD), 
                    AnsiCol.GREEN('?', AnsiCol.BOLD)), 
                AnsiCol.GREEN, AnsiCol.BOLD
            )
            local skill_slot = Stats.get_skill(player.base, skill)
    
            local function xp2level(xp)
                return ExperienceCalculation.skill_level_from_cost(skill.cost_multiplier, xp)
            end
            local function level2xp(xp_level)
                return ExperienceCalculation.cost_from_skill_level(skill.cost_multiplier, xp_level)
            end
            local function needed(gain)
                return level2xp(skill_slot.level + gain) - skill_slot.experience 
            end
    
            local need5, need1, need0_1 = needed(5.0), needed(1.0), needed(0.1)

            local choices, amounts = {"Back"}, {}
            local function reg(k,xp) table.insert(choices, k) ; amounts[k] = xp end

            if need0_1 <= xp_to_spend then
                reg(("+0.1 (Spend %s to become %s)"):format(need0_1, skill_slot.level+0.1), need0_1)
            end
            if need1 <= xp_to_spend then
                reg(("+1 (Spend %s to become %s)"):format(need1, skill_slot.level+1), need1)
            end
            if need5 <= xp_to_spend then
                reg(("+5 (Spend %s to become %s)"):format(need5, skill_slot.level+5), need5)
            end
            local max = xp2level(skill_slot.experience + xp_to_spend)
            reg("Max (Spend ".. xp_to_spend .. " to become "..max..")", xp_to_spend)

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

local function get_attack(attacker)
    local weapon = StatContext.get_equipped_item(attacker, ItemTraits.WEAPON)
    if weapon then
        local modifier = StatContext.calculate_proficiency_modifier(attacker, weapon)
        return ProficiencyPenalties.apply_attack_modifier(weapon.type.attack, modifier)
    end
    return attacker.obj.unarmed_attack -- Default
end

local function damage(attacker, target)

    local dmg = AttackResolution.damage_calc(get_attack(attacker), attacker, target)

    AnsiCol.println(
        LogUtils.resolve_conditional_message(attacker.obj, "{The }$You deal{s} " ..dmg .. " damage!"),
        AnsiCol.GREEN, AnsiCol.BOLD
    )

    StatContext.add_hp(target, -random_round(dmg))

    AnsiCol.println(
        LogUtils.resolve_conditional_message(target.obj, "{The }$You [have]{has} " .. math.ceil(target.base.hp) .. "HP left"), 
        AnsiCol.BLUE, AnsiCol.ITALIC .. AnsiCol.BOLD
    )

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

local function step(context)
    for i=1,50 do
        StatContext.on_step(context)
        StatContext.on_calculate(context)
        frame = frame + 1
        
        report_new_identifications(context)
        track_identification(context)
    end
end

local function dump_stats(player, monster)
    print("Player Before\t", StatUtils.stats_to_string(player.base, --[[Color]] true))
    print("Player After\t", StatUtils.stats_to_string(player.derived, --[[Color]] true))
    print("Monster Before\t", StatUtils.stats_to_string(monster.base, --[[Color]] true))
    print("Monster After\t", StatUtils.stats_to_string(monster.derived, --[[Color]] true))
end

local function battle(player, monster)
    local spells = import "@stats.SpellType"

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
    StatContext.add_spell(player, "Berserk")
    choose_skills(player, 24000)
    step(player)
    track_identification(player)
    while monster.base.hp > 0 do
        print(StatUtils.stats_to_string(player.derived, --[[Color]] true, --[[New lines]] true, player.base.name .. ", the Adventurer"))
        print(StatUtils.attack_to_string(get_attack(player), --[[Color]] true))    
        local action = choose_option("Attack", "Spell", "Item")
        local moved = true
        if action == "Attack" then
            damage(player, monster) 
        elseif action == "Spell" then
            moved = perform_spell(player, monster)
        elseif action == "Item" then
            moved = use_item(player)
        end
        if moved then
            step(player)
            damage(monster, player)
            step(monster)
        end
    end
end

local function choose_race()
    local C = AnsiCol
    local idx = 1
    while true do
        local race = Races.list[idx]
        print(C.YELLOW("Race ".. idx .. ") ", C.BOLD).. C.WHITE(race.name))
        print(C.YELLOW(race.description))
        local stats = PlayerObject.create_player_stats(race, "Level 1 " .. race.name)
        local context = StatContext.stat_context_create(stats)
        StatContext.on_step(context)
        StatContext.on_calculate(context)
        print(StatUtils.stats_to_string(context.derived, --[[Color]] true, --[[New lines]] true, stats.name))
        print(C.BLUE("Do you wish to play as a " .. race.name .. " Adventurer?", C.BOLD))
        if choose_option("No", "Yes") == "Yes" then
            return race
        end
        idx = (idx % #Races.list) + 1 
    end
end

local function main()
    -- OpenGL must be initialized before content loading!
    local Display = import "core.Display"
    Display.initialize("Lanarts", {640, 480}, false)

    -- Load game content
    import "@items.DefineConsumables"
    import "@items.DefineWeapons"
    import "@items.DefineRings"
    import "@items.DefineArmour"

    import "@races.DefineRaces"

    import "@stats.DefineSkills"

    import "@monsters.DefineAnimals"

    -- Make sure EventLog.add prints to console instead
    local EventLog = import "core.ui.EventLog"
    EventLog.add = function(msg, color)
        AnsiCol.println(msg, AnsiCol.from_rgb(color or COL_WHITE))
    end

    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()

    local SM = SimulationMap.create()
    local player = SM:add_player("Tester", choose_race(), {25,25})
    local monster = SM:add_monster("Giant Rat", {75,75})

    Display.draw_start()
    SM:draw()
    Display.draw_finish()

    battle(StatContext.stat_context_create(player.base_stats, player.derived_stats, player), StatContext.stat_context_create(monster.base_stats, monster.derived_stats, monster))
end

main()