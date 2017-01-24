export enemy_init
export enemy_step
export enemy_berserker_init
export enemy_berserker_step

Map = require "core.Map"
ObjectUtils = require "objects.ObjectUtils"
EventLog = require "ui.EventLog"

enemy_init = (enemy) -> nil

enemy_step = (enemy) -> nil

enemy_berserker_init = (enemy) ->
    enemy.berserk_time = 500

enemy_berserker_step = (enemy) ->
    enemy.berserk_time = enemy.berserk_time - 1
    if enemy.berserk_time <= 0 and Map.object_visible(enemy)
        enemy\add_effect("Berserk", 300)
        enemy.berserk_time = 500

Data.effect_create {
    name: "Enraging"
    init_func: (mon) =>
        @damage_tracker = 0
        @damage_interval = mon.stats.max_hp / 3
        @next_hp_threshold = mon.stats.max_hp - @damage_interval
    on_damage_func: (mon, dmg) =>
        @damage_tracker += dmg
        new_hp = mon.stats.hp - dmg
        if new_hp < @next_hp_threshold
            EventLog.add((if mon.unique then "" else "The ") .. mon.name .. " gets mad!", {255,255,255})
            mon\add_effect("Charging", 100)
            @next_hp_threshold -= @damage_interval
        return dmg
}


-- UNDEAD ENEMIES --

Data.enemy_create {
    name: "Crypt Keeper" 
    sprite: "spr_enemies.undead.lich"
    radius: 11
    xpaward: 75
    appear_message: "A frightful Crypt Keeper emerges!"
    defeat_message: "The Crypt Keeper has been destroyed."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 50
        hpregen: 0.03
        movespeed: 2
        -- base stats:
        strength: 15
        defence: 8
        magic: 5
        willpower: 8
    }
    effects_active: {"Pain Aura"}
}

Data.enemy_create {
    name: "Mana Sapper" 
    sprite: "spr_enemies.demons.manasapper"
    radius: 11
    xpaward: 50
    appear_message: "A mana sapper comes onto the scene!"
    defeat_message: "The mana sapper dies."
    stats: {
        attacks: {
            {projectile: "Minor Missile"}
        }
        hp: 100
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        magic: 25
        defence: 5
        magic: 30
        willpower: 8
    }
    effects_active: {"Sap Aura"}
}


Data.enemy_create {
    name: "Gaseous Ghost" 
    sprite: "spr_enemies.undead.hungry_ghost"
    radius: 11
    xpaward: 75
    appear_message: "A gaseous ghost appears!"
    defeat_message: "The vile ghost is gone."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
            {projectile: "Mephitize"}
        }
        hp: 50
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 15
        defence: 5
        magic: 30
        willpower: 8
    }
    effects_active: {"PoisonedWeapon"}
}

Data.enemy_create {
    name: "Spectral Beast" 
    sprite: "spr_enemies.undead.spectral_lizard"
    radius: 11
    xpaward: 10
    appear_message: "A spectral beast is summoned!"
    defeat_message: "The spectre fades."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 60
        hpregen: 0
        movespeed: 4
        -- base stats:
        strength: 5
        defence: 8
        magic: 10
        willpower: 8
    }
}

Data.enemy_create {
    name: "Mummy"
    sprite: "spr_enemies.undead.mummy"
    radius: 11
    xpaward: 10
    appear_message: "A summoned mummy appears!"
    defeat_message: "The summoned mummy is put to rest."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 10
        hpregen: 0
        movespeed: 5
        -- base stats:
        strength: 15
        defence: 0
        magic: 15
        willpower: 0
   }
   effects_active: {"PoisonedWeapon"}
}

summoner_base = (monster, amount, rate = 60, kill_time = 250, duration = 150) -> (data) -> table.merge data, {
    init_func: () =>
        @n_steps = 0
        @summon_rate = rate
        @summoned = {}
        @n_summons = 0
    step_func: () =>
        @n_summons = 0
        for mon, time in pairs @summoned
            if time > kill_time
                mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                @summoned[mon] = nil
            else
                @summoned[mon] += 1
                @n_summons += 1
        if Map.object_visible(@) and not (@has_effect "Summoning") and @n_summons < amount
            if #Map.allies_list(@) == 0
                return
            if @n_steps > @summon_rate
                eff = @add_effect("Summoning", 20)
                eff.monster = (if type(monster) == "string" then monster else random_choice(monster))
                eff.duration = duration
                @n_steps = 0
            else 
                @n_steps += 1
}

Data.enemy_create summoner_base("Mummy", 5) {
    name: "Mummoner" 
    sprite: "spr_enemies.undead.greater_mummy"
    radius: 11
    xpaward: 75
    appear_message: "You find a frightful Mummoner!"
    defeat_message: "The mummoner has been destroyed."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
        }
        hp: 75
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 15
        defence: 8
        magic: 10
        willpower: 8
    }
}

