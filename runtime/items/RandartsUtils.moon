RandartsBonuses = require "items.RandartsBonuses"
TypeEffectUtils = require "spells.TypeEffectUtils"

get_resource_names = (type) ->
    randart_images = require("compiled.Resources").resource_id_list
    return table.filter(randart_images, (res) -> res\match(type))

-- Random descriptions:
_get_name_and_description = (rng, artifact, power_level) ->
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
        {"Sharazard", "A #{lower} crafted by sheer coincidence."}
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
        "Perseverance"
        "Prevention"
        "Preparation"
        "Appropriation"
        "Fluidity"
        "Formidity"
        "Frustration"
        "Rage"
        "Mimickry"
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

NAMES_USED = {}
get_name_and_description = (rng, artifact, power_level) ->
    for i=1,100
        name, description = _get_name_and_description(rng, artifact, power_level)
        if not NAMES_USED[name] 
            NAMES_USED[name] = true
            return name, description
    error "Unexpected!"

-- Desired score ranges for levels:
pmod = 1 -- (math.sqrt(#require("core.World").players) - 1) / 8 + 1
LEVEL_RANGES = {
    {600 * pmod, 900 * pmod}
    {900 * pmod, 1600 * pmod}
    {1200 * pmod, 2500 * pmod}
}

MAX_TRIES=1000
INSTANCES = 20

-- The '@' is a RandartsBonuses.Bonuses object
-- The method syntax is used for shorthand 

-- Enchantments:
apply_core_stat_buff = () =>
    --if @rng\randomf() < 0.5
    --    if @rng\randomf() < 0.4
    --        return @add_bonus @rng\random_choice {
    --            "magic_cooldown_multiplier"
    --            "melee_cooldown_multiplier"
    --            "ranged_cooldown_multiplier"
    --            "spell_velocity_multiplier"
    --        }
    --    return @add_bonus @rng\random_choice {
    --        "mp"
    --        "hp"
    --        "hpregen"
    --        "mpregen"
    --        "strength"
    --        "defence"
    --        "willpower"
    --        "magic"
    --    }
    --else
    if @rng\randomf() < 0.05
        return @add_bonus @rng\random_choice {
            --"magic_cooldown_multiplier"
            --"melee_cooldown_multiplier"
            --"ranged_cooldown_multiplier"
            "Speedbolt"
        }
    return @add_bonus @rng\random_choice {
        "Polymath"
        "Strength"
        "Heart"
        "Mana"
        "Regeneration"
        "Wit"
        "Power"
        "Quickshot"
        "Quickcast"
        "Flurry"
        "Savage"
        "Focus"
        "Shield"
        "Black"
        "White"
        "Green"
        "Red"
    }

_is_melee_weapon = () =>
    return @base.cooldown ~= nil and @base.type ~= 'bows'

_is_bow = () =>
    return @base.cooldown ~= nil and @base.type == 'bows'

_is_amulet = () =>
    return @base.type == 'amulet'

_is_jewellery = () =>
    -- Very rarely, assign 'inappropriate' attributes
    if @rng\randomf() < 0.1
        return true
    return @base.type == 'amulet' or @base.type == 'ring'

_is_armour = () =>
    -- Very rarely, assign 'inappropriate' attributes
    if @rng\randomf() < 0.1
        return true
    return @base.type == 'belt' or @base.type == 'armour' or @base.type == 'gloves' or @base.type == 'legwear'

apply_random_effect = () =>
    if @rng\randomf() < 0.02 or @base.__method == "weapon_create"
        return @add_bonus @rng\random_choice {
            "Vampiric"
            "Confusion"
            "Poison"
            "Knockback"
        }
    if _is_jewellery(@) and @rng\randomf() < 0.01
        return @add_bonus "RandomSpell"
    elseif _is_armour(@) and @rng\randomf() < 0.01
        return @add_bonus "Fortified"
    elseif _is_armour(@) and @rng\randomf() < 0.05
        return @add_bonus "Spiky"
    elseif _is_jewellery(@) and @rng\randomf() < 0.01
        return @add_bonus "Centaur"
    elseif _is_jewellery(@) and @rng\randomf() < 0.01
        return @add_bonus "Golem"
    elseif _is_jewellery(@) and @rng\randomf() < 0.01
        return @add_bonus "Stormcall"
    elseif _is_jewellery(@) and @rng\randomf() < 0.001
        return @add_bonus "Mummycall"
    return apply_core_stat_buff(@)

apply_misc_stat_buff = () =>
    return apply_random_effect(@)

apply_buffs = () =>
    if @base.__method == "weapon_create"
        for i=1,math.min(@level, 2)
            while not apply_random_effect(@)
                nil
        return
    for i=1,@level
        while not apply_random_effect(@)
            nil

randart_pickup = (item, user) ->
    play_sound "sound/randart.ogg"

make_bonus_object = (rng, level, base_score, base_entry) ->
    {min_score, max_score} = LEVEL_RANGES[level]
    for i=1,MAX_TRIES
        if i == MAX_TRIES
            error("Unable to complete")
        bonuses = RandartsBonuses.Bonuses.create(rng, level, base_entry)
        if base_entry.cooldown ~= nil -- 
            -- Apply enchantment only if we detect weapon
            bonuses\add_bonus("enchantment")
        apply_buffs(bonuses)
        success, score = bonuses\validate(min_score - base_score, max_score - base_score)
        print base_entry.name, success, score
        if success
            return bonuses, score + base_score
    error("UNEXPECTED")

_get_randarts = (rng, definer, level, base_entry, base_score, weight, randart_sprites, amount) ->
    list = for i=1,amount 
        bonuses, score = make_bonus_object(rng, level, base_score, base_entry)
        --log_verbose "Completed with #{score} score for level #{level}."
        name, description = get_name_and_description(rng, base_entry.name, level)
        obj = bonuses\create_definer_object(name, description)
        obj.type = base_entry.type
        obj.shop_cost = {score/2, score}
        obj.is_randart = true
        obj.spr_item = rng\random_choice(randart_sprites)
        obj.pickup_func = randart_pickup
        definer(obj)
        obj.name
    list.chance = weight
    -- Simply a list of names:
    list.randart_list = true
    return list

get_randarts_for_item = (rng, definer, level, template) ->
    {name, base_score, weight, randart_sprites} = template
    return _get_randarts(rng, definer, level, items[name], base_score, weight, randart_sprites, 20)

_get_randarts_item_type = (spr_path, name, type) -> (rng, level, weight) ->
    randart_sprites = get_resource_names(spr_path)
    base_entry = {:name, :type}
    return _get_randarts(rng, Data.equipment_create, level, base_entry, 50, weight, randart_sprites, 100)

get_ring_randarts = _get_randarts_item_type("spr_rings.randarts", "Ring", "ring")
get_belt_randarts = _get_randarts_item_type("spr_belts.randarts", "Belt", "belt")
get_legwear_randarts = _get_randarts_item_type("spr_legwear.randarts", "Pants", "legwear")
get_amulet_randarts = _get_randarts_item_type("spr_amulets.randarts", "Amulet", "amulet")

-- Define randart stones and arrows:
get_ammo_randarts = (rng, level, weight) ->
    MAX_ENCHANTMENT = 5 * level
    list = {chance: weight, randart_list: true}
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
        append list, data.name
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
        append list, data.name
    return list

clear_state = () ->
    table.clear(NAMES_USED)

return {:get_randarts_for_item, :clear_state, :get_ring_randarts, :get_ammo_randarts
    :get_amulet_randarts,
    :get_belt_randarts, :get_legwear_randarts, :get_resource_names}
