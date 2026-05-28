local RandartsBonuses = require("items.RandartsBonuses")
local TypeEffectUtils = require("spells.TypeEffectUtils")
local get_resource_names
get_resource_names = function(type)
  local randart_images = require("compiled.Resources").resource_id_list
  return table.filter(randart_images, function(res)
    return res:match(type)
  end)
end
local _get_name_and_description
_get_name_and_description = function(rng, artifact, power_level)
  local lower = artifact:lower()
  local person = rng:random_choice({
    {
      "Paxanarian",
      "A " .. tostring(lower) .. " discovered by the ancient cartographer Paxanarian."
    },
    {
      "Gallanthor",
      "A " .. tostring(lower) .. " requistioned by the legendary mage Gallanthor."
    },
    {
      "Dredd",
      "A " .. tostring(lower) .. " worn by the late tactician Dredd Fiendblood."
    },
    {
      "Ludamad",
      "A " .. tostring(lower) .. " stolen by the jester savant Ludamad."
    },
    {
      "Xom",
      "A " .. tostring(lower) .. " created by Xom upon the grave of his follower."
    },
    {
      "Okawaru",
      "A " .. tostring(lower) .. " gifted by Okawaru to an epic warrior."
    },
    {
      "Trog",
      "A " .. tostring(lower) .. " gifted by Trog upon the scene of 100 deaths."
    },
    {
      "Gilnug",
      "A " .. tostring(lower) .. " created in communion with the fight gods."
    },
    {
      "Gragh",
      "A " .. tostring(lower) .. " crafted with brutish determination."
    },
    {
      "Meeplod",
      "A " .. tostring(lower) .. " crafted over supernaturally hot fire."
    },
    {
      "Nik",
      "A " .. tostring(lower) .. " crafted by a dice roll."
    },
    {
      "Orlov",
      "A " .. tostring(lower) .. " crafted by great strength."
    },
    {
      "Doogal",
      "A " .. tostring(lower) .. " crafted by enchanting magic."
    },
    {
      "Stara",
      "A " .. tostring(lower) .. " crafted by terrible sacrifice."
    },
    {
      "Hopkin",
      "A " .. tostring(lower) .. " crafted by sheer coincidence."
    },
    {
      "Denter",
      "A " .. tostring(lower) .. " crafted by sheer coincidence."
    },
    {
      "Lomen",
      "A " .. tostring(lower) .. " crafted by sheer coincidence."
    },
    {
      "Quantois",
      "A " .. tostring(lower) .. " crafted by sheer coincidence."
    },
    {
      "Sharazard",
      "A " .. tostring(lower) .. " crafted by sheer coincidence."
    },
    {
      "Logic",
      "A mysterious " .. tostring(lower) .. "."
    },
    {
      "Reason",
      "A mysterious " .. tostring(lower) .. "."
    },
    {
      "Forlong",
      "A mysterious " .. tostring(lower) .. "."
    },
    {
      "Silence",
      "A mysterious " .. tostring(lower) .. "."
    },
    {
      "Terror",
      "A mysterious " .. tostring(lower) .. "."
    }
  })
  local power_adjective = ({
    "It is an item of great value.",
    "It is sought after.",
    "It is very sought after.",
    "It is desparately sought after.",
    "Mere knowledge of its existence is prized."
  })[power_level]
  assert(power_adjective ~= nil)
  local trait = rng:random_choice({
    "Anger",
    "Deceit",
    "Vengeance",
    "Confusion",
    "Persuasion",
    "Undoing",
    "Blessing",
    "Curse",
    "Perseverance",
    "Prevention",
    "Preparation",
    "Appropriation",
    "Fluidity",
    "Formidity",
    "Frustration",
    "Rage",
    "Mimickry",
    "Cunning",
    "Silence",
    "Wisdom",
    "Strength",
    "Foreboding",
    "Awe",
    "Reason",
    "Terror",
    "Mistake",
    "Quest",
    "Adventure",
    "Sorrow",
    "Singleton",
    "Savagery"
  })
  return tostring(artifact) .. " of " .. tostring(person[1]) .. "'s " .. tostring(trait), person[2] .. " " .. power_adjective
