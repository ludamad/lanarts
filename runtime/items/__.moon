-- Make an random piece of equipment
-- Equipment is created using various generation schemes, 
-- and then is created based on selection from score range.

DataWrapped = require "DataWrapped"

COMMON = 100
UNCOMMON = 50
SEMIRARE = 20
RARE = 10

-- Desired score ranges for levels:
LEVEL_RANGES = {
    {100, 500}
    {300, 900}
    {500, 1200}
}

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
}

EQUIPMENT_BASES = {
    -- Body armour
    {"Leather Armour", 100, COMMON, {"spr_armour.leather_armour3"}}
    {"Chainmail", 200, SEMIRARE, {"spr_armour.chain_mail2", "spr_armour.chain_mail3"}}
    {"Crystalline Plate", 500, RARE, {"spr_armour.crystal_plate3"}}
    {"Platemail", 500, RARE, {"spr_armour.randart_plate"}} 
    {"Robe", 100, COMMON, (for i=1,2 do "spr_armour.randart_robe"..i)} 
    -- Gloves
    {"Steel Gloves", 100, RARE, (for i=1,4 do "spr_armour.randart_glove"..i)}
    -- Boots
    {"Leather Boots", 100, SEMIRARE, {"spr_boots.randart_leather_boots"}}
    -- Headgear
    {"Iron Helmet", 100, SEMIRARE, {"spr_boots.randart_helmet"}}
    {"Wizard's Hat", 100, SEMIRARE, {"spr_boots.randart_hat"}}
    -- Rings generated without a base
    -- Amulets generated without a base
    -- Ammo generated separately
}

MAX_TRIES=1000
INSTANCES = 20

define_randarts = (rng) ->
    for t in *RANDART_LEVELS do table.clear(t)
    for randart_level=1,RANDART_LEVELS
        list = RANDART_LEVELS[randart_level]
        {score_min, score_max} = LEVEL_RANGES[randart_level]

        equipment_choices = {chance: 100}
        for {name, score, weight, randart_sprites} in pairs EQUIPMENT_BASES
            instances = {}
            for i=1,MAX_TRIES
                if i == MAX_TRIES
                    error("Unable to complete")

                log_verbose "Completed with #{score} score for level #{randart_level}."

            while score < score_min
            EquipmentBonuses.create(rng)
            append equipment_choices, 

