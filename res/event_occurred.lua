function event_occurred(type, --[[Misc data]] ...)
    if type == 'PlayerDeath' then
        game.score_board_store()
    elseif type == 'MonsterDeath' then
        local monster = ...
        if monster.name == "Zin" then
            engine.game_won()
        end
    end
end

function player_has_won()
    require "winning_screen"
    require "game_loop"

    game.wait(400)
    game_loop_control.game_is_over = true
    game.score_board_store(--[[Store winning entry]] true)
    winning_screen_show()
end