end
local NAMES_USED = { }
local get_name_and_description
get_name_and_description = function(rng, artifact, power_level)
  for i = 1, 100 do
    local name, description = _get_name_and_description(rng, artifact, power_level)
    if not NAMES_USED[name] then
      NAMES_USED[name] = true
      return name, description
    end
  end
  return error("Unexpected!")
end
local pmod = 1
local LEVEL_RANGES = {
  {
    600 * pmod,
    900 * pmod
  },
  {
    900 * pmod,
    1600 * pmod
  },
  {
    1200 * pmod,
    2500 * pmod
  }
}
local MAX_TRIES = 1000
local INSTANCES = 20
local apply_core_stat_buff
apply_core_stat_buff = function(self)
  if self.rng:randomf() < 0.05 then
    return self:add_bonus(self.rng:random_choice({
      "Speedbolt"
    }))
  end
  return self:add_bonus(self.rng:random_choice({
    "Polymath",
    "Strength",
    "Heart",
    "Mana",
    "Regeneration",
    "Wit",
    "Power",
    "Quickshot",
    "Quickcast",
    "Flurry",
    "Savage",
    "Focus",
    "Shield",
    "Black",
    "White",
    "Green",
    "Red"
  }))
end
local _is_melee_weapon
_is_melee_weapon = function(self)
  return self.base.cooldown ~= nil and self.base.type ~= 'bows'
end
local _is_bow
_is_bow = function(self)
  return self.base.cooldown ~= nil and self.base.type == 'bows'
end
local _is_amulet
_is_amulet = function(self)
  return self.base.type == 'amulet'
end
local _is_jewellery
_is_jewellery = function(self)
  if self.rng:randomf() < 0.1 then
    return true
  end
  return self.base.type == 'amulet' or self.base.type == 'ring'
end
local _is_armour
_is_armour = function(self)
  if self.rng:randomf() < 0.1 then
    return true
  end
  return self.base.type == 'belt' or self.base.type == 'armour' or self.base.type == 'gloves' or self.base.type == 'legwear'
end
local apply_random_effect
apply_random_effect = function(self)
  if self.rng:randomf() < 0.02 or self.base.__method == "weapon_create" then
    return self:add_bonus(self.rng:random_choice({
      "Vampiric",
      "Confusion",
      "Poison",
      "Knockback"
    }))
  end
  if _is_jewellery(self) and self.rng:randomf() < 0.01 then
    return self:add_bonus("RandomSpell")
  elseif _is_armour(self) and self.rng:randomf() < 0.01 then
    return self:add_bonus("Fortified")
  elseif _is_armour(self) and self.rng:randomf() < 0.05 then
    return self:add_bonus("Spiky")
  elseif _is_jewellery(self) and self.rng:randomf() < 0.01 then
    return self:add_bonus("Centaur")
  elseif _is_jewellery(self) and self.rng:randomf() < 0.01 then
    return self:add_bonus("Golem")
  elseif _is_jewellery(self) and self.rng:randomf() < 0.01 then
    return self:add_bonus("Stormcall")
  elseif _is_jewellery(self) and self.rng:randomf() < 0.001 then
    return self:add_bonus("Mummycall")
  end
  return apply_core_stat_buff(self)
end
local apply_misc_stat_buff
apply_misc_stat_buff = function(self)
  return apply_random_effect(self)
end
local apply_buffs
apply_buffs = function(self)
  if self.base.__method == "weapon_create" then
    for i = 1, math.min(self.level, 2) do
      while not apply_random_effect(self) do
        local _ = nil
      end
    end
    return
  end
  for i = 1, self.level do
    while not apply_random_effect(self) do
      local _ = nil
    end
  end
end
local randart_pickup
randart_pickup = function(item, user)
  return play_sound("sound/randart.ogg")
