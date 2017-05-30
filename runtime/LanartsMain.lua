--- This is the main loading point of the game, after the bootstrap 'main'.
-- This sets up all the engine hooks that define the Lanarts game.

-- For tests:
local interceptable_require = require
if require("tests.main").testcase then
    local test_module = require("tests.main").testcase
    interceptable_require = function(...)
        print("intercepting ", ...)
        return test_module:intercept(...)
    end
end
-- For headless runs:
local drawsystem_require = require
if os.getenv("LANARTS_HEADLESS") then
    drawsystem_require = function(...)
        return {
            draw = do_nothing, post_draw = do_nothing, overlay_draw = do_nothing
        }
    end
end

function Engine.menu_start(...)
    log "function Engine.menu_start(...)"
    local menus = require "Menus"
    return menus.start_menu_show(...)
end

function Engine.pregame_menu_start(...)
    log "function Engine.pregame_menu_show(...)"
    local menus = require "Menus"
    return menus.pregame_menu_show(...)
end

function Engine.loading_screen_draw(...)
    log "function Engine.loading_screen_draw(...)"
    local LoadingScreen = drawsystem_require "menus.LoadingScreen"
    return LoadingScreen.draw(...)
end

function Engine.resources_load(...)
    log "function Engine.resources_load(...)"
    --TODO: Find a better place for these helper functions

    require "effects.Effects"
end

function Engine.resources_post_load(...)
    require "tiles.Tilesets"
end

function Engine.game_start(...)
    -- Dont intercept the actual game loop, that's being tested:
    local game_loop = require "GameLoop"
    -- require("maps.01_Overworld").test_determinism()

    return game_loop.run_loop(...)
end

function Engine.post_draw(...)
    local game_loop = drawsystem_require "GameLoop"

    return game_loop.post_draw(...)
end

function Engine.overlay_draw(...)
    local game_loop = drawsystem_require "GameLoop"

    return game_loop.overlay_draw(...)
end

function Engine.game_won(...)
    local events = interceptable_require "Events"

    return events.player_has_won(...)
end

function Engine.event_occurred(...)
    local events = interceptable_require "Events"

    return events.trigger_event(...)
end

function Engine.first_map_create(...)
    local region1 = interceptable_require "maps.01_Overworld"

    local map = region1.overworld_create(...)
    return map
end

function Engine.pre_serialize()
    local SerializationUtils = require "SerializationUtils"
    local timer = timer_create()
    SerializationUtils.name_global_data()
    SerializationUtils.install_require_fallback()
    log_verbose("Naming globals took " .. timer:get_milliseconds() .. "ms.")
end

function Engine.post_serialize()

end

-- Same steps:
Engine.pre_deserialize = Engine.pre_serialize

function Engine.post_deserialize()

end
