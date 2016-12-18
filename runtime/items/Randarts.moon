HARDCODED_RANDARTS_SEED = 0xBADBABE
MAX_POWER_LEVEL = 3
RANDARTS = {}
for power=1,MAX_POWER_LEVEL
    RANDARTS[power] = {}

get_resource_names = (type) ->
    randart_images = require("compiled.Resources").resource_id_list
    return table.filter(randart_images, (res) -> res\match(type))

-- Random descriptions:
get_name_and_description = (rng, artifact, power_level) ->
    lower = artifact\lower()
    person = rng\random_choice {
        {"Paxanarian", "A #{lower} discovered by the ancient cartographer Paxanarian."}
        {"Gallanthor", "A #{lower} requistioned by the legendary mage Gallanthor."}
        {"Dredd", "A #{lower} worn by the late tactician Dredd Fiendblood."}
        {"Ludamad", "A #{lower} stolen by the jester savant Ludamad."}
        {"Xom", "A #{lower} created by Xom upon the grave of his follower."}
        {"Okawaru", "A #{lower} gifted by Okawaru to an epic warrior."}
        {"Trog", "A #{lower} gifted by Trog upon the scene of 100 deaths."}
        {"Gilnug", "A #{lower} created in communion with the fight gods."}
        {"Gragh", "A #{lower} crafted by sheer coincidence."}
        {"Meeplod", "A #{lower} crafted by sheer coincidence."}
        {"Nik", "A #{lower} crafted by sheer coincidence."}
        {"Orlov", "A #{lower} crafted by sheer coincidence."}
        {"Doogal", "A #{lower} crafted by sheer coincidence."}
        {"Stara", "A #{lower} crafted by sheer coincidence."}
        {"Hopkin", "A #{lower} crafted by sheer coincidence."}
        {"Denter", "A #{lower} crafted by sheer coincidence."}
        {"Lomen", "A #{lower} crafted by sheer coincidence."}
        {"Quantois", "A #{lower} crafted by sheer coincidence."}
        {"Logic", "A mysterious #{lower}."}
        {"Reason", "A mysterious #{lower}."}
        {"Forlong", "A mysterious #{lower}."}
        {"Silence", "A mysterious #{lower}."}
        {"Terror", "A mysterious #{lower}."}
    }
    power_adjective = ({
        "It is an item of great value."
        "It is sought after."
        "It is very sought after."
        "It is desparately sought after."
        "Mere knowledge of its existence is prized."
    })[power_level]
    assert(power_adjective ~= nil)
    trait = rng\random_choice {
        "Anger"
        "Deceit"
        "Vengeance"
        "Confusion"
        "Persuasion"
        "Undoing"
        "Blessing"
        "Curse"
        "Frustration"
        "Cunning"
        "Silence"
        "Wisdom"
        "Strength"
        "Foreboding"
        "Awe"
        "Reason"
        "Terror"
        "Mistake"
        "Quest"
        "Adventure"
        "Sorrow"
        "Singleton"
        "Savagery"
    }
    return "#{artifact} of #{person[1]}'s #{trait}", person[2] .. " " .. power_adjective

additive_stat_bonus = (attr, range) -> (rng, data) ->
    local bonus
    if math.floor(range[1]) ~= range[1] 
        bonus = rng\randomf(range[1], range[2])
    else
        bonus = rng\random(range[1], range[2] + 1)
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 0) + bonus

additive_core_bonus = (attr, range) -> (rng, data) ->
    bonus = rng\random(range[1], range[2] + 1)
    initial = data[attr] or {base: 0}
    if type(initial.base) == "number"
        initial.base = {initial.base, initial.base}
    {b1, b2} = initial.base
    data[attr] = table.merge (data[attr] or {}), {base: {b1 + bonus, b2 + bonus}}

mult_stat_bonus = (attr, range) -> (rng, data) ->
    mult = rng\randomf(range[1], range[2])
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 1) * mult

local MINOR_ENCHANTS, MAJOR_ENCHANTS, MINOR_DEBUFFS, MAJOR_DEBUFFS
-- Minor enchantments:
MINOR_ENCHANTS = {
    mult_stat_bonus("spell_velocity_multiplier", {1.10, 1.25})
    additive_stat_bonus("mp", {10, 35})
    additive_stat_bonus("hp", {10, 35})
    additive_stat_bonus("hpregen", {0.02, 0.03})
    additive_stat_bonus("mpregen", {0.02, 0.03})
    additive_stat_bonus("strength", {1, 1})
    additive_stat_bonus("defence", {1, 1})
    additive_stat_bonus("willpower", {1, 1})
    additive_stat_bonus("magic", {1, 1})
    additive_core_bonus("reduction", {1, 2})
    additive_core_bonus("resistance", {1, 2})
    additive_core_bonus("magic_reduction", {1, 2})
    additive_core_bonus("magic_resistance", {1, 2})
}

-- Minor debuffs:
-- Major enchantments:
MAJOR_ENCHANTS = {
    (rng, data) ->
        while true
            f = rng\random_choice(MINOR_ENCHANTS)
            if f == MINOR_ENCHANTS[1]
                continue
            f(rng, data)
            f(rng, data)
            break
}