end
local make_bonus_object
make_bonus_object = function(rng, level, base_score, base_entry)
  local min_score, max_score
  do
    local _obj_0 = LEVEL_RANGES[level]
    min_score, max_score = _obj_0[1], _obj_0[2]
  end
  for i = 1, MAX_TRIES do
    if i == MAX_TRIES then
      error("Unable to complete")
    end
    local bonuses = RandartsBonuses.Bonuses.create(rng, level, base_entry)
    if base_entry.cooldown ~= nil then
      bonuses:add_bonus("enchantment")
    end
    apply_buffs(bonuses)
    local success, score = bonuses:validate(min_score - base_score, max_score - base_score)
    if success then
      return bonuses, score + base_score
    end
  end
  return error("UNEXPECTED")
end
local _get_randarts
_get_randarts = function(rng, definer, level, base_entry, base_score, weight, randart_sprites, amount)
  local list
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, amount do
      local bonuses, score = make_bonus_object(rng, level, base_score, base_entry)
      local name, description = get_name_and_description(rng, base_entry.name, level)
      local obj = bonuses:create_definer_object(name, description)
      obj.type = base_entry.type
      obj.shop_cost = {
        score / 3,
        score / 2
      }
      obj.is_randart = true
      obj.spr_item = rng:random_choice(randart_sprites)
      obj.pickup_func = randart_pickup
      definer(obj)
      local _value_0 = obj.name
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    list = _accum_0
  end
  list.chance = weight
  list.randart_list = true
  return list
end
local get_randarts_for_item
get_randarts_for_item = function(rng, definer, level, template)
  local name, base_score, weight, randart_sprites
  name, base_score, weight, randart_sprites = template[1], template[2], template[3], template[4]
  return _get_randarts(rng, definer, level, items[name], base_score, weight, randart_sprites, 20)
end
local _get_randarts_item_type
_get_randarts_item_type = function(spr_path, name, type)
  return function(rng, level, weight)
    local randart_sprites = get_resource_names(spr_path)
    local base_entry = {
      name = name,
      type = type
    }
    return _get_randarts(rng, Data.equipment_create, level, base_entry, 50, weight, randart_sprites, 100)
  end
end
local get_ring_randarts = _get_randarts_item_type("spr_rings.randarts", "Ring", "ring")
local get_belt_randarts = _get_randarts_item_type("spr_belts.randarts", "Belt", "belt")
local get_legwear_randarts = _get_randarts_item_type("spr_legwear.randarts", "Pants", "legwear")
local get_amulet_randarts = _get_randarts_item_type("spr_amulets.randarts", "Amulet", "amulet")
local get_ammo_randarts
get_ammo_randarts = function(rng, level, weight)
  local MAX_ENCHANTMENT = 5 * level
  local list = {
    chance = weight,
    randart_list = true
  }
  for e = 1, MAX_ENCHANTMENT do
    local data = table.merge(items["Arrow"], {
      name = "+" .. tostring(e) .. " Arrow"
    })
    data.damage_bonuses = {
      damage = {
        base = math.floor(e / 4) + 2
      },
      power = {
        base = e + 2
      }
    }
    data.spr_item = rng:random_choice({
      "spr_weapons.steel_arrow1",
      "spr_weapons.steel_arrow2"
    })
    Data.projectile_create(data)
    append(list, data.name)
  end
  for e = 1, MAX_ENCHANTMENT do
    local data = table.merge(items["Stone"], {
      name = "+" .. tostring(e) .. " Stone"
    })
    data.damage = table.merge(data.damage, {
      base = {
        2 + math.floor(e / 3),
        2 + math.floor(e / 3)
      }
    })
    data.power = table.merge(data.damage, {
      base = {
        2 + e,
        2 + e
      }
    })
    data.spr_item = rng:random_choice({
      "spr_weapons.stone_randart",
      "spr_weapons.stone_randart2",
      "spr_weapons.stone_randart3"
    })
    data.spr_attack = data.spr_item
    Data.projectile_create(data)
    append(list, data.name)
  end
  return list
end
local clear_state
clear_state = function()
  return table.clear(NAMES_USED)
end
return {
  get_randarts_for_item = get_randarts_for_item,
  clear_state = clear_state,
  get_ring_randarts = get_ring_randarts,
  get_ammo_randarts = get_ammo_randarts,
  get_amulet_randarts = get_amulet_randarts,
  get_belt_randarts = get_belt_randarts,
  get_legwear_randarts = get_legwear_randarts,
  get_resource_names = get_resource_names
}
