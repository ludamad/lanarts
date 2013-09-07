local Sounds = import "@Sounds"
local DeathScreen = import "@menus.DeathScreen"
local PlayerFunctions = import "@Players"
local Relations = import "@objects.Relations"
local World = import "core.GameWorld"

local M = {} -- Submodule

local events = {}

events.PlayerInit = PlayerFunctions.player_init

function events.PlayerDeath(player)
    local classtable = {Mage=Sounds.death_mage, Fighter=Sounds.death_fighter, Archer=Sounds.death_archer}
    classtable[player.class_name]:play()
    if #World.players == 1 then
        DeathScreen.show()
    end
    Game.score_board_store()
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
    local single_player = (settings.connection_type == net.NONE)
    if single_player then
        Game.score_board_store()
        Game.save("saves/savefile.save")
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
    local WinningScreen = import "lanarts.menus.WinningScreen"
    local game_loop_control = (import "lanarts.GameLoop").loop_control

    Game.wait(400)
    game_loop_control.game_is_over = true
    Game.score_board_store(--[[Store winning entry]] true)
    WinningScreen.show()
end

return M