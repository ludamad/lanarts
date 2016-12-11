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
    person = rng\random_choice {
        {"Paxanarian", "A #{artifact} discovered by the ancient cartographer Paxanarian."}
        {"Gallanthor", "A #{artifact} requistioned by the legendary mage Gallanthor."}
        {"Dredd", "A #{artifact} worn by the late tactician Dredd Fiendblood."}
        {"Ludamad", "A #{artifact} stolen by the jester savant Ludamad."}
        {"Xom", "A #{artifact} created by Xom upon the grave of his follower."}
        {"Okawaru", "A #{artifact} gifted by Okawaru to an epic warrior."}
        {"Trog", "A #{artifact} gifted by Trog upon the scene of 100 deaths."}
        {"Gilnug", "A #{artifact} created in communion with the fight gods."}
        {"Gragh", "A #{artifact} crafted by sheer coincidence."}
        {"Logic", "A mysterious #{artifact}."}
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
    }
    return "#{artifact} of #{person[1]}'s #{trait}", person[2] .. " " .. power_adjective

additive_stat_bonus = (attr, range) -> (rng, data) ->
    local bonus
    if math.floor(range[1]) ~= range[1] 
        bonus = rng\randomf(range[1], range[2])
    else
        bonus = rng\random(range[1], range[2] + 1)
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 0) + bonus

additive_armour_bonus = (attr, range) -> (rng, data) ->
    bonus = rng\random(range[1], range[2] + 1)
    data[attr] or= {base: 0}
    data[attr] = {base: data[attr].base + bonus}

mult_stat_bonus = (attr, range) -> (rng, data) ->
    mult = rng\randomf(range[1], range[2])
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 1) * mult

local MINOR_ENCHANTS, MAJOR_ENCHANTS, MINOR_DEBUFFS, MAJOR_DEBUFFS
-- Minor enchantments:
MINOR_ENCHANTS = {
    mult_stat_bonus("spell_velocity_multiplier", {1.10, 1.25})
    additive_stat_bonus("mp", {5, 25})
    additive_stat_bonus("hp", {5, 25})
    additive_stat_bonus("hpregen", {0.01, 0.04})
    additive_stat_bonus("mpregen", {0.01, 0.04})
    additive_stat_bonus("strength", {1, 2})
    additive_stat_bonus("defence", {1, 2})
    additive_stat_bonus("willpower", {1, 2})
    additive_stat_bonus("magic", {1, 2})
    additive_armour_bonus("reduction", {1, 2})
    additive_armour_bonus("resistance", {1, 2})
    additive_armour_bonus("magic_reduction", {1, 2})
    additive_armour_bonus("magic_resistance", {1, 2})
}

-- Minor debuffs:
-- Major enchantments:
MAJOR_ENCHANTS = {
    (rng, data) ->
        f = rng\random_choice(MINOR_ENCHANTS)
        f(rng, data)
        f(rng, data)
}

-- Major debuffs:
MINOR_DEBUFFS = {
    mult_stat_bonus("spell_velocity_multiplier", {0.8, 0.9})
    additive_stat_bonus("mp", {-25, -5})
    additive_stat_bonus("hp", {-25, -5})
    additive_stat_bonus("hpregen", {-0.04, -0.01})
    additive_stat_bonus("mpregen", {-0.04, -0.01})
    additive_stat_bonus("strength", {-2, -1})
    additive_stat_bonus("defence", {-2, -1})
    additive_stat_bonus("willpower", {-2, -1})
    additive_stat_bonus("magic", {-2, -1})
    additive_armour_bonus("reduction", {-2, -1})
    additive_armour_bonus("resistance", {-2, -1})
    additive_armour_bonus("magic_reduction", {-2, -1})
    additive_armour_bonus("magic_resistance", {-2, -1})
}

NAMES_USED = {}
-- Define a single randart ring:
define_randart = (rng, base, images) ->
    power_level = rng\random(1, MAX_POWER_LEVEL + 1)
    local name, description
    while true 
        name, description = get_name_and_description(rng, base.name, power_level)
        if not NAMES_USED[name] 
            NAMES_USED[name] = true
            table.insert(RANDARTS[power_level], name)
            break
    data = table.merge base, {
        :name, :description
        shop_cost: {
            base.shop_cost[1] + 100 * power_level * power_level, 
            base.shop_cost[2] + 150 * power_level * power_level
        }
        spr_item: rng\random_choice(images)
    }
    data.stat_bonuses or= {}
    n_enchants = power_level * 2
    while n_enchants > 0 
        if rng\random(3) == 0 and n_enchants >= 3
            rng\random_choice(MAJOR_ENCHANTS)(rng, data)
            n_enchants -= 3
        elseif rng\random(3) == 0
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_DEBUFFS)(rng, data)
        else
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
            n_enchants -= 1
    Data.equipment_create(data)

-- Define several randart rings:
define_ring_randarts = (rng) ->
    images = get_resource_names("spr_rings.randarts")
    for i=1,100
        base = {
            name: "Ring"
            type: "ring"
            shop_cost: {0, 0}
        }
        define_randart(rng, base, images)

define_equipment_randarts = (rng) ->
    for item in *items
        if item.randart_sprites ~= nil
            for i=1,20
                define_randart(rng, item, item.randart_sprites)

define_randarts = () ->
    -- RNG object just for generating randarts
    -- ATM the following MUST be a deterministic process, because of limitations
    -- in the Lanarts engine. Once we move to a better serialization library
    -- and have a more flexible object system we can move this into the code proper
    -- and not in a phase beforehand.
    rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
    define_ring_randarts(rng)
    define_equipment_randarts(rng)

return {:define_randarts, :RANDARTS, :MAX_POWER_LEVEL}
