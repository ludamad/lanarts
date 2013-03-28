--[[
    This is the only Lua file explicitly loaded by the lanarts engine.

    This sets up all the engine hooks that control program flow, and pull
    in additional files.
]]

function Engine.menu_start(...)
    require "start_menu"

    return start_menu_show(...)
end

function Engine.lobby_menu_start(...)
    require "lobby_menu"

    return lobby_menu_show(...)
end

function Engine.loading_screen_draw(...)
    require "loading_screen"

    return loading_screen_draw(...)
end

function Engine.resources_load(...)
    --TODO: Find a better place for these helper functions
    function is_consumable(item)     return item.type == "consumable" end
    function is_weapon(item)         return item.type == "weapon" end
    function is_armour(item)         return item.type == "armour" end
    function is_projectile(item)     return item.type == "projectile" end

    require "sound"

    dofile "res/enemies/enemies.lua"

    dofile "res/items/weapons/weapons.lua" 

    dofile "res/effects/effects.lua"

    dofile "res/spells/spell_effects.lua"
end

function Engine.game_start(...)
    require "game_loop"

    return game_loop(...)
end

function Engine.post_draw(...)
    require "game_loop"

    return game_post_draw(...)
end

function Engine.overlay_draw(...)
    require "game_loop"

    return game_overlay_draw(...)
end

function Engine.game_won(...)
    require "event_occurred"

    return player_has_won(...)
end

function Engine.event_occurred(...)
    require "event_occurred"

    return event_occurred(...)
end

sound_volume = 0
