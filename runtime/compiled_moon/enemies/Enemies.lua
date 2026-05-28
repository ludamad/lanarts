local Map = require("core.Map")
local ObjectUtils = require("objects.ObjectUtils")
local DataW = require("DataWrapped")
local GameObject = require("core.GameObject")
local World = require("core.World")
local GameState = require("core.GameState")
local Display = require("core.Display")
local TypeEffectUtils = require("spells.TypeEffectUtils")
local random_xy_near
random_xy_near = function(obj)
  return {
    obj.x + random(-32, 32),
    obj.y + random(-32, 32)
  }
end
local draw_boss_name
draw_boss_name = function(self)
  if Map.object_visible(self) then
    local color = with_alpha(COL_WHITE, 0.75)
    local progress = math.abs(1 - (GameState.frame % 100) / 50)
    local screen_xy = Display.to_screen_xy({
      self.x,
      self.y - 24 - 8 * progress
    })
    return bitmap_draw({
      color = color,
      origin = Display.CENTER
    }, screen_xy, self.name)
  end
end
DataW.enemy_create({
  name = "Crypt Keeper",
  sprite = "spr_enemies.undead.lich",
  radius = 11,
  xpaward = 75,
  appear_message = "A frightful Crypt Keeper emerges!",
  defeat_message = "The Crypt Keeper has been destroyed.",
  weapon = {
    cooldown = 40,
    damage = 16
  },
  types = {
    "Black"
  },
  stats = {
    hp = 50,
    hpregen = 0.03,
    movespeed = 2,
    strength = 3,
    defence = 8,
    magic = 5,
    willpower = 8
  },
  effects_active = {
    {
      "Pain Aura",
      {
        damage = 15,
        range = 90
      }
    }
  }
})
DataW.enemy_create({
  name = "Mana Sapper",
  sprite = "spr_enemies.demons.manasapper",
  radius = 11,
  xpaward = 65,
  appear_message = "A mana sapper comes onto the scene!",
  defeat_message = "The mana sapper dies.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        projectile = "Minor Missile"
      }
    },
    hp = 80,
    hpregen = 0.03,
    movespeed = 1,
    magic = 35,
    defence = 18,
    strength = 45,
    willpower = 18
  },
  effects_active = {
    "Sap Aura"
  }
})
DataW.enemy_create({
  name = "Gaseous Ghost",
  sprite = "spr_enemies.undead.hungry_ghost",
  radius = 11,
  xpaward = 75,
  appear_message = "A gaseous ghost appears!",
  defeat_message = "The vile ghost is gone.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      },
      {
        projectile = "Mephitize"
      }
    },
    hp = 50,
    hpregen = 0.03,
    movespeed = 1,
    strength = 15,
    defence = 5,
    magic = 25,
    willpower = 5
  },
  effects_active = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.25
      }
    }
  }
})
DataW.enemy_create({
  name = "Spectral Beast",
  sprite = "spr_enemies.undead.spectral_lizard",
  radius = 11,
  xpaward = 10,
  appear_message = "A spectral beast is summoned!",
  defeat_message = "The spectre fades.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 40,
    hpregen = 0,
    movespeed = 4,
    strength = 5,
    defence = 8,
    magic = 10,
    willpower = 8
  }
})
DataW.enemy_create({
  name = "Flying Skull",
  sprite = "spr_enemies.undead.flying_skull",
  radius = 11,
  xpaward = 10,
  appear_message = "A flying skull appears!",
  defeat_message = "The flying skull fades.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 40,
    hpregen = 0,
    movespeed = 4,
    strength = 5,
    defence = 8,
    magic = 10,
    willpower = 8
  }
})
DataW.enemy_create({
  name = "Mummy",
  sprite = "spr_enemies.undead.mummy",
  radius = 11,
  xpaward = 10,
  appear_message = "A summoned mummy appears!",
  defeat_message = "The summoned mummy is put to rest.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 10,
    hpregen = 0,
    movespeed = 3.5,
    strength = 10,
    defence = 0,
    magic = 10,
    willpower = 0
  },
  effects_active = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.1
      }
    }
  }
})
local summoner_base
summoner_base = function(monster, amount, rate, kill_time, duration)
  if rate == nil then
    rate = 60
  end
  if kill_time == nil then
    kill_time = 250
  end
  if duration == nil then
    duration = 150
  end
  return function(data)
    return table.merge(data, {
      init_func = function(self)
        self.n_steps = 0
        self.summon_rate = rate
        self.summoned = OrderedDict()
        self.n_summons = 0
      end,
      step_func = function(self)
        self.n_summons = 0
        for mon, time in strictpairs(self.summoned) do
          if time > kill_time then
            mon:direct_damage(mon.stats.hp + 1)
          end
          if mon.destroyed then
            event_log("Desummoning monster (%s, id=%d) at (%.2f, %.2f)", mon.name, math.max(0, mon.id), mon.x, mon.y)
            self.summoned[mon] = nil
          else
            self.summoned[mon] = self.summoned[mon] + 1
            self.n_summons = self.n_summons + 1
          end
        end
        if Map.object_visible(self) and not (self:has_effect("Summoning")) and self.n_summons < amount then
          if #Map.allies_list(self) == 0 then
            return
          end
          if self.n_steps > self.summon_rate then
            local eff = self:add_effect("Summoning", 20)
            eff.monster = ((function()
              if type(monster) == "string" then
                return monster
              else
                return random_choice(monster)
              end
            end)())
            eff.duration = duration
            self.n_steps = 0
          else
            self.n_steps = self.n_steps + 1
          end
        end
      end
    })
  end
