--[[
    This is the only Lua file explicitly loaded by the lanarts engine.

    This sets up all the engine hooks that control program flow, and pull
    in additional files.
]]

require_path_add("modules/lanarts/library_files/?.lua")

sound_volume = 0 -- Mute the game

function Engine.menu_start(...)
    import "lanarts.menus.start_menu"

    return start_menu_show(...)
end

function Engine.pregame_menu_start(...)
    import "lanarts.menus.pregame_menu"

    return pregame_menu_show(...)
end

function Engine.loading_screen_draw(...)
    import "lanarts.menus.loading_screen"

    return loading_screen_draw(...)
end

function Engine.resources_load(...)
    --TODO: Find a better place for these helper functions
    function is_consumable(item)     return item.type == "consumable" end
    function is_weapon(item)         return item.type == "weapon" end
    function is_armour(item)         return item.type == "armour" end
    function is_projectile(item)     return item.type == "projectile" end

    import "lanarts.sounds"

    dofile "modules/lanarts/enemies/enemies.lua"

    dofile "modules/lanarts/effects/effects.lua"

    dofile "modules/lanarts/spells/spell_effects.lua"
end

function Engine.resources_post_load(...)
    import "lanarts.tiles.tilesets"
end

function Engine.game_start(...)
    import "lanarts.game_loop"

    return game_loop(...)
end

function Engine.post_draw(...)
    import "lanarts.game_loop"

    return game_post_draw(...)
end

function Engine.overlay_draw(...)
    import "lanarts.game_loop"

    return game_overlay_draw(...)
end

function Engine.game_won(...)
    import "lanarts.event_occurred"

    return player_has_won(...)
end

function Engine.event_occurred(...)
    import "lanarts.event_occurred"

    return event_occurred(...)
end

function Engine.first_map_create(...)
	import "core.maps.map_generation"

	return first_map_create(...)
end
