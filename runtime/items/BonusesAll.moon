-- Simple helper module to import all bonuses
{:compile_bonuses, :BONUSES} = require "items.Bonuses"

require "items.BonusesArmour"
require "items.BonusesColor"
require "items.BonusesRing"
require "items.BonusesWeapon"
require "items.BonusesSummons"
require "items.BonusesStats"

KEY_SPRITES = {
    "spr_rings.agate"
    "spr_rings.big-skull-ring"
    "spr_rings.brass"
    "spr_rings.bronze"
    "spr_rings.clay"
    "spr_rings.copper"
    "spr_rings.coral"
    "spr_rings.diamond"
    "spr_rings.emerald"
    "spr_rings.ethereal"
    "spr_rings.glass"
    "spr_rings.gold_blue"
    "spr_rings.gold_green"
    "spr_rings.gold"
    "spr_rings.gold_red"
    "spr_rings.granite"
    "spr_rings.iron"
    "spr_rings.ivory"
    "spr_rings.jade"
    "spr_rings.moonstone"
    "spr_rings.opal"
    "spr_rings.pearl"
    "spr_rings.plain_black"
    "spr_rings.plain_red"
    "spr_rings.plain_yellow"
    "spr_rings.ruby"
    "spr_rings.silver"
    "spr_rings.steel"
    "spr_rings.tiger_eye"
    "spr_rings.tourmaline"
    "spr_rings.wooden"
}

-- TODO properly calculate shop cost
-- TODO -- piggy back into randarts system - use new tech to enhance RandartsBonuses
USED = {}
for bonus1 in *BONUSES.__keys
    define = (bonus2) ->
        bonuses = {bonus1, bonus2}
        table.sort(bonuses)
        created = compile_bonuses {
            name: "Ring"
            description: ""
            type: "ring"
            shop_cost: {30, 90}
            spr_item: "spr_rings.ivory"
        }, bonuses
        if USED[created.name]
            return
        created.spr_item = KEY_SPRITES[created.name\hash() % #KEY_SPRITES + 1]
        Data.equipment_create(created)
        USED[created.name] = true

        created = compile_bonuses items.Mace, {bonus1, bonus2}
        Data.weapon_create(created)

    for bonus2 in *BONUSES.__keys
        define bonus2
    define nil