end
DataW.enemy_create(summoner_base("Mummy", 3, 90)({
  name = "Mummoner",
  sprite = "spr_enemies.undead.greater_mummy",
  radius = 11,
  xpaward = 75,
  appear_message = "You find a frightful Mummoner!",
  defeat_message = "The mummoner has been destroyed.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 75,
    hpregen = 0.03,
    movespeed = 1,
    strength = 15,
    defence = 8,
    magic = 10,
    willpower = 8
  }
}))
DataW.enemy_create({
  name = "Queen Bee",
  sprite = "spr_enemies.animals.queen_bee",
  death_sprite = "green blood",
  radius = 10,
  xpaward = 100,
  appear_message = "A queen bee appears!",
  defeat_message = "The queen bee has buzzed its last buzz.",
  types = {
    "Green"
  },
  stats = {
    attacks = {
      {
        weapon = "Fast Melee"
      }
    },
    hp = 60,
    hpregen = 0.1,
    movespeed = 3,
    strength = 20,
    defence = 0,
    willpower = 0
  },
  effects_active = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.25
      }
    }
  }
})
DataW.enemy_create({
  name = "Giant Bee",
  sprite = "spr_enemies.animals.killer_bee",
  death_sprite = "green blood",
  radius = 10,
  xpaward = 20,
  appear_message = "A giant killer bee appears!",
  defeat_message = "The giant bee has buzzed its last buzz.",
  types = {
    "Green"
  },
  stats = {
    attacks = {
      {
        weapon = "Fast Melee"
      }
    },
    hp = 20,
    hpregen = 0.1,
    movespeed = 3,
    strength = 25,
    defence = 0,
    willpower = 0
  }
})
DataW.enemy_create({
  name = "Poison Bee",
  sprite = "spr_enemies.animals.poison_bee",
  death_sprite = "green blood",
  radius = 10,
  xpaward = 40,
  appear_message = "A giant killer bee appears!",
  defeat_message = "The giant bee has buzzed its last buzz.",
  types = {
    "Green"
  },
  stats = {
    attacks = {
      {
        weapon = "Fast Melee"
      }
    },
    hp = 40,
    hpregen = 0.1,
    movespeed = 3,
    strength = 25,
    defence = 0,
    willpower = 0
  },
  effects_active = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.25
      }
    }
  }
})
DataW.enemy_create(summoner_base("Imp", 1, 140, 70)({
  name = "Waxanarian",
  sprite = "spr_enemies.bosses.boss_bee",
  radius = 20,
  xpaward = 150,
  appear_message = "The demonic bee Waxanarian!",
  defeat_message = "Waxanarian has been vanquished.",
  unique = true,
  types = {
    "Green",
    "Black"
  },
  init_func = enemy_berserker_init,
  step_func = enemy_berserker_step,
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 200,
    hpregen = 0.125,
    movespeed = 5,
    strength = 25,
    magic = 10,
    defence = 8,
    willpower = 8
  },
  on_post_draw = draw_boss_name,
  death_func = function(self)
    local item = random_choice({
      "Will Scroll",
      "Strength Scroll",
      "Defence Scroll",
      "Magic Scroll"
    })
    ObjectUtils.spawn_item_near(self, item, 1)
    ObjectUtils.spawn_item_near(self, "Swarm Lanart", 1)
    local ItemUtils = require("maps.ItemUtils")
    local type, amount
    do
      local _obj_0 = ItemUtils.randart_generate(1)
      type, amount = _obj_0.type, _obj_0.amount
    end
    return ObjectUtils.spawn_item_near(self, type, amount)
  end
}))
DataW.enemy_create({
  name = "Mouther",
  sprite = "spr_enemies.animals.tyrant_leech",
  radius = 11,
  xpaward = 25,
  appear_message = "You hear a mouther start screeching!",
  defeat_message = "The mouther has been shut.",
  types = {
    "Green",
    "Black"
  },
  stats = {
    hp = 40,
    hpregen = 0.01,
    movespeed = 1,
    strength = 25,
    defence = 0,
    magic = 20,
    willpower = 0
  },
  weapon = {
    cooldown = 105,
    range = 300,
    alt_action = function(self, xy, target)
      local _list_0 = Map.radius_collision_check(self.map, 8, self.xy)
      for _index_0 = 1, #_list_0 do
        local obj = _list_0[_index_0]
        if obj.is_enemy == false then
          return
        end
      end
      return self:projectile_attack("Mephitize")
    end
  }
})
DataW.enemy_create({
  name = "Black Mamba",
  sprite = "spr_enemies.animals.black_mamba",
  death_sprite = "blood",
  radius = 10,
  xpaward = 10,
  appear_message = "A poisonous black mamba slithers onto the scene!",
  defeat_message = "The black mamba is dead.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 40,
    damage = 12
  },
  stats = {
    hp = 40,
    hpregen = 0.04,
    movespeed = 3.5,
    strength = 3,
    defence = 0,
    willpower = 0
  },
  effects_active = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.25
      }
    }
  }
})
DataW.enemy_create({
  name = "Clown Mage",
  sprite = "spr_enemies.humanoid.killer_klown_purple",
  radius = 11,
  xpaward = 30,
  appear_message = "A clown mage appears!",
  defeat_message = "The clown mage is defeated.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        projectile = "Chain Lightning"
      }
    },
    hp = 65,
    hpregen = 0.03,
    movespeed = 3,
    strength = 0,
    defence = 0,
    magic = 25,
    willpower = 0
  },
  effects_active = {
    "StopOnceInRange",
    "Enraging"
  }
})
DataW.enemy_create({
  name = "Elephant",
  sprite = "spr_enemies.animals.elephant",
  death_sprite = "blood",
  radius = 15,
  xpaward = 50,
  appear_message = "A loud elephant!",
  defeat_message = "The elephant is dead.",
  types = {
    "Green"
  },
  stats = {
    attacks = {
      {
        weapon = "Slow Melee"
      }
    },
    hp = 70,
    hpregen = 0.05,
    movespeed = 2,
    strength = 20,
    defence = 8,
    willpower = 8
  },
  effects_active = {
    "Enraging"
  }
})
DataW.enemy_create({
  name = "Sheep",
  sprite = "spr_enemies.animals.sheep",
  death_sprite = "blood",
  radius = 14,
  xpaward = 10,
  appear_message = "A hostile sheep approaches!",
  defeat_message = "The sheep recieves a fatal blow.",
  types = {
    "Green"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 20,
    hpregen = 0.05,
    movespeed = 2,
    strength = 20,
    defence = 8,
    willpower = 8
  },
  effects_active = {
    "Enraging"
  }
})
DataW.enemy_create(summoner_base("Fire Bat", 3, 160, 100)({
  name = "Purple Dragon",
  appear_message = "A frighteningly large purple dragon comes into view!",
  defeat_message = "You have slain the purple dragon!",
  sprite = "purple dragon",
  radius = 27,
  xpaward = 300,
  types = {
    "Red"
  },
  unique = true,
  stats = {
    hp = 220,
    hpregen = 0.04,
    movespeed = 2.5,
    strength = 20,
    magic = 20,
    defence = 15,
    willpower = 15
  },
  projectile = {
    weapon_class = "magic",
    can_wall_bounce = true,
    damage_type = {
      magic = 0.5,
      physical = 0.5
    },
    range = 1150 * 3,
    radius = 11,
    power = {
      base = {
        15,
        15
      }
    },
    cooldown = 200,
    speed = 8,
    spr_attack = "spr_effects.dragon_fireball",
    on_map_init = function(self)
      self.n_bounces = 3
    end,
    on_wall_bounce = function(self)
      self.n_bounces = self.n_bounces - 1
      if self.n_bounces < 0 then
        return GameObject.destroy(self)
      end
    end
  },
  resistances = {
    Red = 1,
    Blue = -1.25,
    Green = 0
  },
  init_func = function(self)
    self.timeout = 0
  end,
  step_func = function(self)
    self.timeout = self.timeout - 1
    if self.timeout <= 0 then
      self:projectile_attack("Purple Dragon Projectile", random_xy_near(self))
      self.timeout = 60 * 5
    end
  end,
  on_post_draw = draw_boss_name,
  death_func = function(self)
    local ItemUtils = require("maps.ItemUtils")
    ObjectUtils.spawn_item_near(self, 'Dragon Lanart', 1)
    for i = 1, 1 do
      local type, amount
      do
        local _obj_0 = ItemUtils.randart_generate(1)
        type, amount = _obj_0.type, _obj_0.amount
      end
      ObjectUtils.spawn_item_near(self, type, amount)
    end
    local item = random_choice({
      "Will Scroll",
      "Strength Scroll",
      "Defence Scroll",
      "Magic Scroll"
    })
    return ObjectUtils.spawn_item_near(self, item, 1)
  end
}))
DataW.enemy_create({
  name = "Red Dragon",
  appear_message = "A frighteningly large red dragon comes into view!",
  defeat_message = "You have slain the red dragon!",
  sprite = "red dragon",
  radius = 27,
  xpaward = 150,
  unique = true,
  types = {
    "Red"
  },
  stats = {
    hp = 205,
    hpregen = 0.1,
    movespeed = 2.5,
    strength = 35,
    magic = 30,
    defence = 15,
    willpower = 15
  },
  projectile = {
    weapon_class = "magic",
    can_wall_bounce = true,
    damage_type = {
      magic = 0.5,
      physical = 0.5
    },
    range = 1150,
    radius = 11,
    cooldown = 40,
    speed = 8,
    spr_attack = "large fire"
  },
  on_post_draw = draw_boss_name,
  death_func = function(self)
    local ItemUtils = require("maps.ItemUtils")
    ObjectUtils.spawn_item_near(self, "Red Dragonplate", 1)
    ObjectUtils.spawn_item_near(self, "Dragon Lanart", 1)
    local type, amount
    do
      local _obj_0 = ItemUtils.randart_generate(1)
      type, amount = _obj_0.type, _obj_0.amount
    end
    return ObjectUtils.spawn_item_near(self, type, amount)
  end
})
DataW.enemy_create(summoner_base("Imp", 1, 100, 100)({
  name = "Hell Warrior",
  appear_message = "A Hell Warrior commands you to die!",
  defeat_message = "You have rebuked the Hell Warrior!",
  sprite = "hell warrior",
  radius = 16,
  xpaward = 150,
  unique = true,
  init_func = enemy_berserker_init,
  step_func = enemy_berserker_step,
  types = {
    "Red"
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 200,
    hpregen = 0.125,
    movespeed = 5,
    strength = 25,
    magic = 10,
    defence = 12,
    willpower = 8
  },
  on_post_draw = draw_boss_name,
  death_func = function(self)
    local item = random_choice({
      "Will Scroll",
      "Strength Scroll",
      "Defence Scroll",
      "Magic Scroll"
    })
    ObjectUtils.spawn_item_near(self, item, 1)
    ObjectUtils.spawn_item_near(self, "War Lanart", 1)
    local ItemUtils = require("maps.ItemUtils")
    local type, amount
    do
      local _obj_0 = ItemUtils.randart_generate(1)
      type, amount = _obj_0.type, _obj_0.amount
    end
    return ObjectUtils.spawn_item_near(self, type, amount)
  end
}))
DataW.enemy_create({
  name = "Gragh",
  appear_message = "Gragh greets you uproariously!",
  defeat_message = "Gragh yells goodbye!",
  sprite = "spr_enemies.bosses.gragh",
  radius = 18,
  xpaward = 250,
  unique = true,
  types = {
    "Red",
    "Bludgeon"
  },
  resistances = {
    Red = -2,
    White = 1,
    Black = 2,
    Bludgeon = 1
  },
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    hp = 140,
    hpregen = 0.02,
    movespeed = 2.9,
    strength = 28,
    magic = 10,
    defence = 8,
    willpower = 8
  },
  on_post_draw = draw_boss_name,
  death_func = function(self)
    local ItemUtils = require("maps.ItemUtils")
    for i = 1, 1 do
      local type, amount
      do
        local _obj_0 = ItemUtils.randart_generate(1)
        type, amount = _obj_0.type, _obj_0.amount
      end
      ObjectUtils.spawn_item_near(self, type, amount)
    end
    return ObjectUtils.spawn_item_near(self, "Rage Lanart", 1)
  end,
  effects_active = {
    "Enraging"
  }
})
DataW.enemy_create({
  name = "Stara",
  appear_message = "Stara cackles!",
  defeat_message = "Stara is dead!",
  sprite = "spr_enemies.bosses.stara",
  radius = 15,
  xpaward = 200,
  unique = true,
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        projectile = "Skullthrow"
      }
    },
    hp = 160,
    hpregen = 0.18,
    movespeed = 3,
    strength = 25,
    magic = 10,
    defence = 8,
    willpower = 8
  },
  on_post_draw = draw_boss_name,
  death_func = function(self)
    local ItemUtils = require("maps.ItemUtils")
    ObjectUtils.spawn_item_near(self, "Obliteration Lanart", 1)
    ObjectUtils.spawn_item_near(self, "Amulet of Greater Fire", 1)
    for i = 1, 1 do
      local type, amount
      do
        local _obj_0 = ItemUtils.randart_generate(2)
        type, amount = _obj_0.type, _obj_0.amount
      end
      ObjectUtils.spawn_item_near(self, type, amount)
    end
  end,
  effects_active = {
    {
      "Pain Aura",
      {
        damage = 20,
        range = 40
      }
    }
  }
})
DataW.enemy_create(summoner_base({
  "Mouther",
  "Mana Sapper",
  "Crypt Keeper"
}, 1, 180, 150, 20)({
  name = "Pixulloch",
  appear_message = "Pixulloch laughs at your feeble presence!",
  defeat_message = "Pixulloch has been defeated! Consider the game won. For now.",
  sprite = "spr_enemies.bosses.spider",
  radius = 20,
  xpaward = 300,
  unique = true,
  stats = {
    attacks = {
      {
        weapon = "Basic Melee"
      }
    },
    movespeed = 3.5,
    hp = 215,
    hpregen = 0.08,
    strength = 25,
    magic = 30,
    defence = 16,
    willpower = 16
  },
  types = {
    "Black"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 0.5,
      physical = 0.5
    },
    range = 300,
    cooldown = 135,
    speed = 8,
    spr_attack = "blue blast"
  }
}))
DataW.enemy_create({
  name = "Centaur Hunter",
  sprite = "spr_enemies.humanoid.centaur",
  death_sprite = "blood",
  radius = 12,
  xpaward = 40,
  appear_message = "A centaur hunter readies his bow.",
  defeat_message = "The centaur hunter has been defeated.",
  stats = {
    hp = 60,
    hpregen = 0.01,
    movespeed = 4,
    strength = 20,
    defence = 8,
    willpower = 8
  },
  types = {
    "Green"
  },
  effects_active = {
    "StopOnceInRange"
  },
  projectile = {
    weapon_class = 'bows',
    damage_type = {
      physical = 1.0
    },
    types = {
      "Piercing"
    },
    cooldown = 30,
    speed = 9,
    power = {
      base = {
        10,
        10
      }
    },
    spr_attack = 'nessos arrows'
  }
})
DataW.enemy_create({
  name = "Centaur Marksman",
  sprite = "spr_enemies.humanoid.centaur_warrior",
  death_sprite = "blood",
  radius = 12,
  xpaward = 80,
  appear_message = "A centaur marksman readies his bow.",
  defeat_message = "The centaur marksman has been defeated.",
  stats = {
    hp = 90,
    hpregen = 0.02,
    movespeed = 4,
    strength = 24,
    defence = 12,
    willpower = 12
  },
  effects_active = {
    "StopOnceInRange"
  },
  types = {
    "Green"
  },
  projectile = {
    weapon_class = 'bows',
    damage_type = {
      physical = 1.0
    },
    cooldown = 40,
    types = {
      "Piercing"
    },
    speed = 9,
    spr_attack = 'nessos arrows',
    power = {
      base = {
        10,
        10
      }
    },
    on_hit_func = function(self, target, atkstats)
      if chance(.15 * TypeEffectUtils.get_resistance(target, 'Green')) then
        return target:add_effect("Poison", {
          time_left = 80,
          poison_rate = 25,
          attacker = self.caster,
          damage = 20,
          power = self.caster:effective_stats().strength,
          magic_percentage = 0
        })
      end
    end
  }
})
DataW.enemy_create({
  name = "Nesso",
  sprite = "spr_enemies.humanoid.centaur_warrior",
  death_sprite = "blood",
  unique = true,
  radius = 12,
  xpaward = 120,
  appear_message = "Nesso readies his bow.",
  defeat_message = "Nesso has been defeated.",
  stats = {
    hp = 200,
    hpregen = 0.02,
    movespeed = 4,
    strength = 60,
    defence = 16,
    willpower = 16
  },
  effects_active = {
    "StopOnceInRange"
  },
  types = {
    "Green"
  },
  projectile = {
    weapon_class = 'bows',
    damage_type = {
      physical = 1.0
    },
    cooldown = 20,
    speed = 9,
    spr_attack = 'nessos arrows',
    types = {
      "Piercing"
    },
    on_hit_func = function(self, target, atkstats)
      if chance(.25 * TypeEffectUtils.get_resistance(target, 'Green')) then
        return target:add_effect("Poison", {
          time_left = 100,
          poison_rate = 25,
          attacker = self.caster,
          damage = 30,
          power = self.caster:effective_stats().strength,
          magic_percentage = 0
        })
      end
    end
  }
})
DataW.enemy_create({
  name = "Hell Forged",
  sprite = "hell forged",
  radius = 12,
  xpaward = 90,
  appear_message = "A hell-forged creature appears before you!",
  defeat_message = "The hell-forged is destroyed.",
  stats = {
    movespeed = 3.5,
    hp = 110,
    hpregen = 0.01,
    strength = 36,
    magic = 36,
    defence = 18,
    willpower = 18
  },
  types = {
    "Red"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 0.5,
      physical = 0.5
    },
    range = 250,
    radius = 11,
    cooldown = 50,
    speed = 8,
    spr_attack = "large fire"
  }
})
DataW.enemy_create({
  name = "Cloud Elemental",
  sprite = "cloud elemental",
  appear_message = "A cloud elemental floats onto the scene.",
  defeat_message = "The cloud elemental dissolves.",
  radius = 12,
  xpaward = 20,
  stats = {
    movespeed = 2,
    hp = 25,
    hpregen = 0.02,
    strength = 5,
    magic = 17,
    defence = 4,
    willpower = 5
  },
  types = {
    "White"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 1.0
    },
    range = 130,
    cooldown = 45,
    speed = 7,
    spr_attack = "storm bolt"
  }
})
DataW.enemy_create({
  name = "Storm Elemental",
  sprite = "storm elemental",
  radius = 14,
  xpaward = 20,
  appear_message = "A storm elemental floats onto the scene.",
  defeat_message = "The storm elemental dissolves.",
  stats = {
    movespeed = 1,
    hp = 50,
    hpregen = 0.05,
    strength = 5,
    magic = 25,
    defence = 6,
    willpower = 8
  },
  types = {
    "White"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 1.0
    },
    range = 130,
    cooldown = 45,
    speed = 7,
    spr_attack = "storm bolt"
  }
})
DataW.enemy_create({
  name = "Hell Storm",
  sprite = "hell storm",
  radius = 14,
  xpaward = 55,
  appear_message = "A raging storm elemental appears!",
  defeat_message = "The raging storm elemental dissipates.",
  stats = {
    movespeed = 2,
    hp = 75,
    hpregen = 0.05,
    strength = 5,
    magic = 45,
    defence = 8,
    willpower = 10
  },
  types = {
    "White"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 1.0
    },
    range = 130,
    cooldown = 45,
    speed = 7,
    spr_attack = "storm bolt"
  }
})
DataW.enemy_create({
  name = "Giant Rat",
  sprite = "giant rat",
  death_sprite = "blood",
  radius = 14,
  xpaward = 5,
  description = "A giant, aggressive vermin.",
  appear_message = "A hostile large rat approaches!",
  defeat_message = "Blood splatters as the large rat is destroyed.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 35
  },
  stats = {
    hp = 15,
    hpregen = 0.03,
    movespeed = 2,
    strength = 10,
    defence = 5,
    willpower = 5
  }
})
DataW.enemy_create({
  name = "Fire Bat",
  sprite = "spr_enemies.animals.fire_bat",
  death_sprite = "blood",
  radius = 12,
  xpaward = 12,
  appear_message = "A giant fiery bat surveys the scene.",
  defeat_message = "The fire bat has died.",
  stats = {
    hp = 15,
    hpregen = 0.03,
    movespeed = 3,
    strength = 12,
    magic = 12,
    defence = 5,
    willpower = 5
  },
  init_func = function(self)
    self.timeout = 0
  end,
  step_func = function(self)
    self.timeout = self.timeout - 1
    if self.timeout <= 0 then
      for i = 1, 3 do
        self:projectile_attack("Fire Bat Projectile", random_xy_near(self))
      end
      self.timeout = 30
    end
  end,
  types = {
    "Red"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 0.5,
      physical = 0.5
    },
    range = 96,
    radius = 11,
    power = {
      base = {
        10,
        10
      }
    },
    cooldown = 20,
    speed = 8,
    spr_attack = "spr_effects.fireball_small"
  }
})
DataW.enemy_create({
  name = "Giant Bat",
  sprite = "giant bat",
  death_sprite = "blood",
  radius = 12,
  xpaward = 8,
  appear_message = "A hungry giant bat surveys the scene.",
  defeat_message = "The giant bat has died.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 25
  },
  stats = {
    hp = 18,
    hpregen = 0.03,
    movespeed = 3,
    strength = 10,
    defence = 5,
    willpower = 5
  }
})
DataW.enemy_create({
  name = "Tarantella",
  sprite = "spr_enemies.animals.tarantella",
  death_sprite = "green blood",
  radius = 14,
  xpaward = 10,
  appear_message = "A demonic tarantella crawls towards you!",
  defeat_message = "The demonic tarantella has been squashed.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 25
  },
  stats = {
    hp = 65,
    hpregen = 0.04,
    movespeed = 4.5,
    strength = 25,
    defence = 12,
    willpower = 10
  }
})
DataW.enemy_create({
  name = "Giant Spider",
  sprite = "giant spider",
  death_sprite = "green blood",
  radius = 14,
  xpaward = 10,
  appear_message = "A giant spider crawls towards you!",
  defeat_message = "The giant spider has been squashed.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 25
  },
  stats = {
    hp = 15,
    hpregen = 0.04,
    movespeed = 4,
    strength = 12,
    defence = 4,
    willpower = 10
  }
})
DataW.enemy_create({
  name = "Hound",
  sprite = "hound",
  death_sprite = "blood",
  radius = 14,
  xpaward = 10,
  appear_message = "A hungry, feral hound notices you.",
  defeat_message = "The hound yelps as it is pulverized.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 40
  },
  stats = {
    hp = 25,
    hpregen = 0.03,
    movespeed = 2.5,
    strength = 12,
    defence = 5,
    willpower = 5
  }
})
DataW.enemy_create({
  name = "Hydra",
  sprite = "hydra",
  death_sprite = "dead hydra",
  radius = 17,
  xpaward = 65,
  appear_message = "A hydra appears!",
  defeat_message = "The hydra is vanquished.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 40
  },
  stats = {
    hp = 120,
    hpregen = 0.05,
    movespeed = 3.0,
    strength = 30,
    defence = 20,
    willpower = 15
  },
  effects_active = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.25
      }
    }
  }
})
DataW.enemy_create({
  name = "Giant Frog",
  sprite = "giant frog",
  death_sprite = "green blood",
  radius = 14,
  xpaward = 10,
  appear_message = "A giant frog hops onto the scene.",
  defeat_message = "The giant frog dies!",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 40
  },
  stats = {
    attacks = {
      {
        weapon = "Slow Melee"
      }
    },
    hp = 25,
    hpregen = 0.01,
    movespeed = 3,
    strength = 12,
    defence = 5,
    willpower = 5
  }
})
DataW.enemy_create({
  name = "Adder",
  sprite = "adder",
  death_sprite = "blood",
  radius = 9,
  xpaward = 10,
  appear_message = "An adder slithers onto the scene! ",
  defeat_message = "The adder is dead. ",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 20,
    types = {
      "Piercing",
      "Green"
    },
    damage = 11
  },
  stats = {
    hp = 20,
    hpregen = 0.04,
    movespeed = 4,
    strength = 3,
    defence = 0,
    willpower = 0
  }
})
DataW.enemy_create({
  name = "Chicken",
  sprite = "chicken",
  death_sprite = "blood",
  radius = 10,
  xpaward = 15,
  appear_message = "A hostile chicken appears!",
  defeat_message = "The chicken may as well be fried.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 35,
    damage = 9
  },
  stats = {
    hp = 25,
    hpregen = 0.04,
    movespeed = 3,
    strength = 3,
    defence = 7,
    willpower = 7
  }
})
DataW.enemy_create({
  name = "Horse",
  sprite = "horse",
  death_sprite = "blood",
  radius = 12,
  xpaward = 15,
  appear_message = "A horse comes at full gallop!",
  defeat_message = "The horse is beaten.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 30
  },
  stats = {
    hp = 35,
    hpregen = 0.02,
    movespeed = 5,
    strength = 25,
    defence = 15,
    willpower = 15
  }
})
DataW.enemy_create({
  name = "Super Chicken",
  sprite = "super_chicken",
  death_sprite = "green blood",
  radius = 14,
  xpaward = 20,
  appear_message = "An unnaturally glowing chicken appears!",
  defeat_message = "The glowing chicken recieves a final blow.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 20
  },
  stats = {
    hp = 40,
    hpregen = 0.08,
    movespeed = 4,
    strength = 25,
    defence = 10,
    willpower = 10
  }
})
DataW.enemy_create({
  name = "Giant Chicken",
  death_sprite = "blood",
  sprite = "spr_enemies.animals.chicken_giant",
  radius = 32,
  xpaward = 40,
  appear_message = "An unnaturally giant chicken appears!",
  defeat_message = "The giant chicken topples over.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 100
  },
  stats = {
    hp = 80,
    hpregen = 0.08,
    movespeed = 4,
    strength = 40,
    defence = 10,
    willpower = 10
  },
  init_func = function(self)
    self.timeout = 0
  end,
  step_func = function(self)
    self.timeout = self.timeout - 1
    if self.timeout <= 0 then
      self:add_effect("Dash Attack", {
        time_left = 10,
        angle = math.atan2(self.vy, self.vx)
      })
      self.timeout = 200
    end
  end,
  effects_active = {
    "KnockbackWeapon"
  }
})
DataW.enemy_create({
  name = "Clucky",
  sprite = "spr_enemies.animals.chicken_boss1",
  death_sprite = "blood",
  radius = 32,
  xpaward = 50,
  appear_message = "The grizzled chicken archer warrior Clucky greets you!",
  defeat_message = "Clucky may as well be fried.",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 20
  },
  stats = {
    hp = 100,
    hpregen = 0.1,
    movespeed = 3,
    strength = 35,
    defence = 10,
    willpower = 10
  },
  projectile = {
    weapon_class = 'bows',
    damage_type = {
      physical = 1.0
    },
    cooldown = 30,
    types = {
      "Piercing"
    },
    speed = 12,
    spr_attack = 'stone_arrow_projectile',
    power = {
      base = {
        10,
        10
      }
    },
    on_hit_func = function(self, target, atkstats)
      if chance(.25 * TypeEffectUtils.get_resistance(target, 'Green')) then
        return target:add_effect("Exhausted", 5)
      end
    end
  }
})
local spellcaster_method_base
spellcaster_method_base = function(args)
  return table.merge({
    _target_xy = function(self)
      return ((function()
        if self.target == nil then
          return random_xy_near(self)
        else
          return self.target.xy
        end
      end)())
    end,
    _projatk = function(self, proj)
      return self:projectile_attack(proj, self:_target_xy())
    end
  }, args)
