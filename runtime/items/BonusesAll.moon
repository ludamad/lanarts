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

generate = (name) ->
    parts = name\split(" ")
    i = 1
    bonuses = {}
    while i <= #parts
        potential_bonus = parts[i]
        if not BONUSES[potential_bonus]
            break
        append bonuses, potential_bonus
        i += 1
    base_name = table.concat [parts[j] for j=i,#parts], ' '
    base_obj = items[base_name]
    if not base_obj
        return
    
    -- "Do not have an item definition for '#{base_name}'!"
    definer_method = Data[base_obj.__method]

    created = compile_bonuses(base_obj, bonuses)
    name_hash = created.name\hash()

    if created.type == "ring"
        created.spr_item = KEY_SPRITES[name_hash % #KEY_SPRITES + 1]

    definer_method(created)
    

return {:generate}
