local utils = require "debug_scripts.utils"

dofile "debug_scripts/dungeon2.lua"

utils.complete_maps {
    "Plain Valley",
    nearest_portal = {
        from = "Plain Valley", 
        to = "Gragh's Lair"
    }
}
