--- This is the main loading point of the game, after the bootstrap 'main'.
-- This sets up all the engine hooks that define the Lanarts game.

sound_volume = 0 -- Mute the game

print "function Engine.menu_start(...)"
function Engine.menu_start(...)
    local menus = require "Menus"

    return menus.start_menu_show(...)
end

function Engine.pregame_menu_start(...)
    local menus = require "Menus"

    return menus.pregame_menu_show(...)
end

function Engine.loading_screen_draw(...)
    local LoadingScreen = require "menus.LoadingScreen"

    return LoadingScreen.draw(...)
end

function Engine.resources_load(...)
    --TODO: Find a better place for these helper functions
    function is_consumable(item)     return item.type == "consumable" end
    function is_weapon(item)         return item.type == "weapon" end
    function is_armour(item)         return item.type == "armour" end
    function is_projectile(item)     return item.type == "projectile" end

    require "effects.Effects"

    require "spells.SpellEffects"
end

function Engine.resources_post_load(...)
    require "tiles.Tilesets"
end

function Engine.game_start(...)
    local game_loop = require "GameLoop"
    require("maps.01_Overworld").test_determinism()

    return game_loop.run_loop(...)
end

function Engine.post_draw(...)
    local game_loop = require "GameLoop"

    return game_loop.post_draw(...)
end

function Engine.overlay_draw(...)
    local game_loop = require "GameLoop"

    return game_loop.overlay_draw(...)
end

function Engine.game_won(...)
    local events = require "Events"

    return events.player_has_won(...)
end

function Engine.event_occurred(...)
    local events = require "Events"

    return events.trigger_event(...)
end

function Engine.first_map_create(...)
    local region1 = require "maps.01_Overworld"

    local map = region1.overworld_create(...)
    return map
end

function Engine.pre_serialize()
    local SerializationUtils = require "SerializationUtils"
    local timer = timer_create()
    SerializationUtils.name_global_data()
    SerializationUtils.install_require_fallback()
    print("Naming globals took " .. timer:get_milliseconds() .. "ms.")
end

function Engine.post_serialize()

end

-- Same steps:
Engine.pre_deserialize = Engine.pre_serialize

function Engine.post_deserialize()

end
