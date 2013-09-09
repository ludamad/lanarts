local Apts = import "@stats.AptitudeTypes"
local Traits = import ".ItemTraits"
local ring_define = (import ".ItemUtils").ring_define

ring_define {
    name = "Ring of Spells",
    description = "A small but sharp blade, adept at stabbing purposes.",

    gold_worth = 15, difficulty = 0,
    effectiveness = 6, damage = 4, delay = 1.0
}
