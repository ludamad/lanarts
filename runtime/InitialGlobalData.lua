local World = require "core.World"

return function() 
    local var = random(1,2)
    return {
        keys_picked_up = {dummykey = true},
        lanarts_picked_up = {},
        n_lives = #World.players * 5,
        midrange_bastard_enemy = "Ciribot"--({"Ciribot", "Clown Mage"})[var]
    }
end
