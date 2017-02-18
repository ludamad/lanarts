local GameState = require "core.GameState"
local DeathScreen = require "menus.DeathScreen"
local Network = require "core.Network"
local World = require "core.World"

local M = {} -- Submodule

local events = {}

function events.PlayerInit(player) 
end

function events.PlayerDeath(player)
    if settings.regen_on_death then
        -- On soft-core, turn players into ghosts when they die:
        player.is_ghost = true
        player:add_effect("Reviving", 20 * 60) -- 20 'seconds'
        if #World.players == 1 then
            return false -- On soft-core single-player, never die.
        end
        local are_all_ghosts = true
        for _, player in ipairs(World.players) do
            if not player.instance.is_ghost then
                are_all_ghosts = false
                break
            end
        end
        -- On soft-core multiplayer, die if everyone is a ghost:
        if are_all_ghosts then
            GameState.score_board_store()
        end
        return are_all_ghosts
    else
        if #World.players == 1 then
            GameState.wait(100)
            DeathScreen.show()
        end
        GameState.score_board_store()
        return true
    end
end

function events.MonsterInit(monster)
end

function events.MonsterDeath(monster)
    if monster.name == "Pixulloch" then
        Engine.game_won()
    end
end

function events.PlayerEnterLevel()
    local single_player = (settings.connection_type == Network.NONE)
    if single_player then
        GameState.score_board_store()
        GameState.save("saves/savefile.save")
    end
end

-- Simple forwarding function
local testcase = os.getenv("LANARTS_TESTCASE") -- If a test is active, replace normal events.
function M.trigger_event(type, --[[Misc data]] ...)
    local module = testcase and require(testcase) or nil
    local event = (module or events)[type]
    if event then
        return event(...)
    end
end

function M.player_has_won()
    local WinningScreen = require "menus.WinningScreen"
    local game_loop_control = (require "GameLoop").loop_control

    GameState.wait(400)
    game_loop_control.game_is_over = true
    GameState.score_board_store(--[[Store winning entry]] true)
    WinningScreen.show()
end

return M
