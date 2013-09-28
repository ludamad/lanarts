local Display = import "core.Display"
Display.initialize("Lanarts", {640, 640}, false)

local AnsiCol = import "core.terminal.AnsiColors"
local GameState = import "core.GameState"
local Keys = import "core.Keyboard"
local Simulation = import ".Simulation"
local SimulationMap = import ".SimulationMap"
local Newregion = import "lanarts.maps.Newregion"

local function main()
    -- Load game content
    import "@DefineAll"

    -- Make sure EventLog.add prints to console instead
    local EventLog = import "core.ui.EventLog"
    EventLog.add = function(msg, color)
        AnsiCol.println(msg, AnsiCol.from_rgb(color or COL_WHITE))
    end

    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()

    local map, gmap = Newregion.overworld_create_helper()
    local SM = SimulationMap.create(map, gmap)
    local race, class = Simulation.choose_player_stats()
    local player = SM:add_player("Tester", race, class)
    local monster = SM:add_monster("Giant Rat")

    Display.view_snap(player.xy)
    while GameState.input_capture() and not Keys.key_pressed(Keys.ESCAPE) do
        SM:step()

        Display.draw_start()
        SM:draw()
        Display.draw_finish()
        GameState.wait(5)
    end
end

main()