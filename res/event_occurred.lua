function event_occurred(type, --[[Misc data]] ...)
    if type == 'PlayerDeath' then
        game.score_board_store()
    end
end
