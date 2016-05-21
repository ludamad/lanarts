
local M = nilprotect {} -- Submodule

function M.main(cmd_args)
    local Display = import "core.Display"
    Display.initialize("TestMap", {settings.view_width, settings.view_height}, settings.fullscreen)
    local AnsiCol = import "terminal.AnsiColors"
    local GameState = import "core.GameState"
    local Keys = import "core.Keyboard"
    local Simulation = import ".Simulation"
    local GameMap = import "@maps.GameMap"
    local Newregion = import "lanarts.maps.Newregion"
    local StatContext = import "@StatContext"
    local GameInterface = import "@ui.GameInterface"

    -- Load game content
    import "@DefineAll"

    -- Make sure EventLog.add prints to console instead
    local EventLog = import "ui.EventLog"
    EventLog.add = function(msg, color)
        AnsiCol.println(msg, AnsiCol.from_rgb(color or COL_WHITE))
    end

    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()

    local map, gmap = Newregion.overworld_create_helper(--[[Empty]] true)
    local GM = GameMap.create(map, gmap)
    local race, class = Simulation.choose_player_stats(cmd_args)
    local player = GM:add_player("Tester", race, class)
    player:stat_context().base.hp = 1000
    player:stat_context().base.max_hp = 1000
    player:stat_context().derived.hp = 1000
    player:stat_context().derived.max_hp = 1000

    for i=1,15 do GM:add_monster("Gnome Skeleton") end
    for i=1,5 do GM:add_monster("Cloud Elemental") end

    local interface = GameInterface.create(Display.display_size, player)
    Display.view_snap(player.xy)
    while GameState.input_capture() and not Keys.key_pressed(Keys.ESCAPE) do
        perf.timing_begin("**Step**")
        Display.view_snap(player.xy)
        GM:step()
        perf.timing_end("**Step**")
        perf.timing_begin("**Draw**")
        Display.draw_start()
        GM:draw()
        interface:draw()
        Display.draw_finish()
        perf.timing_end("**Draw**")
--        GameState.wait(10)
    end
    perf.timing_print()
    print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
    print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )
end

return M