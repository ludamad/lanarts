local RandartsUtils = require("items.RandartsUtils")
local HARDCODED_RANDARTS_SEED = 0xBADBABE
local MAX_POWER_LEVEL = 3
local RANDARTS = { }
for level = 1, MAX_POWER_LEVEL do
  RANDARTS[level] = { }
end
local COMMON = 100
local UNCOMMON = 50
local SEMIRARE = 20
local RARE = 10
local WEAPON_BASES = {
  {
    "Dagger",
    50,
    COMMON,
    ((function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 2 do
        _accum_0[_len_0] = "spr_weapons.randart_dagger" .. i
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
  },
  {
    "Short Sword",
    100,
    SEMIRARE,
    ((function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 2 do
        _accum_0[_len_0] = "spr_weapons.randart_short_sword" .. i
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
  },
  {
    "Mace",
    100,
    COMMON,
    ((function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 3 do
        _accum_0[_len_0] = "spr_weapons.randart_mace" .. i
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
  },
  {
    "Short Bow",
    100,
    SEMIRARE,
    {
      "spr_weapons.randart_shortbow"
    }
  },
  {
    "Long Bow",
    100,
    RARE,
    {
      "spr_weapons.urand_krishna",
      "spr_weapons.longbow3"
    }
  },
  {
    "Flaming Scimitar",
    200,
    RARE,
    {
      "spr_weapons.urand_flaming_death"
    }
  },
  {
    "Ice Axe",
    200,
    RARE,
    {
      "spr_weapons.urand_frostbite"
    }
  },
  {
    "Mace of Light",
    100,
    RARE,
    {
      "spr_weapons.urand_brilliance"
    }
  },
  {
    "Dark Scythe",
    200,
    RARE,
    {
      "spr_weapons.spwpn_scythe_of_curses"
    }
  }
}
local ARMOUR_BASES = {
  {
    "Leather Armour",
    100,
    UNCOMMON,
    {
      "spr_armour.leather_armour3"
    }
  },
  {
    "Chainmail",
    200,
    UNCOMMON / 2,
    {
      "spr_armour.chain_mail2",
      "spr_armour.chain_mail3"
    }
  },
  {
    "Platemail",
    500,
    UNCOMMON / 5,
    {
      "spr_armour.randart_plate"
    }
  },
  {
    "Robe",
    100,
    UNCOMMON * 2,
    ((function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 2 do
        _accum_0[_len_0] = "spr_armour.randart_robe" .. i
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
  },
  {
    "Steel Gloves",
    100,
    RARE,
    ((function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 1, 4 do
        _accum_0[_len_0] = "spr_armour.randart_glove" .. i
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
  },
  {
    "Leather Boots",
    100,
    RARE,
    {
      "spr_boots.randart_leather_boots"
    }
  },
  {
    "Iron Helmet",
    100,
    RARE,
    {
      "spr_armour.randart_helmet",
      "spr_armour.helmet_ego1",
      "spr_armour.helmet_ego2",
      "spr_armour.helmet_ego3",
      "spr_armour.helmet_ego4",
      "spr_armour.helmet_art1"
    }
  },
  {
    "Wizard's Hat",
    100,
    UNCOMMON,
    ((function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 2, 5 do
        _accum_0[_len_0] = "spr_armour.hat" .. tostring(i)
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)())
  }
}
local define_equipment_randarts
define_equipment_randarts = function(rng, level)
  log_verbose("Defining equipment randarts of power " .. tostring(level) .. ".")
  local R = RANDARTS[level]
  append(R, RandartsUtils.get_ring_randarts(rng, level, 110))
  append(R, RandartsUtils.get_belt_randarts(rng, level, 50))
  append(R, RandartsUtils.get_legwear_randarts(rng, level, 50))
  append(R, RandartsUtils.get_amulet_randarts(rng, level, 75))
  append(R, (function()
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #ARMOUR_BASES do
      local base = ARMOUR_BASES[_index_0]
      _accum_0[_len_0] = RandartsUtils.get_randarts_for_item(rng, Data.equipment_create, level, base)
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)())
  R[#R].chance = 150
  append(R, (function()
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #WEAPON_BASES do
      local base = WEAPON_BASES[_index_0]
      _accum_0[_len_0] = RandartsUtils.get_randarts_for_item(rng, Data.weapon_create, level, base)
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)())
  R[#R].chance = 100
end
local define_randarts
define_randarts = function()
  local rng = require("mtwist").create(HARDCODED_RANDARTS_SEED)
  RandartsUtils.clear_state()
  for level = 1, MAX_POWER_LEVEL do
    table.clear(RANDARTS[level])
    define_equipment_randarts(rng, level)
  end
end
return {
  RANDARTS = RANDARTS,
  MAX_POWER_LEVEL = MAX_POWER_LEVEL,
  define_randarts = define_randarts
}
