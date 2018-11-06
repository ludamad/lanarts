local MiscSpellAndItemEffects = require "core.MiscSpellAndItemEffects"
local Map = require "core.Map"

local function mock_transfer(f)
    local prev = Map.transfer
    Map.transfer = do_nothing
    f()
    Map.transfer = prev
end

local function complete_maps(maps)
    local player = require("core.World").players[1].instance
    noscreen(function()
        mock_transfer(function()
            MiscSpellAndItemEffects.visit_all_maps(player)
        end)
        for _, map_label in ipairs(maps) do
            MiscSpellAndItemEffects.map_completion(player, map_label)
        end
    end)
end

local function go_to_portal(nearest_portal)
    local player = require("core.World").players[1].instance
    noscreen(function()
        MiscSpellAndItemEffects.use_portal_between_maps(
            player,
            -- Use a portal from the 'to' map, possibly counterintuitively.
            -- This means we end up on the 'from' map, near the portal we desire.
            nearest_portal.to,
            nearest_portal.from
        )
    end)
end

local function complete_checkpoint1()
    complete_maps {
        "Temple 1",
        "Temple 2",
        "Temple 3",
        "Snake Pit 1",
        "Snake Pit 2",
    }
end

local function complete_checkpoint2()
    complete_checkpoint1()
    complete_maps {
        "Outpost 1",
        "Outpost 2",
        "Outpost 3",
    }
end

local function complete_checkpoint3()
    complete_checkpoint2()
    complete_maps {
        "Plain Valley",
    }
end

local function complete_checkpoint4()
    complete_checkpoint3()
    complete_maps {
        "Gragh's Lair"
    }
end

return {
    checkpoint1 = function()
        complete_checkpoint1()
        go_to_portal {
            from = "Plain Valley",
            to = "Outpost 1"
        }
    end,
    checkpoint2 = function()
        complete_checkpoint2()
        go_to_portal {
            from = "Plain Valley",
            to = "Outpost 1"
        }
    end,
    checkpoint3 = function()
        complete_checkpoint3()
        go_to_portal {
            from = "Plain Valley",
            to = "Gragh's Lair"
        }
    end,
    checkpoint4 = function()
        complete_checkpoint4()
        go_to_portal {
            from = "Plain Valley",
            to = "Crypt"
        }
    end,
}
