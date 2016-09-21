--[[
    Implements the game loop, which calls the step & draw events of all objects.
]]

local Display = require "core.Display"
local EventLog = require "ui.EventLog"

local GameState = require "core.GameState"
local World = require "core.World"
local Map = require "core.Map"
local help_overlay = require "HelpOverlay"
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

local function game_loop_body(steponly)
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

    if not GameState.input_handle() then 
        return false 
    end

    local surplus = settings.time_per_step - timer:get_milliseconds()

    perf.timing_begin("**Surplus**")
    GameState.wait(surplus)
    perf.timing_end("**Surplus**")

    perf.timing_end("**Game Frame**")

    return true
end


function M.post_draw()
    local player = World.local_player
    for pdata in values(World.players) do
        local p = pdata.instance
        if not p:is_local_player() and p.map == player.map then
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
    help_overlay_draw()
end


function M.run_loop()
    M.loop_control.startup_function()

    GameState.input_capture()

    while true do 
        local single_player = (settings.connection_type == Network.NONE)
    
        if Keys.key_pressed(Keys.F2) and single_player then 
            GameState.resources_load()
        end
    
        if Keys.key_pressed(Keys.F4) then 
            M.loop_control.game_is_paused = not M.loop_control.game_is_paused
        end

        if Keys.key_pressed(Keys.ESCAPE) then 
            EventLog.add("Press Shift + Esc to exit, your progress will be saved.")
        end

        local steponly = (GameState.frame % settings.steps_per_draw ~= 0)
        if not game_loop_body(steponly) then
            if single_player then
                GameState.score_board_store()
                GameState.save("saves/savefile.save")
            end
            break
        end

        if M.loop_control.game_is_over then
            break
        end

        if Keys.key_pressed(Keys.F5) or Network.should_send_sync_message() then
            GameState.input_capture(true) -- reset input
            Network.sync_message_send()
        end
    end

    perf.timing_print()

    print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
    print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )
end

return M
