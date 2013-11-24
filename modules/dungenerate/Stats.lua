local M = nilprotect {} -- Submodule

typedef(M) "AptVec" [[
    Eff, Dam, Res, Def : float
]]

typedef(M) "Aptitudes" [[
    Melee : AptVec
]]

typedef(M) "Stats" [[
    aptitudes : Aptitudes
]]

typedef(M) "StatContext" [[
    base : Stats
    derived : Stats
]]

return M