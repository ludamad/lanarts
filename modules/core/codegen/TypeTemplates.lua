local M = nilprotect {} -- Submodule

local function istype(t) return "type(${self}) == '" .. t .. "'" end
local function primitive(typecheck) 
    return {assign = "${self} = ${other}", typecheck}
end

M.float = primitive(istype("number"))
M.double = primitive(istype("number"))
M.int = primitive(istype("number") .. "and ${self}%1==0")

return M