end
DataW.enemy_create({
  name = "Poulter",
  sprite = "spr_enemies.animals.chicken_boss2",
  death_sprite = "blood",
  radius = 32,
  xpaward = 50,
  appear_message = "The experienced chicken mage Poulter greets you!",
  defeat_message = "Poulter is defeated!",
  types = {
    "Green"
  },
  weapon = {
    cooldown = 20
  },
  stats = {
    hp = 100,
    hpregen = 0.1,
    movespeed = 3,
    strength = 35,
    defence = 10,
    willpower = 20
  },
  methods = spellcaster_method_base({
    _tornado_storm = function(self) end,
    _energy_spear = function(self)
      self:_projatk("Energy Spear")
      self.timeout = 5
    end,
    _berserk = function(self)
      self:use_spell("Berserk", {
        self.x,
        self.y
      })
      self.timeout = 10
    end,
    _dash = function(self)
      local angle = math.atan2(self.vy, self.vx)
      self:add_effect("Dash Attack", {
        time_left = 10,
        angle = angle
      })
      self.timeout = 20
    end,
    _healing_aura = function(self)
      local angle = math.atan2(self.vy, self.vx)
      self:add_effect("Healing Aura", {
        time_left = 10,
        angle = angle
      })
      self.timeout = 20
    end,
    _mephitize = function(self) end
  }),
  step_func = function(self)
    self.timeout = self.timeout - 1
    if self.timeout <= 0 then
      local effect
      if chance(.1) then
        effect = random_choice({
          self._tornado_storm
        })
      elseif chance(.1) then
        effect = random_choice({
          self._berserk
        })
      else
        effect = random_choice({
          self._energy_spear,
          self._dash
        })
      end
      return effect(self)
    end
  end,
  init_func = function(self)
    self.timeout = 0
  end,
  step_func = function(self)
    self.timeout = self.timeout - 1
    if self.timeout <= 0 then
      self.timeout = 200
    end
  end
})
DataW.enemy_create({
  name = "Waffles",
  sprite = "spr_enemies.animals.chicken_boss3",
  death_sprite = "blood",
  radius = 32,
  xpaward = 50,
  appear_message = "The chicken jester Waffles greets you!",
  defeat_message = "Waffles is defeated!",
  types = {
    "Green",
    "White",
    "Red"
  },
  weapon = {
    cooldown = 20
  },
  stats = {
    hp = 100,
    hpregen = 0.1,
    movespeed = 3,
    strength = 35,
    defence = 10,
    willpower = 10
  },
  init_func = function(self)
    self.timeout = 0
  end,
  methods = spellcaster_method_base({
    _tornado_storm = function(self) end,
    _energy_spear = function(self)
      self:_projatk("Energy Spear")
      self.timeout = 5
    end,
    _berserk = function(self)
      self:use_spell("Berserk", {
        self.x,
        self.y
      })
      self.timeout = 10
    end,
    _dash = function(self)
      local angle = math.atan2(self.vy, self.vx)
      self:add_effect("Dash Attack", {
        time_left = 10,
        angle = angle
      })
      self.timeout = 20
    end
  }),
  step_func = function(self)
    self.timeout = self.timeout - 1
    if self.timeout <= 0 then
      local effect
      if chance(.1) then
        effect = random_choice({
          self._tornado_storm
        })
      elseif chance(.1) then
        effect = random_choice({
          self._berserk
        })
      else
        effect = random_choice({
          self._energy_spear,
          self._dash
        })
      end
      return effect(self)
    end
  end
})
DataW.enemy_create({
  name = "SummonedSkeleton",
  sprite = "spr_enemies.undead.skeleton_ugly_thing",
  radius = 11,
  xpaward = 10,
  appear_message = "A re-animated skeleton abomination appears!",
  defeat_message = "The skeleton abomination is broken unto a pile of bones.",
  types = {
    "Black"
  },
  weapon = {
    cooldown = 40
  },
  stats = {
    hp = 5,
    hpregen = 0,
    movespeed = 2,
    strength = 7,
    magic = 2,
    defence = 0,
    willpower = 2
  }
})
DataW.enemy_create({
  name = "Skeleton",
  sprite = "skeleton",
  radius = 11,
  xpaward = 15,
  appear_message = "A re-animated skeleton appears!",
  defeat_message = "The skeleton is broken unto a pile of bones.",
  types = {
    "Black"
  },
  weapon = {
    cooldown = 40,
    damage = 10
  },
  stats = {
    hp = 35,
    hpregen = 0,
    movespeed = 2,
    strength = 2,
    magic = 2,
    defence = 0,
    willpower = 2
  }
})
DataW.enemy_create({
  name = "Skeleton Fighter",
  sprite = "skeleton fighter",
  radius = 11,
  xpaward = 30,
  appear_message = "A re-animated skeleton appears!",
  defeat_message = "The skeleton is broken unto a pile of bones.",
  types = {
    "Black"
  },
  weapon = {
    cooldown = 40,
    damage = 25
  },
  stats = {
    hp = 75,
    hpregen = 0.05,
    movespeed = 2,
    strength = 5,
    magic = 5,
    defence = 5,
    willpower = 5
  }
})
DataW.enemy_create({
  name = "Zombie",
  sprite = "zombie",
  death_sprite = "green blood",
  appear_message = "A re-animated zombie appears!",
  defeat_message = "The zombie is beaten into a mess of flesh.",
  radius = 14,
  xpaward = 25,
  types = {
    "Black"
  },
  weapon = {
    cooldown = 60,
    damage = 25
  },
  stats = {
    hp = 40,
    hpregen = 0.05,
    movespeed = 2,
    strength = 10,
    magic = 10,
    defence = 8,
    willpower = 8
  }
})
DataW.enemy_create({
  name = "Krell",
  sprite = "franken",
  death_sprite = "green blood",
  description = "A creature of unholy will to live.",
  appear_message = "A horrible krell enters the scene.",
  defeat_message = "The krell dies at last.",
  radius = 14,
  xpaward = 30,
  types = {
    "Black"
  },
  weapon = {
    cooldown = 60,
    damage = 35
  },
  stats = {
    hp = 35,
    hpregen = 0.25,
    movespeed = 2,
    strength = 12,
    magic = 0,
    defence = 10,
    willpower = 10
  }
})
DataW.enemy_create({
  name = "Unseen Horror",
  sprite = "unseen horror",
  death_sprite = "purple blood",
  description = "A creature adept at withstanding powerful magical spells.",
  appear_message = "A strange one-eyed creatured approaches ...",
  defeat_message = "The horrible one-eyed creatured dies.",
  radius = 14,
  xpaward = 30,
  types = {
    "Black"
  },
  weapon = {
    cooldown = 20
  },
  stats = {
    hp = 130,
    hpregen = 0,
    movespeed = 3,
    strength = 25,
    defence = 0,
    willpower = 80
  }
})
DataW.enemy_create({
  name = "Imp",
  sprite = "spr_enemies.demons.crimson_imp",
  death_sprite = "blood",
  description = "A small demon that regenerates wounds unnaturally fast.",
  appear_message = "An imp shows himself!",
  defeat_message = "The imp curses your name as he is defeated.",
  radius = 12,
  xpaward = 30,
  types = {
    "Red"
  },
  weapon = {
    cooldown = 60,
    damage = 20
  },
  stats = {
    movespeed = 3.3,
    hp = 20,
    hpregen = 0.25,
    strength = 8,
    magic = 8,
    defence = 10,
    willpower = 10
  }
})
DataW.enemy_create({
  name = "Executioner",
  sprite = "exec",
  description = "An undead creature equipped with terrifying blades.",
  appear_message = "The terrifying executioner appears.",
  defeat_message = "The executioner is vanquished.",
  init_func = enemy_berserker_init,
  step_func = enemy_berserker_step,
  radius = 12,
  xpaward = 60,
  types = {
    "Black"
  },
  weapon = {
    cooldown = 20,
    damage = 5
  },
  stats = {
    movespeed = 4.0,
    hp = 70,
    hpregen = 0.15,
    strength = 24,
    magic = 12,
    defence = 10,
    willpower = 10
  }
})
DataW.enemy_create({
  name = "Ogre Mage",
  appear_message = "A terrifying ogre mage!",
  defeat_message = "You have slain the ogre mage!",
  sprite = "ogre mage",
  death_sprite = "blood",
  radius = 12,
  xpaward = 50,
  stats = {
    hp = 80,
    hpregen = 0.1,
    movespeed = 2.5,
    strength = 8,
    magic = 25,
    defence = 5,
    willpower = 10
  },
  types = {
    "Red"
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 0.5,
      physical = 0.5
    },
    range = 250,
    radius = 11,
    cooldown = 40,
    speed = 8,
    spr_attack = "large fire"
  }
})
DataW.enemy_create({
  name = "Orc Warrior",
  sprite = "orc knight",
  death_sprite = "blood",
  radius = 12,
  xpaward = 30,
  appear_message = "You see an orc warrior!",
  defeat_message = "The orc warrior dies.",
  init_func = enemy_berserker_init,
  step_func = enemy_berserker_step,
  types = {
    "Red"
  },
  weapon = {
    cooldown = 30
  },
  stats = {
    hp = 80,
    hpregen = 0.02,
    movespeed = 3.4,
    strength = 20,
    defence = 8,
    willpower = 6
  }
})
DataW.enemy_create({
  name = "Ciribot",
  sprite = "ciribot",
  radius = 14,
  xpaward = 40,
  appear_message = "A strange blue metallic creature appears!",
  defeat_message = "The blue metallic creature is broken into scrap.",
  types = {
    "White"
  },
  stats = {
    movespeed = 3,
    hp = 65,
    hpregen = 0.05,
    strength = 20,
    magic = 25,
    defence = 10,
    willpower = 15
  },
  projectile = {
    weapon_class = "magic",
    damage_type = {
      magic = 1.0
    },
    range = 65,
    cooldown = 35,
    power = {
      base = {
        5,
        5
      }
    },
    speed = 7,
    spr_attack = "magic bolt"
  }
})
DataW.enemy_create({
  name = "Grue",
  sprite = "grue",
  death_sprite = "blood",
  radius = 14,
  xpaward = 35,
  appear_message = "You are likely to be eaten by a grue!",
  defeat_message = "The grue has been vanquished.",
  types = {
    "Black"
  },
  stats = {
    attacks = {
      {
        weapon = "Slow Melee"
      }
    },
    hp = 80,
    hpregen = 0.02,
    movespeed = 2,
    strength = 45,
    defence = 20,
    willpower = 20
  },
  DataW.enemy_create({
    name = "Jester",
    sprite = "jester",
    death_sprite = "green blood",
    radius = 12,
    xpaward = 85,
    appear_message = "An odd and quick creature appears!",
    defeat_message = "The odd and quick creature has been destroyed.",
    types = {
      "Black"
    },
    stats = {
      attacks = {
        {
          weapon = "Basic Melee"
        }
      },
      hp = 80,
      hpregen = 0.06,
      movespeed = 5,
      strength = 25,
      defence = 20,
      willpower = 20
    }
  })
})
DataW.enemy_create({
  name = "Golem",
  sprite = "golem",
  description = "An animated creature made from rock. Unable to regenerate, but very sturdy.",
  appear_message = "A golem eyes you with a cold gaze!",
  defeat_message = "The golem breaks apart.",
  radius = 14,
  xpaward = 85,
  stats = {
    hp = 160,
    hpregen = 0,
    movespeed = 1,
    strength = 20,
    defence = 10,
    willpower = 10
  },
  types = {
    "Green"
  },
  projectile = {
    weapon_class = "physical",
    damage_type = {
      physical = 1.0
    },
    range = 300,
    cooldown = 100,
    power = {
      base = {
        40,
        40
      }
    },
    speed = 9,
    spr_attack = "big stone"
  }
})
DataW.enemy_create({
  name = "Green Slime",
  sprite = "green slime",
  death_sprite = "green blood",
  description = "Oozing ball of slime",
  appear_message = "A green ball of goop slides into view",
  defeat_message = "The green goop falls apart",
  radius = 14,
  xpaward = 10,
  types = {
    "Green"
  },
  stats = {
    attacks = {
      {
        weapon = "Fast Melee"
      }
    },
    hp = 20,
    hpregen = 0.05,
    movespeed = 1,
    strength = 15,
    defence = 0,
    willpower = 0
  }
})
DataW.enemy_create({
  name = "Red Slime",
  sprite = "red slime",
  death_sprite = "green blood",
  description = "Oozing ball of slime",
  appear_message = "A red ball of goop slides into view",
  defeat_message = "The red goop falls apart",
  radius = 14,
  xpaward = 20,
  types = {
    "Red"
  },
  stats = {
    attacks = {
      {
        weapon = "Fast Melee"
      }
    },
    hp = 25,
    hpregen = 0.1,
    movespeed = 1,
    strength = 6,
    defence = 4,
    willpower = 6
  }
})
return DataW.enemy_create({
  name = "Firelord",
  sprite = "red slime",
  death_sprite = "green blood",
  description = "Oozing ball of slime",
  appear_message = "A red ball of goop slides into view",
  defeat_message = "The red goop falls apart",
  radius = 14,
  xpaward = 20,
  types = {
    "Red"
  },
  stats = {
    attacks = {
      {
        weapon = "Fast Melee"
      }
    },
    hp = 25,
    hpregen = 0.1,
    movespeed = 1,
    strength = 6,
    defence = 4,
    willpower = 6
  }
})
