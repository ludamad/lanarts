return function() 
    local var = random(1,2)
    return {
        keys_picked_up = {dummykey = true},
        lanarts_picked_up = {},
        midrange_bastard_enemy = "Ciribot"--({"Ciribot", "Clown Mage"})[var]
    }
end
