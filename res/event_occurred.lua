local events = {}

function events.PlayerDeath()
    game.score_board_store()
end

function events.MonsterDeath(monster)
    if monster.name == "Zin" then
        engine.game_won()
    end
end

function events.PlayerEnterLevel()
    local single_player = (settings.connection_type == net.NONE)
    if single_player then
        game.score_board_store()
        game.save("savefile.save")
    end
end

-- Simple forwarding function
function event_occurred(type, --[[Misc data]] ...)
    local event = events[type]
    if event then
        return event(...)
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