-- SNAKE PIT ENEMIES

Data.enemy_create {
    name: "Queen Bee"
    sprite: "spr_enemies.animals.queen_bee"
    death_sprite: "green blood"
    radius: 10
    xpaward: 100
    appear_message: "A queen bee appears!"
    defeat_message: "The queen bee has buzzed its last buzz."
    stats: {
        attacks: {{weapon: "Fast Melee"}}
        hp: 80
        hpregen: 0.1
        movespeed: 3.5
        strength: 15
        defence: 0
        willpower: 12
    }
    effects_active: {"PoisonedWeapon", "Enraging"}
}

Data.enemy_create {
    name: "Giant Bee"
    sprite: "spr_enemies.animals.killer_bee"
    death_sprite: "green blood"
    radius: 10
    xpaward: 25
    appear_message: "A giant killer bee appears!"
    defeat_message: "The giant bee has buzzed its last buzz."
    stats: {
        attacks: {{weapon: "Fast Melee"}}
        hp: 40
        hpregen: 0.1
        movespeed: 3.5
        strength: 15
        defence: 0
        willpower: 12
    }
    effects_active: {"PoisonedWeapon", "Spiky"}
}
 
Data.enemy_create summoner_base("Giant Bee", 7, 30, 400, 5) {
    name: "Ramitawil"
    sprite: "spr_enemies.bosses.boss_bee"
    radius: 20
    xpaward: 100
    appear_message: "The demonic bee Ramitawil!"
    defeat_message: "Ramitawil has been vanquished."
    stats: {
        attacks: {{weapon: "Fast Melee"}}
        hp: 200
        hpregen: 0.2
        movespeed: 3.5
        strength: 25
        defence: 15
        willpower: 20
    }
    effects_active: {"PoisonedWeapon", "Spiky", "Enraging"}
    death_func: () =>
        ItemUtils = require "maps.ItemUtils"
        ItemGroups = require "maps.ItemGroups"
        -- Spawn 3 level 1 randarts:
        ObjectUtils.spawn_item_near(@, "Swarm Lanart", 1)
        for i=1,3
            {:type, :amount} = ItemUtil.randart_generate(1)
            ObjectUtils.spawn_item_near(@, type, amount)
        ObjectUtils.spawn_item_near(@, "Amulet of Great Pain", 1)
}
 
Data.enemy_create {
    name: "Mouther" 
    sprite: "spr_enemies.animals.tyrant_leech"
    radius: 11
    xpaward: 15
    appear_message: "You hear a mouther start screeching!"
    defeat_message: "The mouther has been shut."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
            {projectile: "Mephitize"}
        }
        hp: 40
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 5
        defence: 0
        magic: 2
        willpower: 0
    }
    effects_active: {"PoisonedWeapon"}
}

Data.enemy_create {
    name: "Black Mamba"
    sprite: "spr_enemies.animals.black_mamba"
    death_sprite: "blood"
    radius: 10
    xpaward: 10
    appear_message: "A poisonous black mamba slithers onto the scene!"
    defeat_message: "The black mamba is dead."
    stats: {
        attacks: {{weapon: "Fast Melee"}}
        hp: 40
        hpregen: 0.04
        movespeed: 3.5
        strength: 15
        defence: 0
        willpower: 5
    }
    effects_active: {"PoisonedWeapon"}
}

Data.enemy_create {
    name: "Clown Mage" 
    sprite: "spr_enemies.humanoid.killer_klown_purple"
    radius: 11
    xpaward: 30
    appear_message: "A clown mage appears!"
    defeat_message: "The clown mage is defeated."
    stats: {
        attacks: {
            {projectile: "Magic Blast"}
        }
        hp: 65
        hpregen: 0.03
        movespeed: 3
        -- base stats:
        strength: 0
        defence: 0
        magic: 25
        willpower: 0
    }
    effects_active: {"StopOnceInRange", "Enraging"}
}

Data.enemy_create {
    name: "Mouther" 
    sprite: "spr_enemies.animals.tyrant_leech"
    radius: 11
    xpaward: 15
    appear_message: "You hear a mouther start screeching!"
    defeat_message: "The mouther has been shut."
    stats: {
        attacks: {
            {weapon: "Basic Melee"}
            {projectile: "Mephitize"}
        }
        hp: 40
        hpregen: 0.03
        movespeed: 1
        -- base stats:
        strength: 5
        defence: 0
        magic: 2
        willpower: 0
    }
    effects_active: {"PoisonedWeapon"}
}