-- Major debuffs:
MINOR_DEBUFFS = {
    mult_stat_bonus("spell_velocity_multiplier", {0.8, 0.9})
    additive_stat_bonus("mp", {-20, -5})
    additive_stat_bonus("hp", {-20, -5})
    additive_stat_bonus("hpregen", {-0.06, -0.02})
    additive_stat_bonus("mpregen", {-0.06, -0.02})
    additive_stat_bonus("strength", {-3, -1})
    additive_stat_bonus("defence", {-3, -1})
    additive_stat_bonus("willpower", {-3, -1})
    additive_stat_bonus("magic", {-3, -1})
    additive_core_bonus("reduction", {-3, -1})
    additive_core_bonus("resistance", {-3, -1})
    additive_core_bonus("magic_reduction", {-3, -1})
    additive_core_bonus("magic_resistance", {-3, -1})
}

NAMES_USED = {}
-- Define a single randart:
define_randart = (rng, base, images, enchanter) ->
    power_level = rng\random(1, MAX_POWER_LEVEL + 1)
    local name, description
    while true 
        name, description = get_name_and_description(rng, base.name, power_level)
        if not NAMES_USED[name] 
            NAMES_USED[name] = true
            break
    data = table.merge base, {
        :name, :description
        shop_cost: {
            base.shop_cost[1] + 100 * power_level * power_level, 
            base.shop_cost[2] + 150 * power_level * power_level
        }
        spr_item: rng\random_choice(images)
    }
    -- Make sure we don't see this as a randart-derivable item:
    data.randart_sprites = nil
    data.stat_bonuses or= {}
    n_enchants = power_level * 2
    -- Have less on weapons:
    if data.cooldown
        n_enchants /= 2
    while n_enchants > 0 
        if rng\random(8) == 0
            rng\random_choice(MAJOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_DEBUFFS)(rng, data)
        elseif rng\random(5) == 0
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_DEBUFFS)(rng, data)
            n_enchants += 1
        else
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
        n_enchants -= 1
    if enchanter
       enchanter(rng, data)
    table.insert(RANDARTS[power_level], data.name)
    return data

apply_enchantment = (rng, data) ->
    enchantment = if data.cooldown then 1 else 0
    while enchantment < 3 and rng\randomf() < (if data.cooldown then 0.2 else 0.1)
        enchantment += 1
    if enchantment > 0
        data.name = "+#{enchantment} #{data.name}"
        if data.cooldown
            additive_core_bonus("damage", {enchantment, enchantment})(rng, data)
            additive_core_bonus("power", {enchantment, enchantment})(rng, data)
        elseif rng\randomf() < 0.5
            additive_core_bonus("magic_reduction", {enchantment, enchantment})(rng, data)
            additive_core_bonus("magic_resistance", {enchantment, enchantment})(rng, data)
        else
            additive_core_bonus("reduction", {enchantment, enchantment})(rng, data)
            additive_core_bonus("resistance", {enchantment, enchantment})(rng, data)
        for i=1,2
            data.shop_cost[i] += math.floor((enchantment ^ 1.5) * 50)

-- Define several randart rings:
define_ring_randarts = (rng) ->
    images = get_resource_names("spr_rings.randarts")
    for i=1,100 * 2
        base = {
            name: "Ring"
            type: "ring"
            shop_cost: {0, 0}
        }
        Data.equipment_create(define_randart(rng, base, images))

define_equipment_randarts = (rng) ->
    -- RNG object just for generating randarts
    -- ATM the following MUST be a deterministic process, because of limitations
    -- in the Lanarts engine. Once we move to a better serialization library
    -- and have a more flexible object system we can move this into the code proper
    -- and not in a phase beforehand.
    rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
    define_ring_randarts(rng)
    for name, item in pairs(items)
        -- Judge whether its equipment by a cooldown not being present
        if item.randart_sprites ~= nil and item.cooldown == nil
            for i=1,(item.randart_weight or 20) * 2
                Data.equipment_create(define_randart(rng, item, item.randart_sprites, apply_enchantment))

define_weapon_randarts = () ->
    table.clear(NAMES_USED)
    table.clear(RANDARTS)
    for power=1,MAX_POWER_LEVEL
        RANDARTS[power] = {}
    -- RNG object just for generating randarts
    -- ATM the following MUST be a deterministic process, because of limitations
    -- in the Lanarts engine. Once we move to a better serialization library
    -- and have a more flexible object system we can move this into the code proper
    -- and not in a phase beforehand.
    rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
    for name, item in pairs(items)
        -- Judge whether its a weapon by a cooldown being present
        if item.randart_sprites ~= nil and item.cooldown ~= nil
            for i=1,(item.randart_weight or 20) * 2
                template = define_randart(rng, item, item.randart_sprites, apply_enchantment)
                Data.weapon_create(template)

return {:RANDARTS, :MAX_POWER_LEVEL, :define_equipment_randarts, :define_weapon_randarts}
