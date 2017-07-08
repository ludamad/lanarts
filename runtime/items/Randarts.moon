RandartsUtils = require "items.RandartsUtils"

HARDCODED_RANDARTS_SEED = 0xBADBABE
MAX_POWER_LEVEL = 3
RANDARTS = {}
for level=1,MAX_POWER_LEVEL
    RANDARTS[level] = {}

COMMON = 100
UNCOMMON = 50
SEMIRARE = 20
RARE = 10

WEAPON_BASES = {
    -- Name, base value, weight, sprites
    -- Slashing base weapons
    {"Dagger", 50, COMMON, (for i=1,2 do "spr_weapons.randart_dagger"..i)}
    {"Short Sword", 100, SEMIRARE, (for i=1,2 do "spr_weapons.randart_short_sword"..i)}
    -- Bludgeoning base weapons
    {"Mace", 100, COMMON, (for i=1,3 do "spr_weapons.randart_mace"..i)}
    -- Piercing base weapons 
    {"Short Bow", 100, SEMIRARE, {"spr_weapons.randart_shortbow"}}
    {"Long Bow", 100, RARE, {"spr_weapons.urand_krishna", "spr_weapons.longbow3"}}
    -- Unclassified base weapons
    {"Wizard's Staff", 100, UNCOMMON, {"spr_weapons.staff_mummy"}}
    {"Flaming Scimitar", 200, RARE, {"spr_weapons.urand_flaming_death"}}
    {"Ice Axe", 200, RARE, {"spr_weapons.urand_frostbite"}}
    {"Vine Staff", 200, RARE, {"spr_weapons.spwpn_staff_of_olgreb"}}
    {"Mace of Light", 100, RARE, {"spr_weapons.urand_brilliance"}}
    {"Dark Scythe", 200, RARE, {"spr_weapons.spwpn_scythe_of_curses"}}
}

ARMOUR_BASES = {
    -- Body armour
    {"Leather Armour", 100, UNCOMMON, {"spr_armour.leather_armour3"}}
    {"Chainmail", 200, SEMIRARE, {"spr_armour.chain_mail2", "spr_armour.chain_mail3"}}
    --{"Crystalline Plate", 500, RARE, {"spr_armour.crystal_plate3"}}
    {"Platemail", 500, RARE, {"spr_armour.randart_plate"}} 
    {"Robe", 100, UNCOMMON, (for i=1,2 do "spr_armour.randart_robe"..i)} 
    -- Gloves
    {"Steel Gloves", 100, RARE, (for i=1,4 do "spr_armour.randart_glove"..i)}
    -- Boots
    {"Leather Boots", 100, UNCOMMON, {"spr_boots.randart_leather_boots"}}
    -- Headgear
    {"Iron Helmet", 100, UNCOMMON, {"spr_armour.randart_helmet", "spr_armour.helmet_ego1", "spr_armour.helmet_ego2", "spr_armour.helmet_ego3", "spr_armour.helmet_ego4", "spr_armour.helmet_art1"}}
    {"Wizard's Hat", 100, UNCOMMON, (for i=2,5 do "spr_armour.hat#{i}")}
    -- Rings generated without a base
    -- Amulets generated without a base
    -- Ammo generated separately
    -- Legwear generated separately
}

define_equipment_randarts = (rng, level) ->
    log_verbose "Defining equipment randarts of power #{level}."
    -- RNG object just for generating randarts
    -- ATM the following MUST be a deterministic process, because of limitations
    -- in the Lanarts engine. Once we move to a better serialization library
    -- and have a more flexible object system we can move this into the code proper
    -- and not in a phase beforehand.

    R = RANDARTS[level]
    append R, RandartsUtils.get_ring_randarts(rng, level, 75)
    append R, RandartsUtils.get_belt_randarts(rng, level, 50)
    append R, RandartsUtils.get_legwear_randarts(rng, level, 50)
    append R, RandartsUtils.get_amulet_randarts(rng, level, 50)

    -- Define randart armour pieces:
    append R, for base in *ARMOUR_BASES
        RandartsUtils.get_randarts_for_item(rng, Data.equipment_create, level, base)
    R[#R].chance = 150

    -- Define randart weapons:
    append R, for base in *WEAPON_BASES
        RandartsUtils.get_randarts_for_item(rng, Data.weapon_create, level, base)
    R[#R].chance = 100

define_randarts = () ->
    rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
    RandartsUtils.clear_state()
    for level=1,MAX_POWER_LEVEL
        table.clear(RANDARTS[level])
        --define_weapon_randarts(rng)
        define_equipment_randarts(rng, level)

return {:RANDARTS, :MAX_POWER_LEVEL, :define_randarts}
