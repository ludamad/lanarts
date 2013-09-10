local Apts = import "@stats.AptitudeTypes"
local Attacks = import "@Attacks"
local Traits = import ".ItemTraits"
local Utils = import ".ItemUtils"
local Proficiency = import "@Proficiency"

local function bonus_str(val)
    if not val then return "+0" end
    if val >= 0 then return '+'..val end
    return val
end

local function base_weapon_init(self)
    local P = Proficiency
    local eff_bonus, dam_bonus = self.effectiveness_bonus or random(-2,3), self.damage_bonus or random(-2,10)
    self.effectiveness_bonus, self.damage_bonus = eff_bonus, dam_bonus
    local difficulty = random_round((math.abs(eff_bonus) * math.abs(dam_bonus)) ^ .85) + random(-1,3) + (self.difficulty or 0)

    local types = Utils.filter_main_aptitudes(self.types)
    table.insert(types, Apts.WEAPON_IDENTIFICATION)
    self.identify_requirements = {P.proficiency_requirement_create(types, difficulty)}
    self.unidentified_name = self.name
    self.name = ("%s,%s %s"):format(bonus_str(self.effectiveness_bonus), bonus_str(self.damage_bonus), self.unidentified_name)

    self.attack = table.deep_clone(self.attack)
    Attacks.attack_add_effectiveness(self.attack, eff_bonus)
    Attacks.attack_add_damage(self.attack, dam_bonus)
end

local function weapon_define(args)
    args.needs_identification = true
    args.on_init = base_weapon_init
    return Utils.weapon_define(args)
end

weapon_define {
    name = "Dagger",
    description = "A small but sharp blade, adept at stabbing purposes.",

    gold_worth = 15, difficulty = 0,
    types = {Apts.MELEE, Apts.PIERCING, Apts.BLADE},
    effectiveness = 6, damage = 4, delay = 1.0
}

weapon_define {
    name = "Short Sword",
    description = "A small, light sword.",

    gold_worth = 35, difficulty = 1,
    types = {Apts.MELEE, Apts.PIERCING, Apts.BLADE},
    effectiveness = 4, damage = 6, delay = 1.1
}

weapon_define {
    name = "Long Sword",
    description = "A large trusty sword.",

    gold_worth = 80, difficulty = 3,
    types = {Apts.MELEE, Apts.SLASHING, Apts.BLADE},
    effectiveness = 1, damage = 10, delay = 1.4
}

weapon_define {
    name = "Great Sword",
    description = "An oversized brutish sword.",

    gold_worth = 120, difficulty = 5,
    types = {Apts.MELEE, Apts.SLASHING, Apts.BLADE},
    effectiveness = -3, damage = 16, delay = 1.6
}

weapon_define {
    name = "Hand Axe",
    description = "A light, small and sturdy axe.",

    gold_worth = 20, difficulty = 0,
    types = {Apts.MELEE, Apts.SLASHING, Apts.AXE},
    effectiveness = 3, damage = 7, delay = 1.3
}