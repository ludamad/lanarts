--[[
    Implements the game loop, which calls the step & draw events of all objects.
]]

local Display = require "core.Display"
local EventLog = require "ui.EventLog"

local GameState = require "core.GameState"
local World = require "core.World"
local Map = require "core.Map"
local HelpOverlay = require "HelpOverlay"
local Network = require "core.Network"
local Keys = require "core.Keyboard"

local M = {} -- Submodule

--- Externally visible control structure for the main loop
-- Global mutable state, but no serialization guards, as storing it is not needed.
M.loop_control = {
    game_is_over = false,
    game_is_paused = false,
    startup_function = do_nothing
}

function M.exit_game()
    M.loop_control.game_is_over = true
end

local HEADLESS = os.getenv("LANARTS_HEADLESS")
function M.game_step(steponly)
    if HEADLESS then
        steponly = true
    end
    perf.timing_begin("**Game Frame**")

    perf.timing_begin("**Sync Message**")
    Network.sync_message_consume()
    perf.timing_end("**Sync Message**")

    local timer = timer_create()

    if not steponly then
        perf.timing_begin("**Draw**")
        GameState.draw()
        perf.timing_end("**Draw**")
    end

    perf.timing_begin("**Step**")
    if not M.loop_control.game_is_paused and not GameState.step() then
        return false
    end
    perf.timing_end("**Step**")

    -- Backup save every ~20 minutes
    if GameState.frame > 0 and GameState.frame % (100 * 60 * 20) == 0 then
        for _ in screens() do
            EventLog.add("BACKING UP SAVE!", COL_WHITE)
        end
        local GameState = require "core.GameState"
        GameState.save("saves/backup-frame-" .. GameState.frame .. ".save")
    end
    -- ROBUSTNESS
    -- Will be called after a game restarts during step event, but for a newly generated game world.
    -- See robustness note below which assures that each run has the same ordering of input_handle's.
    if not GameState.input_handle() then
        return false
    end

    local surplus = settings.time_per_step - timer:get_milliseconds()

    perf.timing_begin("**Surplus**")
    if not HEADLESS and not __EMSCRIPTEN then
        GameState.wait(surplus)
    end
    perf.timing_end("**Surplus**")

    perf.timing_end("**Game Frame**")

    return true
end

function M.post_draw()
    local player = World.local_player
    for _, pdata in ipairs(World.players) do
        local p = pdata.instance
        if p and not p:is_local_player() and p.map == player.map then
            Fonts.small:draw({color=COL_WHITE, origin=Display.CENTER}, Display.screen_coords{p.x, p.y-18}, pdata.name)
        end
    end
end

local fps_timer = timer_create()
local fps_count, fps_lastframe, fps = 1, 0, nil

function M.overlay_draw()
    local frame_increase = math.max(0, GameState.frame - fps_lastframe)
    fps_lastframe = GameState.frame
    fps_count = fps_count + frame_increase

    if fps then
        local w,h = unpack( Display.display_size )
        Fonts.small:draw( {origin=Display.RIGHT_BOTTOM}, {w, h}, "FPS: " .. math.floor(fps) )
    end

    local ms = fps_timer:get_milliseconds()
    if ms > 1000 then
        fps = fps_count / ms * 1000
        fps_timer:start()
        fps_count = 0
    end
    --HelpOverlay.draw()
end

return M
