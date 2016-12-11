HARDCODED_RANDARTS_SEED = 0xBADBABE
MAX_POWER_LEVEL = 3
RANDARTS = {}
for power=1,MAX_POWER_LEVEL
    RANDARTS[power] = {}

get_resource_names = (type) ->
    randart_images = require("compiled.Resources").resource_id_list
    return table.filter(randart_images, (res) -> res\match(type))

-- Random descriptions:
get_ring_name_and_description = (rng, power_level) ->
    person = rng\random_choice {
        {"Paxanarian", "A ring discovered by the ancient cartographer Paxanarian."}
        {"Gallanthor", "A ring requistioned by the legendary mage Gallanthor."}
        {"Dredd", "A ring worn by the late tactician Dredd Fiendblood."}
        {"Ludamad", "A ring stolen by the jester savant Ludamad."}
        {"Xom", "A ring created by Xom upon the grave of his follower."}
        {"Okawaru", "A ring gifted by Okawaru to an epic warrior."}
        {"Trog", "A ring gifted by Trog upon the scene of 100 deaths."}
        {"Gilnug", "A ring created in communion with the fight gods."}
        {"Gragh", "A ring crafted by sheer coincidence."}
        {"Logic", "A mysterious ring."}
    }
    power_adjective = ({
        "It is not very sought after."
        "It is a bit sought after."
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
    }
    return "Ring of #{person[1]}'s #{trait}", person[2] .. " " .. power_adjective

additive_stat_bonus = (attr, range) -> (rng, data) ->
    bonus = rng\random(range[1], range[2] + 1)
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 0) + bonus

mult_stat_bonus = (attr, range) -> (rng, data) ->
    mult = rng\randomf(range[1], range[2])
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 1) * mult

local MINOR_ENCHANTS, MAJOR_ENCHANTS, MINOR_DEBUFFS, MAJOR_DEBUFFS
-- Minor enchantments:
MINOR_ENCHANTS = {
    mult_stat_bonus("spell_velocity_multiplier", {1.10, 1.25})
    additive_stat_bonus("mp", {5, 25})
    additive_stat_bonus("hp", {5, 25})
}

-- Minor debuffs:
-- Major enchantments:
MAJOR_ENCHANTS = {
    additive_stat_bonus("mp", {25, 75})
    additive_stat_bonus("hp", {25, 75})
}

-- Major debuffs:
MINOR_DEBUFFS = MINOR_ENCHANTS

NAMES_USED = {}
-- Define a single randart ring:
define_ring_randart = (rng, images) ->
    power_level = rng\random(1, MAX_POWER_LEVEL + 1)
    local name, description
    while true 
        name, description = get_ring_name_and_description(rng, power_level)
        if not NAMES_USED[name] 
            NAMES_USED[name] = true
            table.insert(RANDARTS[power_level], name)
            break
    data = {
        type: "ring"
        :name, :description
        shop_cost: {100 * power_level * power_level, 150 * power_level * power_level}
        spr_item: rng\random_choice(images)
        stat_bonuses: {}
    }
    for i=1,power_level
        if rng\random(3) == 0
            rng\random_choice(MAJOR_ENCHANTS)(rng, data)
        elseif rng\random(3) == 0
            rng\random_choice(MAJOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_DEBUFFS)(rng, data)
        else
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
    Data.equipment_create(data)

-- Define several randart rings:
define_ring_randarts = (rng) ->
    images = get_resource_names("spr_rings.randarts")
    for i=1,100
        define_ring_randart(rng, images)

define_randarts = () ->
    -- RNG object just for generating randarts
    -- ATM the following MUST be a deterministic process, because of limitations
    -- in the Lanarts engine. Once we move to a better serialization library
    -- and have a more flexible object system we can move this into the code proper
    -- and not in a phase beforehand.
    rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
    define_ring_randarts(rng)

return {:define_randarts, :RANDARTS, :MAX_POWER_LEVEL}
