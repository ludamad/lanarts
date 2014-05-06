local StatContext = import "@StatContext"
local Apts = import "@stats.AptitudeTypes"
local Stats = import "@Stats"
local StatContext = import "@StatContext"
local CodeGeneration = import "core.CodeGeneration"
local CooldownTypes = import ".CooldownTypes"
local ExperienceCalculation = import ".ExperienceCalculation"
local C = import "terminal.AnsiColors"

local M = nilprotect {} -- Submodule

function M.round_for_print(num)
    assert(num >= 0)
    if num < 1 then return 1 end
    return math.floor(num)
end

local function to_camelcase(str)
    local parts = str:split("_")
    for i,part in pairs(parts) do
        parts[i] = part:lower():gsub("^%l", string.upper)
    end
    return table.concat(parts)
end

function M.stats_to_string(s, --[[Optional]] use_color, --[[Optional]] use_new_lines, --[[Optional]] alternate_name)
    local R = M.round_for_print
    local function if_color(col, --[[Optional]] ...)
        if use_color then
            if (...) then
                return col(...)
            else
                return tostring(col)
            end
        end
        return (...) or ""
    end

    local ret = ("%s %s %s %s %s"):format(
        if_color(C.WHITE, alternate_name or s.name), 
        if_color(C.GREEN, ("HP %d/%d"):format(R(s.hp), s.max_hp)), 
        if_color(C.BLUE, ("MP %d/%d"):format(R(s.mp), s.max_mp)),
        if_color(C.YELLOW, ("XP %d/%d"):format(s.xp, ExperienceCalculation.level_experience_needed(s.level))),
        if_color(C.YELLOW, ("(%d%%)"):format(math.floor(ExperienceCalculation.level_progress(s.xp, s.level)*100)))
    )

    local traits = {}
    for category,apts in pairs(s.aptitudes) do
        for trait,amnt in pairs(apts) do
            traits[trait] = true
        end
    end

    local trait_list = table.key_list(traits)
    table.sort(trait_list)
    local trait_strings = {""}
    for trait in values(trait_list) do
        local apts = s.aptitudes
        local not_all_0 = false
        local function apt(cat) -- Helper function
            local s = apts[cat][trait]
            local pre = to_camelcase(cat:sub(1,3)) .. ' '
            if s and s ~= 0 then
                not_all_0  = true 
                if s > 0 then 
                    return if_color(C.CYAN, pre.. '+' .. s, C.BOLD)
                else 
                    return if_color(C.RED, pre .. s, C.BOLD)
                end
            end
            return if_color(C.MAGENTA, pre .. '--')
        end
        local str = ("%s%s %s %s %s %s%s"):format(
            if_color(C.WHITE, '('),
            if_color(C.GREEN, to_camelcase(trait), C.BOLD), 
            apt("effectiveness"), apt("damage"), apt("resistance"), apt("defence"),
            if_color(C.WHITE, ')')
        )
        if not_all_0 then
            table.insert(trait_strings, str)
        end
    end
    ret = ret .. (use_new_lines and "\n" or " "):join(trait_strings)
    return ret
end

function M.attack_to_string(attack, --[[Optional]] use_color)
    local C = import "terminal.AnsiColors"
    local function if_color(col, --[[Optional]] ...)
        if use_color then
            if (...) then
                return col(...)
            else
                return tostring(col)
            end
        end
        return (...) or ""
    end
    assert(#attack.sub_attacks == 1) -- For now
    local sub = attack.sub_attacks[1]
    local ret = if_color(C.RED,
        ("{%s,%s "):format(
            sub.base_effectiveness > 0 and '+'..sub.base_effectiveness or sub.base_effectiveness, 
            '+'..sub.base_damage), 
        C.BOLD
    )

    local traits = {}
    for category,mults in pairs(sub) do
        if type(mults) == "table" then
            for trait,amnt in pairs(mults) do
                traits[trait] = true
            end
        end
    end

    local trait_list = table.sorted_key_list(traits)

    local apt_list = {}
    for trait in values(trait_list) do
        local function apt(cat)
            if sub[cat][trait] then
                return if_color(C.RED, '✔', C.BOLD)
            else
                return if_color(C.MAGENTA, ' --', C.BOLD)
            end
        end
        local apt = ("%s %s %s %s %s"):format(
            if_color(C.GREEN, to_camelcase(trait), C.BOLD), 
            apt("effectiveness_multipliers"), apt("damage_multipliers"), apt("resistance_multipliers"), apt("defence_multipliers")
        )
        table.insert(apt_list, apt)
    end

    return ret .. (" "):join(apt_list) .. if_color(C.RED, "}", C.BOLD)
end

local apt_func_signature = "copy_apts(a,b)"
M.stat_copy = CodeGeneration.copy_function_compile {
    prelude = ("local function %s\n%send\n"):format(
        apt_func_signature,
        CodeGeneration.copy_block_create {
            simple = table.sorted_key_list(Apts.allowed_aptitudes)
        }
    ),
    simple = {
        "name", "team", "level", "skill_points", "xp", 
        "hp", "max_hp", "hp_regen",
        "mp", "max_mp", "mp_regen",
        "movement_speed", "inventory"
    },
    deep = {
        "skills"
    },
    meta = {
        "cooldowns"
    },
    method = {
        hooks = "shallow_copy"
    },
    complex = {
        aptitudes = {
            complex = {
                defence = apt_func_signature, damage = apt_func_signature,
                resistance = apt_func_signature, effectiveness = apt_func_signature
            }
        }
    }
}

function M.stat_context_on_step(context)
    return StatContext.on_step(context, M.stat_copy)
end

function M.stat_clone(stats)
    local new_stats = Stats.stats_create()
    M.stat_copy(stats, new_stats)
    return new_stats    
end

function M.stat_full_clone(stats)
    local new_stats = M.stat_clone(stats)
    new_stats.skills = {}
    new_stats.spells = {}
    for k in values{"inventory", "skills", "spells"} do
        new_stats[k] = {}
        table.deep_copy(stats[k], new_stats[k], --[[Do not invoke meta]] false)
    end
    return new_stats
end

return M