-- enemy_charge = (caster) ->
--     for target in *Map.enemies_list(caster)
--         if vector_distance({target.x, target.y}, {caster.x, caster.y}) < target.target_radius + caster.target_radius + 30
--             str_diff = math.max(0, caster.stats.strength - target.stats.strength)
--             thrown = target\add_effect("Thrown", 10)
--             thrown.angle = vector_direction({caster.x, caster.y}, {target.x, target.y})
--             if not caster.is_enemy and caster\is_local_player() 
--                 EventLog.add("The " .. target.name .." is thrown back!", {200,200,255})
--             elseif not target.is_enemy and target\is_local_player()
--                 EventLog.add("You are thrown back!", {200,200,255})

Data.effect_create {
    name: "Charging"
    stat_func: (mon, old, new) =>
        new.speed *= 4
        if @n_steps > @n_ramp
            new.melee_cooldown_multiplier *= 0.5
        new.hpregen *= 2
    effected_sprite: "spr_effects.i-loudness"
    effected_colour: {255,0,0}
    fade_out: 15
    init_func: (mon) =>
        @n_steps = 0
        @n_ramp = 10
    step_func: () =>
        if @n_steps % 60 == 0
            play_sound "sound/wavy.ogg"
        @n_steps += 1
    on_melee_func: (mon, defender, damage, attack_stats) =>
        if @n_steps > @n_ramp
            thrown = defender\add_effect("Thrown", 10)
            thrown.angle = vector_direction({mon.x, mon.y}, {defender.x, defender.y})
            if not mon.is_enemy and mon\is_local_player() 
                EventLog.add("The " .. defender.name .." is thrown back!", {200,200,255})
            elseif not defender.is_enemy and defender\is_local_player()
                EventLog.add("You are thrown back!", {200,200,255})
            -- mon\remove_effect("Charging")
}
Data.enemy_create {
    name: "Elephant"
    sprite: "spr_enemies.animals.elephant"
    death_sprite: "blood"
    radius: 15
    xpaward: 50
    appear_message: "A loud elephant!"
    defeat_message: "The elephant is dead."
    stats: {
        attacks: {{weapon: "Slow Melee"}}
        hp: 70
        hpregen: 0.05
        movespeed: 2
        strength: 20
        defence: 8
        willpower: 8
    }
    effects_active: {"Enraging"}
}
 
Data.enemy_create {
    name: "Sheep"
    sprite: "spr_enemies.animals.sheep"
    death_sprite: "blood"
    radius: 14
    xpaward: 10
    appear_message: "A hostile sheep approaches!"
    defeat_message: "The sheep recieves a fatal blow."
    stats: {
        attacks: {{weapon: "Basic Melee"}}
        hp: 20
        hpregen: 0.05
        movespeed: 2
        strength: 20
        defence: 8
        willpower: 8
    }
    effects_active: {"Enraging"}
}
 
Data.enemy_create {
    name: "Red Dragon"
    appear_message: "A frighteningly large red dragon comes into view!"
    defeat_message: "You have slain the red dragon!"
    sprite: "red dragon"
    radius: 27
    xpaward: 150
    unique: true
    stats: {
        attacks: { {weapon: "Basic Melee"}, { projectile: "Large Fire Ball"} }
        hp: 220
        hpregen: 0.1
        movespeed: 2.5
        strength: 35
        magic: 35
        defence: 15
        willpower: 15
    }
    death_func: () =>
        ItemUtils = require "maps.ItemUtils"
        ItemGroups = require "maps.ItemGroups"
        ObjectUtils.spawn_item_near(@, "Red Dragonplate", 1)
        ObjectUtils.spawn_item_near(@, "Dragon Lanart", 1)
        -- Spawn a level 1 randart:
        {:type, :amount} = ItemUtils.randart_generate(1)
        ObjectUtils.spawn_item_near(@, type, amount)
}

Data.enemy_create summoner_base("Imp", 1, 100, 100) {
    name: "Hell Warrior"
    appear_message: "A Hell Warrior commands you to die!"
    defeat_message: "You have rebuked the Hell Warrior!"
    sprite: "hell warrior"
    radius: 16
    xpaward: 150
    unique: true
    init_func: enemy_berserker_init 
    step_func: enemy_berserker_step
    stats: {
        attacks: {{weapon: "Basic Melee"}}
        hp: 200
        hpregen: 0.125
        movespeed: 5
        strength: 25
        magic: 10
        defence: 12
        willpower: 8
    }
    death_func: () =>
        item = random_choice {"Will Scroll", "Strength Scroll", "Defence Scroll", "Magic Scroll"}
        ObjectUtils.spawn_item_near(@, item, 1)
        ObjectUtils.spawn_item_near(@, "Abyssal Lanart", 1)
        ItemUtils = require "maps.ItemUtils"
        ItemGroups = require "maps.ItemGroups"
        -- Spawn a level 1 randart:
        {:type, :amount} = ItemUtil.randart_generate(1)
        ObjectUtils.spawn_item_near(@, type, amount)
}

