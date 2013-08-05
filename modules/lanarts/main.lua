--[[
    This is the only Lua file explicitly loaded by the lanarts engine.

    This sets up all the engine hooks that control program flow, and pull
    in additional files.
]]

require_path_add("modules/lanarts/library_files/?.lua")

sound_volume = 0 -- Mute the game

function Engine.menu_start(...)
    local menus = import "@menus"

    return menus.start_menu_show(...)
end

function Engine.pregame_menu_start(...)
    local menus = import "@menus"

    return menus.pregame_menu_show(...)
end

function Engine.loading_screen_draw(...)
    local loading_screen = import "@menus.loading_screen"

    return loading_screen.draw(...)
end

function Engine.resources_load(...)
    --TODO: Find a better place for these helper functions
    function is_consumable(item)     return item.type == "consumable" end
    function is_weapon(item)         return item.type == "weapon" end
    function is_armour(item)         return item.type == "armour" end
    function is_projectile(item)     return item.type == "projectile" end

    import "@enemies.enemies"

    import "@effects.effects"

    import "@spells.spell_effects"
end

function Engine.resources_post_load(...)
    import "@tiles.tilesets"
end

function Engine.game_start(...)
    local game_loop = import "@game_loop"

    return game_loop.run_loop(...)
end

function Engine.post_draw(...)
    local game_loop = import "@game_loop"

    return game_loop.post_draw(...)
end

function Engine.overlay_draw(...)
    local game_loop = import "@game_loop"

    return game_loop.overlay_draw(...)
end

function Engine.game_won(...)
    local events = import "@events"

    return events.player_has_won(...)
end

function Engine.event_occurred(...)
    local events = import "@events"

    return events.trigger_event(...)
end

function Engine.first_map_create(...)
    local region1 = import "@maps.region1"

    local map_id = region1.overworld_create(...)
	return map_id
end
