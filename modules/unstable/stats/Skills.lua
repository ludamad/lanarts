local Skills = import "@Skills"
local StatContext = import "@StatContext"
local Apts = import ".AptitudeTypes"

local M = {} -- Submodule

M.melee = Skills.define {
    name = "Melee",
    description = "Increases efficacy in physical combat.",

    on_calculate = function(level, user)
        StatContext.add_effectiveness(user, Apts.MELEE, level)
    end
}

M.magic = Skills.define {
    name = "Magic",
    description = "Increases efficacy in the magic arts.",

    on_calculate = function(level, user)
        StatContext.add_effectiveness(user, Apts.MAGIC, level)
    end
}

M.ranged = Skills.define {
    name = "Ranged",
    description = "Increases efficacy in ranged weaponry.",

    on_calculate = function(level, user)
        StatContext.add_effectiveness(user, Apts.RANGED, level)
    end
}

return M