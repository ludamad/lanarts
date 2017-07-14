ENEMY_SMELL = 10000.0
PLAYER_SMELL = -100000.0
WALL_SMELL = 1000

GameState = require "core.GameState"

-- Tightly coupled with src/objects/MonsterController.cpp
MonsterController = newtype {
    init: (map) =>
        @map = map
    -- Tightly coupled with MonsterController::pre_step
    step: (base, monsters, players) =>
        -- Set initial monster headings
        --if GameState.frame % 3 == 0
        base.smell_map_step()
        -- Se the monster headings:
        for monster in *monsters
            t = monster.type
            if t.step_func ~= nil
                t.step_func(monster)
            -- Set monster heading
            raw_heading = base.towards_least_smell(monster)
            heading = base.ensure_legal_direction(monster, raw_heading)
            if vector_equals raw_heading, heading
                monster.fixed_heading = false
            elseif monster.fixed_heading and monster.fixed_heading[2] > GameState.frame
                heading = monster.fixed_heading[1]
            elseif monster\effective_stats().speed > 0
                n_frames = math.min(32, 32 / monster\effective_stats().speed)
                monster.fixed_heading = {heading, GameState.frame + n_frames}
            {monster.vx, monster.vy} = heading
        -- Use wall adjustments after collision avoidance
        for monster in *monsters
            base.adjust_heading_if_near_wall(monster)
        -- Use collision avoidance after setting monster headings
        base.rvo_adjust_headings()
        -- Use adjusted headings 
        for monster in *monsters
            base.attempt_move_object(monster, {monster.x + monster.vx, monster.y + monster.vy})
        -- Set the monster smells
        for monster in *monsters
            base.add_object_smell(monster, (if monster.team == 1 then ENEMY_SMELL else PLAYER_SMELL))
        -- Set the player smells
        for player in *players
            if player.map == @map
                base.add_object_smell(player, PLAYER_SMELL)
        -- Try attacking players
        for monster in *monsters
            base.try_attack(monster)
}

return MonsterController
