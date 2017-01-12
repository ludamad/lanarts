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
        {"Gragh", "A #{lower} crafted with brutish determination."}
        {"Meeplod", "A #{lower} crafted over supernaturally hot fire."}
        {"Nik", "A #{lower} crafted by a dice roll."}
        {"Orlov", "A #{lower} crafted by great strength."}
        {"Doogal", "A #{lower} crafted by enchanting magic."}
        {"Stara", "A #{lower} crafted by terrible sacrifice."}
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

add_random_effect = () -> (rng, data) ->
    if data.effects_granted
        return -- For now, dont have double effects.
    data.effects_granted or= {}
    {effect, description} = rng\random_choice {
        {"VampiricWeapon", "You gain the power to steal life with your melee blows."}
        {"ConfusingWeapon", "You gain the power to daze foes in melee."}
        {"PoisonedWeapon", "You gain the power to poison foes in melee."}
        {"PossiblySummonStormElementalOnKill", "You gain the power to summon storm elementals as you kill things."}
        {"PossiblySummonGolemOnKill", "You gain the power to summon golems as you kill things."}
        {"PossiblySummonCentaurOnKill", "You gain the power to summon centaurs as you kill things."}
    }
    data.description ..= " #{description}"
    append data.effects_granted, effect

add_random_spell = () -> (rng, data) ->
    if data.spells_granted
        return -- For now, dont have double spell items.
    data.spells_granted or= {}
    spell = rng\random_choice {
        "Minor Missile",
        "Fireball",
        "Fire Bolt",
        "Mephitize",
        "Trepidize",
        "Regeneration",
        "Berserk",
        "Blink",
        "Magic Arrow",
        "Magic Blast",
        "Power Strike",
        "Pain",
        "Greater Pain",
        "Fear Strike",
        "Expedite",
        "Wallanthor",
        "Call Spikes",
        "Luminos",
        "Healing Aura",
        "Ice Form",
    }
    if table.contains(data.spells_granted, spell) 
        return
    data.description ..= " Grants the user the ability to use '#{spell}'"
    append data.spells_granted, spell

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

mult_core_bonus = (attr, range) -> (rng, data) ->
    mult = rng\randomf(range[1], range[2])
    data[attr] = (data[attr] or 1) * mult

mult_stat_bonus = (attr, range) -> (rng, data) ->
    mult = rng\randomf(range[1], range[2])
    data.stat_bonuses[attr] = (data.stat_bonuses[attr] or 1) * mult

local MINOR_ENCHANTS, MAJOR_ENCHANTS, MINOR_DEBUFFS, MAJOR_DEBUFFS
-- Minor enchantments:
MINOR_ENCHANTS = {
    mult_stat_bonus("spell_velocity_multiplier", {1.10, 1.25})
    --add_random_spell()
    add_random_effect()
    additive_stat_bonus("mp", {10, 25})
    additive_stat_bonus("hp", {10, 25})
    additive_stat_bonus("hpregen", {0.02, 0.03})
    additive_stat_bonus("mpregen", {0.02, 0.03})
    additive_stat_bonus("strength", {1, 2})
    additive_stat_bonus("defence", {1, 2})
    additive_stat_bonus("willpower", {1, 2})
    additive_stat_bonus("magic", {1, 2})
    additive_core_bonus("reduction", {1, 2})
    additive_core_bonus("resistance", {1, 2})
    additive_core_bonus("magic_reduction", {1, 2})
    additive_core_bonus("magic_resistance", {1, 2})
    mult_core_bonus("magic_cooldown_multiplier", {0.89, 0.95})
    mult_core_bonus("melee_cooldown_multiplier", {0.89, 0.95})
    mult_core_bonus("ranged_cooldown_multiplier", {0.89, 0.95})
}

-- Minor debuffs:
-- Major enchantments:
MAJOR_ENCHANTS = {
    (rng, data) -> -- TODO
        for i=1,2
            f = rng\random_choice(MINOR_ENCHANTS)
            f(rng, data)
}