Data.enemy_create {
    name: "Gragh"
    appear_message: "Gragh greets you uproariously!"
    defeat_message: "Gragh yells goodbye!"
    sprite: "spr_enemies.bosses.gragh"
    radius: 18
    xpaward: 200
    unique: true
    stats: {
        attacks: {{weapon: "Basic Melee"}}
        hp: 200
        hpregen: 0.25
        movespeed: 3
        strength: 25
        magic: 10
        defence: 8
        willpower: 8
    }
    death_func: () =>
        ItemUtils = require "maps.ItemUtils"
        ItemGroups = require "maps.ItemGroups"
        -- Spawn 2 level 1 randarts:
        for i=1,2
            {:type, :amount} = ItemUtil.randart_generate(1)
            ObjectUtils.spawn_item_near(@, type, amount)
        ObjectUtils.spawn_item_near(@, "Gragh's Club", 1)
        ObjectUtils.spawn_item_near(@, "Rage Lanart", 1)
        ObjectUtils.spawn_item_near(@, "Amulet of the Berserker", 1)
    effects_active: {"Enraging"}
}

Data.enemy_create {
    name: "Stara"
    appear_message: "Stara cackles!"
    defeat_message: "Stara is dead!"
    sprite: "spr_enemies.bosses.stara"
    radius: 15
    xpaward: 200
    unique: true
    stats: {
        attacks: {{projectile: "Skullthrow"}}
        hp: 200
        hpregen: 0.25
        movespeed: 3
        strength: 25
        magic: 10
        defence: 8
        willpower: 8
    }
    step_func: () =>
        if @has_effect "Pain Aura"
            @get_effect("Pain Aura").range = 40
    death_func: () =>
        ItemUtils = require "maps.ItemUtils"
        ItemGroups = require "maps.ItemGroups"
        ObjectUtils.spawn_item_near(@, "Obliteration Lanart", 1)
        ObjectUtils.spawn_item_near(@, "Amulet of Greater Fire", 1)
        -- Spawn 1 level 2 randart:
        for i=1,2
            {:type, :amount} = ItemUtil.randart_generate(1)
            ObjectUtils.spawn_item_near(@, type, amount)
    effects_active: {"Pain Aura"}
}

Data.enemy_create summoner_base({"Mouther", "Mana Sapper", "Crypt Keeper"}, 3, 100, 200, 25) {
    name: "Pixulloch"
    appear_message: "Pixulloch laughs at your feeble presence!"
    defeat_message: "Pixulloch has been defeated! Consider the game won. For now."
    sprite: "spr_enemies.bosses.spider"
    radius: 20
    xpaward: 300
    unique: true
    stats: {
        attacks: {{ weapon: "Basic Melee"}, { projectile: "Longer Range Blue Blast"}}
        movespeed: 3.5
        hp: 400
        hpregen: 0.15
        strength: 25
        magic: 30
        defence: 20
        willpower: 20
    }
}

Data.enemy_create {
    name: "Centaur Hunter"
    sprite: "spr_enemies.humanoid.centaur"
    death_sprite: "blood"
    radius: 12
    xpaward: 30
    appear_message: "A centaur hunter readies his bow."
    defeat_message: "The centaur hunter has been defeated."
    stats: {
        attacks: {{projectile: "Dark Arrows"}}
        hp: 80
        hpregen: 0.01
        movespeed: 4
        strength: 20
        defence: 8
        willpower: 8
    }
    effects_active: {"StopOnceInRange"}
}

Data.enemy_create {
    name: "Centaur Marksman"
    sprite: "spr_enemies.humanoid.centaur_warrior"
    death_sprite: "blood"
    radius: 12
    xpaward: 60
    appear_message: "A centaur marksman readies his bow."
    defeat_message: "The centaur marksman has been defeated."
    stats: {
        attacks: {{projectile: "Poison Arrows"}}
        hp: 120
        hpregen: 0.02
        movespeed: 4
        strength: 40
        defence: 16
        willpower: 16
    }
    effects_active: {"StopOnceInRange"}
}

Data.enemy_create {
    name: "Nesso"
    sprite: "spr_enemies.humanoid.centaur_warrior"
    death_sprite: "blood"
    unique: true
    radius: 12
    xpaward: 120
    appear_message: "Nesso readies his bow."
    defeat_message: "Nesso has been defeated."
    stats: {
        attacks: {{projectile: "Rapid Poison Arrows"}}
        hp: 200
        hpregen: 0.02
        movespeed: 4
        strength: 40
        defence: 16
        willpower: 16
    }
    effects_active: {"StopOnceInRange"}
}

