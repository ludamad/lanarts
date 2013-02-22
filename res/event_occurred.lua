function event_occurred(type, --[[Misc data]]...)
    if type == 'PlayerDeath' then
        local player = ...
        print("You died!")
    end
end
