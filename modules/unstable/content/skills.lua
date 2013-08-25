local Skills = import "@Skills"
local StatContext = import "@StatContext"
local AptitudeTypes = import ".aptitude_types"

local M = {} -- Submodule

Skills.define {
    name = "Melee",
    description = "Increases efficacy in physical combat.",

    on_calculate = function(level, user)
        StatContext.effectiveness_add(user, AptitudeTypes.melee, level)
    end
}

Skills.define {
    name = "Magic",
    description = "Increases efficacy in the magic arts.",

    on_calculate = function(level, user)
        StatContext.effectiveness_add(user, AptitudeTypes.magic, level)
    end
}

Skills.define {
    name = "Ranged",
    description = "Increases efficacy in ranged weaponry.",

    on_calculate = function(level, user)
        StatContext.effectiveness_add(user, AptitudeTypes.ranged, level)
    end
}

return M