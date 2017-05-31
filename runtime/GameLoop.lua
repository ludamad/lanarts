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

local HEADLESS = os.getenv("LANARTS_HEADLESS")
local function game_loop_body(steponly)
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

    -- ROBUSTNESS
    -- Will be called after a game restarts during step event, but for a newly generated game world.
    -- See robustness note below which assures that each run has the same ordering of input_handle's.
    if not GameState.input_handle() then 
        return false 
    end

    local surplus = settings.time_per_step - timer:get_milliseconds()

    perf.timing_begin("**Surplus**")
    if not HEADLESS then 
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
    HelpOverlay.draw()
end

function M.run_loop()
    M.loop_control.startup_function()

    GameState.input_capture()
    if require("tests.main").testcase then
        require("tests.main").testcase:game_start()
    end
    if argv_configuration.load_file then -- Global from GlobalVariableSetup.lua
        if file_exists(argv_configuration.load_file) then
            GameState.load(argv_configuration.load_file)
        else
            error("'" .. argv_configuration.load_file .. "' does not exist!")
        end
    end

    -- ROBUSTNESS
    -- Since when a game restarts, an input_handle is called during game_loop_body after step(),
    -- we call input_handle here for reproducibility.
    if not GameState.input_handle() then 
        return false 
    end

    while true do 
        local single_player = (settings.connection_type == Network.NONE)
    
        -- TODO fix hotloading
        -- Crashes with unknown effects
        -- if Keys.key_pressed(Keys.F2) and single_player then 
        --     GameState.resources_load()
        -- end
    
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
                GameState.save(argv_configuration.save_file or "saves/savefile.save")
            end
            break
        end

        if M.loop_control.game_is_over then
            break
        end

        if Keys.key_pressed(Keys.F5) then -- or Network.should_send_sync_message() then
            GameState.input_capture(true) -- reset input
            Network.sync_message_send()
        end
    end

    perf.timing_print()

    print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
    print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )

    if argv_configuration.load_file then
        os.exit()
    end
end

return M
