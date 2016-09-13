local Sounds = require "Sounds"
local GameState = require "core.GameState"
local DeathScreen = require "menus.DeathScreen"
local PlayerFunctions = require "Players"
local Relations = require "objects.Relations"
local Network = require "core.Network"
local World = require "core.World"

local M = {} -- Submodule

local events = {}

events.PlayerInit = PlayerFunctions.player_init

function events.PlayerDeath(player)
    local classtable = {Mage=Sounds.death_mage, Fighter=Sounds.death_fighter, Archer=Sounds.death_archer}
    classtable[player.class_name]:play()
    if #World.players == 1 then
        DeathScreen.show()
    end
    GameState.score_board_store()
end

function events.MonsterInit(monster)
    monster.team = Relations.TEAM_MONSTER_ROOT
end

function events.MonsterDeath(monster)
    local monstersounds = { ["Giant Rat"]=Sounds.death_rat ,
                            ["Giant Bat"]=Sounds.death_bat ,
                            ["Giant Spider"]=Sounds.death_spider ,
                            ["Hound"]=Sounds.death_hound ,
                            ["Sheep"]=Sounds.death_sheep ,
                            ["Giant Frog"]=Sounds.death_frog ,
                            ["Green Slime"]=Sounds.death_slime ,
                            ["Red Slime"]=Sounds.death_slime                           
                        }
    if monstersounds[monster.name] ~= nil then
        monstersounds[monster.name]:play()
    end
    if monster.name == "Zin" then
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
function M.trigger_event(type, --[[Misc data]] ...)
    local event = events[type]
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