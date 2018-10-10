-- Implementing the 'Engine' hooks that define the high-level functions that
-- make up Lanarts.
--
-- For tests:

Engine.require = require

function Engine.menu_start(...)
    log "function Engine.menu_start(...)"
    local menus = Engine.require "Menus"
    return menus.start_menu_show(...)
end

function Engine.pregame_menu_start(...)
    log "function Engine.pregame_menu_show(...)"
    local menus = Engine.require "Menus"
    return menus.pregame_menu_show(...)
end

function Engine.loading_screen_draw(...)
    log "function Engine.loading_screen_draw(...)"
    local LoadingScreen = Engine.require "menus.LoadingScreen"
    return LoadingScreen.draw(...)
end

function Engine.is_explorable_solid_object(obj)
    -- Hook from C++
    local func = getmetatable(obj).is_explorable
    if func then
        return func(obj)
    else
        return false
    end
end

function Engine.resources_load(...)
    log "function Engine.resources_load(...)"

    local function _req(module)
        local GameState = Engine.require("core.GameState")
        --log_verbose
        print("Loading resources from '" .. module .. "'")
        local ret = yield_point(Engine.require, module)
        return ret
    end

    -- DO NOT mess with the order of these willy nilly.
    _req "spells.Spells"

    _req "items.Items"
    _req "items.Weapons"
    _req "items.BodyArmour"
    _req "items.Boots"
    _req "items.Gloves"
    _req "items.Helmets"
    _req "items.Belts"
    _req "items.Legwear"
    _req "items.Amulets"
    _req "items.Rings"

    -- Start the game with 1000 'randarts' -- for now, preconfigured item generations.
    yield_point(_req("items.Randarts").define_randarts)

    _req "enemies.Enemies"

    _req "tiles.Tilesets"

    _req "classes.Classes"
end

function Engine.game_start(...)
    -- Dont intercept the actual game loop, that's being tested:
    local game_loop = Engine.require "GameLoop"
    -- require("maps.01_Overworld").test_determinism()

    return game_loop.run_loop(...)
end

function Engine.post_draw(...)
    local game_loop = Engine.require "GameLoop"

    return game_loop.post_draw(...)
end

function Engine.overlay_draw(...)
    local game_loop = Engine.require "GameLoop"

    return game_loop.overlay_draw(...)
end

function Engine.post_pickup(player, slot_idx)
    local InventoryManagement = Engine.require "engine.InventoryManagement"
    InventoryManagement.post_pickup(player, slot_idx)
end

function Engine.game_won(...)
    local events = Engine.require "Events"

    return events.player_has_won(...)
end

function Engine.event_occurred(...)
    local events = Engine.require "Events"

    return events.trigger_event(...)
end

function Engine.first_map_create(...)
    if os.getenv("LINEAR_MODE") then
        local linear_mode = Engine.require "maps.LinearMode"
        return linear_mode.first_map_create()
    end
    local region1 = Engine.require "maps.01_Overworld"
    local map = region1.overworld_create(...)
    return map
end

function Engine.pre_serialize()
    local SerializationUtils = Engine.require "SerializationUtils"
    local timer = timer_create()
    SerializationUtils.name_global_data()
    SerializationUtils.install_require_fallback()
    log_verbose("Naming globals took " .. timer:get_milliseconds() .. "ms.")

end

function Engine.post_serialize()
end

function Engine.io()
    --local Gamepad = require "input.Gamepad"
    --Gamepad.step_for_all()
end

local GAME_PADS = {}

function Engine.player_input(player)
    if os.getenv("LANARTS_TESTCASE") then
        return (require "input.KeyboardInputSource").create(player)
    end
    local Gamepad = require "core.Gamepad"
    local ids = Gamepad.ids()
    local offset = 1
    if os.getenv("LANARTS_CONTROLLER") then
        offset = 0
    end
    for i=2,10 do
        if player.name == "Player " .. tostring(i) then
            GAME_PADS[i] = GAME_PADS[i] or (require "input.GamepadInputSource").create(player, ids[i-offset])
            return GAME_PADS[i]
        end
    end
    local key_input = (require "input.KeyboardInputSource").create(player)
    if os.getenv("LANARTS_CONTROLLER") then
        GAME_PADS[1] = GAME_PADS[1] or (require "input.GamepadInputSource").create(player, ids[1])
        return (require "input.CombinedInputSource").create(GAME_PADS[1], key_input)
    else
        return key_input
    end
end

-- Same steps:
Engine.pre_deserialize = Engine.pre_serialize

function Engine.post_deserialize()
    local GlobalData = require "core.GlobalData"
    for _, name in ipairs(GlobalData["__dynamic_item_generations"]) do
        ensure_item(name)
    end
end

function Engine.draw_sidebar_color(obj, xy)
    local Display = require "core.Display"
    local BonusesUtils = require "items.BonusesUtils"
    local x, y = unpack(xy)
    if BonusesUtils.of_color(obj, "Green") then
        tosprite("spr_bonuses.green"):draw({origin=Display.CENTER, scale={0.75, 0.75}}, {x + 16, y + 16})
        font_cached_load(settings.font, 14):draw({
            color = COL_PALE_GREEN,
            origin = Display.LEFT_CENTER
        }, {x + 32, y + 16}, obj.class_name)
    elseif  BonusesUtils.of_color(obj, "White") then
        tosprite("spr_bonuses.white"):draw({origin=Display.CENTER, scale={0.75, 0.75}}, {x + 16, y + 16})
        font_cached_load(settings.font, 14):draw({
            color = COL_WHITE,
            origin = Display.LEFT_CENTER
        }, {x + 32, y + 16}, obj.class_name)
    elseif  BonusesUtils.of_color(obj, "Blue") then
        tosprite("spr_bonuses.blue"):draw({origin=Display.CENTER, scale={0.75, 0.75}}, {x + 16, y + 16})
        font_cached_load(settings.font, 14):draw({
            color = COL_PALE_BLUE,
            origin = Display.LEFT_CENTER
        }, {x + 32, y + 16}, obj.class_name)
    elseif  BonusesUtils.of_color(obj, "Red") then
        tosprite("spr_bonuses.red"):draw({origin=Display.CENTER, scale={0.75, 0.75}}, {x + 16, y + 16})
        font_cached_load(settings.font, 14):draw({
            color = COL_PALE_RED,
            origin = Display.LEFT_CENTER
        }, {x + 32, y + 16}, obj.class_name)
    elseif  BonusesUtils.of_color(obj, "Black") then
        tosprite("spr_bonuses.black"):draw({origin=Display.CENTER, scale={0.75, 0.75}}, {x + 16, y + 16})
        font_cached_load(settings.font, 14):draw({
            color = COL_LIGHT_GRAY,
            origin = Display.LEFT_CENTER
        }, {x + 32, y + 16}, obj.class_name)
    end
end

Engine.resources_loaded = false

function Engine.on_item_get_miss(name)
    -- When looking up an item by name fails, generate it
    local GlobalData = require "core.GlobalData"
    append(GlobalData["__dynamic_item_generations"], name)
    local BonusesAll = require "items.BonusesAll"
    BonusesAll.generate(name)
    --.generate_by_name(name)
end

return Engine
