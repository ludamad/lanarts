local StatML = import "statml.StatML"
local StatMLUtil = import "statml.StatMLUtil"
local M = nilprotect {} -- Submodule

typedef(M) "Aptitude" [[
    uses_eff, uses_dam, uses_res, uses_def : bool
]]

local apt_tests = {"Eff", "Dam", "Res", "Def"}
StatML.define_map_parse {
    Aptitude = function(node)
        local uses = node:remove("uses")
        local bools = StatMLUtil.bool_vector_parse(uses, apt_tests)
        return M.Aptitude.create(bools[1],bools[2],bools[3],bools[4])
    end
}

function M.statml_onload()
    typedef(M) "AptVec" [[
        Eff, Dam, Res, Def : float
    ]]
    
    typedef(M) "Aptitudes" (
        (", "):join()
    )
    
    typedef(M) "Stats" [[
        Aptitudes
    ]]
    
    typedef(M) "StatContext" [[
        base, derived : Stats
    ]]
end

return M