-- Major debuffs:
MINOR_DEBUFFS = {
    mult_stat_bonus("spell_velocity_multiplier", {0.8, 0.9})
    additive_stat_bonus("mp", {-20, -15})
    additive_stat_bonus("hp", {-20, -15})
    additive_stat_bonus("hpregen", {-0.03, -0.01})
    additive_stat_bonus("mpregen", {-0.03, -0.01})
    additive_stat_bonus("strength", {-3, -1})
    additive_stat_bonus("defence", {-3, -1})
    additive_stat_bonus("willpower", {-3, -1})
    additive_stat_bonus("magic", {-3, -1})
    additive_core_bonus("reduction", {-3, -1})
    additive_core_bonus("resistance", {-3, -1})
    additive_core_bonus("magic_reduction", {-3, -1})
    additive_core_bonus("magic_resistance", {-3, -1})
    mult_core_bonus("magic_cooldown_multiplier", {1.10, 1.20})
    mult_core_bonus("melee_cooldown_multiplier", {1.1, 1.2})
    mult_core_bonus("ranged_cooldown_multiplier", {1.1, 1.2})
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
    data = table.merge table.deep_clone(base), {
        :name, :description
        shop_cost: {
            base.shop_cost[1] + 100 * power_level * power_level, 
            base.shop_cost[2] + 150 * power_level * power_level
        }
        spr_item: rng\random_choice(images)
    }
    -- Make sure we don't see this as a randart-derivable item:
    data.randart_sprites = nil
    data.randart_weight = 0
    data.stat_bonuses or= {}
    n_enchants = power_level * 2
    while n_enchants > 0 
        if rng\random(4) == 0
            rng\random_choice(MAJOR_ENCHANTS)(rng, data)
            n_enchants -= 1
        elseif rng\random(4) == 0
            rng\random_choice(MAJOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_DEBUFFS)(rng, data)
        elseif rng\random(8) == 0
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
            rng\random_choice(MINOR_DEBUFFS)(rng, data)
            n_enchants += 1
        else
            rng\random_choice(MINOR_ENCHANTS)(rng, data)
        n_enchants -= 1
    if enchanter
       enchanter(rng, data, power_level)
    table.insert(RANDARTS[power_level], data.name)
    return data

MAX_ENCHANTMENT = 20

apply_enchantment = (rng, data, power_level) ->
    enchantment = power_level
    for i=power_level + 1,MAX_ENCHANTMENT
        if rng\randomf() >= (if data.cooldown then 0.33 else 0.15)
            break
        enchantment += 1
    if enchantment > 0
        data.name = "+#{enchantment} #{data.name}"
        if data.cooldown
            for i=1,enchantment,4
                additive_core_bonus("damage", {1, 1})(rng, data)
            additive_core_bonus("power", {enchantment, enchantment})(rng, data)
            data["resist_modifier"] or= 1
            data["resist_modifier"] /= 1 + 0.05 * enchantment
        elseif rng\randomf() < 0.5
            for i=1,enchantment,4
                additive_core_bonus("magic_reduction", {1, 1})(rng, data)
            additive_core_bonus("magic_resistance", {enchantment, enchantment})(rng, data)
        else
            additive_core_bonus("resistance", {enchantment, enchantment})(rng, data)
        for i=1,2
            for i=1,enchantment,4
                additive_core_bonus("magic_reduction", {1, 1})(rng, data)
            data.shop_cost[i] += math.floor((enchantment ^ 1.5) * 50)

-- Define several randart amulets:
define_amulet_randarts = (rng) ->
    images = get_resource_names("spr_amulets.randarts")
    for i=1,50
        base = {
            name: "Amulet"
            type: "amulet"
            shop_cost: {0, 0}
        }
        Data.equipment_create(define_randart(rng, base, images))

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

-- Define several randart belts:
define_belt_randarts = (rng) ->
    images = get_resource_names("spr_belts.randarts")
    for i=1,50
        base = {
            name: "Belt"
            type: "belt"
            shop_cost: {0, 0}
        }
        Data.equipment_create(define_randart(rng, base, images))

-- Define several randart legwear:
define_legwear_randarts = (rng) ->
    images = get_resource_names("spr_legwear.randarts")
    for i=1,50
        base = {
            name: "Pants"
            type: "legwear"
            shop_cost: {0, 0}
        }
        Data.equipment_create(define_randart(rng, base, images))

-- Define randart stones and arrows:
define_ammo_randarts = (rng) ->
    for e=1,MAX_ENCHANTMENT
        data = table.merge items["Arrow"], {
            name: "+#{e} Arrow"
        }
        data.damage_bonuses = {
            damage: {base: math.floor(e /4) + 2}
            power: {base: e + 2}
        }
        data.spr_item = rng\random_choice {"spr_weapons.steel_arrow1","spr_weapons.steel_arrow2"}
        Data.projectile_create(data)
        table.insert RANDARTS[math.max(1, math.min(3, math.floor(e / 3) + 1))], data.name
    for e=1,MAX_ENCHANTMENT
        data = table.merge items["Stone"], {
            name: "+#{e} Stone"
        }
        data.damage = table.merge data.damage, {
            base: {2 + math.floor(e/3),2 + math.floor(e/3)}
        }
        data.power = table.merge data.damage, {
            base: {2 + e,2 + e}
        }
        data.spr_item = rng\random_choice {"spr_weapons.stone_randart","spr_weapons.stone_randart2", "spr_weapons.stone_randart3"}
        data.spr_attack = data.spr_item
        Data.projectile_create(data)
        table.insert RANDARTS[math.max(1, math.min(3, math.floor(e / 3) + 1))], data.name

define_equipment_randarts = (rng) ->
    -- RNG object just for generating randarts
    -- ATM the following MUST be a deterministic process, because of limitations
    -- in the Lanarts engine. Once we move to a better serialization library
    -- and have a more flexible object system we can move this into the code proper
    -- and not in a phase beforehand.
    rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
    define_ring_randarts(rng)
    define_belt_randarts(rng)
    define_legwear_randarts(rng)
    define_amulet_randarts(rng)
    define_ammo_randarts(rng) 
    candidates = {}
    for name, item in pairs(items)
        -- Judge whether its equipment by a cooldown not being present
        if item.randart_sprites ~= nil and item.cooldown == nil
            append candidates, item
    for item in *candidates
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
    candidates = {}
    for name, item in pairs(items)
        -- Judge whether its a weapon by a cooldown being present
        if item.randart_sprites ~= nil and item.cooldown ~= nil
            append candidates, item
    for item in *candidates
        for i=1,(item.randart_weight or 20) * 2
            template = define_randart(rng, item, item.randart_sprites, apply_enchantment)
            Data.weapon_create(template)

return {:RANDARTS, :MAX_POWER_LEVEL, :define_equipment_randarts, :define_weapon_randarts}
