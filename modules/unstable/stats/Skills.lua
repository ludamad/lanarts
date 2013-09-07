local Skills = import "@Skills"
local StatContext = import "@StatContext"
local AptitudeTypes = import ".AptitudeTypes"

local M = {} -- Submodule

M.melee = Skills.define {
    name = "Melee",
    description = "Increases efficacy in physical combat.",

    on_calculate = function(level, user)
        StatContext.add_effectiveness(user, AptitudeTypes.melee, level)
    end
}

M.magic = Skills.define {
    name = "Magic",
    description = "Increases efficacy in the magic arts.",

    on_calculate = function(level, user)
        StatContext.add_effectiveness(user, AptitudeTypes.magic, level)
    end
}

M.ranged = Skills.define {
    name = "Ranged",
    description = "Increases efficacy in ranged weaponry.",

    on_calculate = function(level, user)
        StatContext.add_effectiveness(user, AptitudeTypes.ranged, level)
    end
}

return M