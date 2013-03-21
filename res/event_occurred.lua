require "sound"

local events = {}

function events.PlayerDeath(player)
    local classtable = {Mage=sounds.death_mage, Fighter=sounds.death_fighter, Archer=sounds.death_archer}
    classtable[player.class_name]:play()
    Game.score_board_store()
end

function events.MonsterDeath(monster)
    local monstersounds = { ["Giant Rat"]=sounds.death_rat ,
                            ["Giant Bat"]=sounds.death_bat ,
                            ["Giant Spider"]=sounds.death_spider ,
                            ["Hound"]=sounds.death_hound ,
                            ["Sheep"]=sounds.death_sheep ,
                            ["Giant Frog"]=sounds.death_frog ,
                            ["Green Slime"]=sounds.death_slime ,
                            ["Red Slime"]=sounds.death_slime                           
                        }
    if monstersounds[monster.name] ~= nil then
        monstersounds[monster.name]:play()
    end
    if monster.name == "Zin" then
        Engine.Game_won()
    end
end

function events.PlayerEnterLevel()
    local single_player = (settings.connection_type == net.NONE)
    if single_player then
        Game.score_board_store()
        Game.save("res/savefile.save")
    end
end

-- Simple forwarding function
function event_occurred(type, --[[Misc data]] ...)
    local event = events[type]
    if event then
        return event(...)
    end
end

function player_has_won()
    require "winning_screen"
    require "game_loop"

    Game.wait(400)
    game_loop_control.game_is_over = true
    Game.score_board_store(--[[Store winning entry]] true)
    winning_screen_show()
end
