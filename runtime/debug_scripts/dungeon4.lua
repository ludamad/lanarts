local utils = require "debug_scripts.utils"

dofile "debug_scripts/dungeon3.lua"

utils.complete_maps {
    "Gragh's Lair",
    nearest_portal = {
        from = "Plain Valley", 
        to = "Crypt"
    }
}
