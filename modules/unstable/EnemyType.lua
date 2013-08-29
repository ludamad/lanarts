-- Usage: 'EnemyType.define { ... attributes ... }', 'EnemyType.lookup(<name or ID>)'

local Stats = import "@Stats"
local Item = import "@Item"
local Schemas = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local EnemyType = ResourceTypes.type_create(
    Schemas.checker_create {
        name = Schemas.STRING,
        description = Schemas.STRING,
        -- TODO sprites, radius, appear message, defeat message
        traits = Schemas.TABLE,
        on_prerequisite = Schemas.FUNCTION_OR_NIL
    }
)

-- OK, need a battle plan.
-- Integration into Lanarts, it can be all-or-nothing ...
-- This would mean serious recoding of various parts -- but for now the player controls are OK
-- Or, recode player controls now...
-- Plan right now is to replace all items, spells, and enemies
-- Problem: Game objects are inflexible
-- Have to take steps ...
--  * Recode projectiles in Lua ? Not strictly necessary ... but not worth doing any other way.
--  * Recode enemies in Lua ? Can start writing new enemy AI while enthused, eventually replace old one
--  * Recode player in Lua ? Not immediately necessary, however much kludgey glue-code may have to be written ...
--      - Player logic that intersects with stats is mainly the io control stuff

--  - name: Unseen Horror
--    sprite: unseen horror
--    death_sprite: purple blood
--    description: A creature adept at withstanding powerful magical spells.
--    appear_message: A strange one-eyed creatured approaches ...
--    defeat_message: The horrible one-eyed creatured dies.
--    radius: 14
--    xpaward: 30
--    stats:
--        attacks: [ {weapon: Fast Magic Melee} ]
--        hp: 130
--        hpregen: 0
--        movespeed: 3
--        strength: 25
--        defence: 0
--        willpower: 80 

return EnemyType