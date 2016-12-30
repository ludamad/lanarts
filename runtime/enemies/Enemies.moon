export enemy_init
export enemy_step
export enemy_berserker_init
export enemy_berserker_step

Map = require "core.Map"

enemy_init = (enemy) -> nil

enemy_step = (enemy) -> nil

enemy_berserker_init = (enemy) ->
    enemy.berserk_time = 500

enemy_berserker_step = (enemy) ->
    enemy.berserk_time = enemy.berserk_time - 1
    if enemy.berserk_time <= 0 and Map.object_visible(enemy)
        enemy\add_effect("Berserk", 300)
        enemy.berserk_time = 500

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

Data.enemy_create {
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
    init_func: () =>
        @n_steps = 0
        @summon_rate = 60
        @summoned = {}
        @n_summons = 0
    step_func: () =>
        @n_summons = 0
        for mon, time in pairs @summoned
            if time > 250
                mon\direct_damage(mon.stats.hp + 1)
            if mon.destroyed
                @summoned[mon] = nil
            else
                @summoned[mon] += 1
                @n_summons += 1
        if Map.object_visible(@) and not (@has_effect "SummoningMummy") and @n_summons < 2
            if #Map.players_list() == 0
                return
            if @n_steps > @summon_rate
                @add_effect "SummoningMummy", 20
                @n_steps = 0
            else 
                @n_steps += 1
}

-- SNAKE PIT ENEMIES

Data.enemy_create {
    name: "Black Mamba"
    sprite: "spr_enemies.animals.black_mamba"
    death_sprite: "blood"
    radius: 10
    xpaward: 20
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
    name: "Mouther" 
    sprite: "spr_enemies.animals.tyrant_leech"
    radius: 11
    xpaward: 25
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


