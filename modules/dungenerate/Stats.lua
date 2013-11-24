local M = nilprotect {} -- Submodule

typedef(M, "AptVec")[[
    Eff, Dam, Res, Def : float(0)
]]

typedef(M, "Aptitudes")[[
    Melee : AptVec
]]

typedef(M, "Stats")[[
    aptitudes : Aptitudes
]]

typedef(M, "StatContext")[[
    
]]

return M