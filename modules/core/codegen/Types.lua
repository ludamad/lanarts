local M = nilprotect {} -- Submodule

local function istype(t) return "type(${self}) == '" .. t .. "'" end
local function primitive(typecheck) 
    return {assign = "${self} = ${other}", typecheck = typecheck}
end

M.float = primitive(istype("number"))
M.double = primitive(istype("number"))
M.int = primitive(istype("number") .. " and (${self}%1==0)")
M.list = primitive(istype("table") .. " and not getmetatable(${self})")
M.object = primitive(istype("table") .. " and getmetatable(${self})")
M.any = primitive()

return M