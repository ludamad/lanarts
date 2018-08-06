-- Simple helper module to import all bonuses
{:compile_bonuses, :BONUSES} = require "items.Bonuses"

require "items.BonusesColor"
require "items.BonusesRing"
require "items.BonusesWeapon"
require "items.BonusesSummons"

-- TODO properly calculate shop cost
-- TODO -- piggy back into randarts system - use new tech to enhance RandartsBonuses
for bonus1 in *BONUSES.__keys
    for bonus2 in *BONUSES.__keys
        created = compile_bonuses {
            name: "Ring"
            description: ""
            type: "ring"
            shop_cost: {30, 90}
            spr_item: "spr_rings.ivory"
        }, {bonus1, bonus2}
        -- pretty created
        Data.equipment_create(created)

        created = compile_bonuses items.Mace, {bonus1, bonus2}
        pretty created
        -- pretty created
        Data.weapon_create(created)
