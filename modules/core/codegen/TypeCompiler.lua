local TypeSchema = import ".TypeSchema"

local function callstring(str) 
    for i, s in ipairs(str:split("\n")) do
        print(i, s)
    end
    local func_loader, err = loadstring(str)
    if err then error(err) end
    return func_loader()
end

local TypeCompiler = newtype()

function TypeCompiler:init() end

--    local prelude = ("do local a,b = a%s, b%s\n"):format(R(attrib), R(attrib))

local FOUR_SPACE = '    '

function TypeCompiler:index_compile(S)
    local parts = {}
    for i,field in ipairs(S.fields) do
        local start = (i == 1 and "if " or "elseif ") 
        append(parts, FOUR_SPACE .. start .. field:index_body(i))
    end
    append(parts, FOUR_SPACE .. 'elseif rawget(TYPETABLE, k) then return rawget(TYPETABLE, k)')
    append(parts, FOUR_SPACE .. 'else error(("No such key \'%s\'"):format(k)) end')
    return ("\n"):join {
        "function(self, k)", 
        ("\n"):join(parts), 
        "end"
    }
end

function TypeCompiler:tostring_compile(S)
    local parts = {"local parts = {}"}
    for i,field in ipairs(S.fields) do
        append(parts, FOUR_SPACE .. ("parts[#parts+1] = '%s' .. '=' .. tostring(self[%d])"):format(
            field.name, i
        ))
    end
    append(parts, FOUR_SPACE .. "return '[' .. (' '):join(parts) .. ']'")
    return ("\n"):join {
        "function(self)", 
        ("\n"):join(parts), 
        "end"
    }
end

function TypeCompiler:newindex_compile(S)
    local parts = {}
    for i,field in ipairs(S.fields) do
        local start = (i == 1 and "if " or "elseif ") 
        append(parts, FOUR_SPACE ..  start .. field:newindex_body(i))
    end
    return ("\n"):join {
        "function(self, k, v)",
        ("\n"):join(parts),
        FOUR_SPACE .. 'else error(("No such key \'%s\'"):format(k)) end',
        "end"
    }
end

function TypeCompiler:create_compile(S)
    local signature_parts = {}
    for v in values(S.fields) do
        append(signature_parts, v.name)
    end
    local signature = "function(" .. (", "):join(signature_parts) .. ")"
    local parts = {}
    for i,field in ipairs(S.fields) do
        append(parts, FOUR_SPACE..field:assign_body(i, field.name))
    end
    return ("\n"):join {
        signature,
        FOUR_SPACE .. "local self = setmetatable({}, METATABLE)", 
        ("\n"):join(parts), 
        FOUR_SPACE .. "return self",
        "end"
    }
end

function TypeCompiler:compile(S)
    if type(S) == "string" then 
        S = TypeSchema.create(S)
    end
    local parts = {
        "local rawget,rawset,setmetatable,getmetatable=rawget,rawset,setmetatable,getmetatable", -- Performance 
        "local TYPETABLE = {} ; local METATABLE = {}"
    }
    append(parts, "METATABLE.__index = " .. self:index_compile(S))
    append(parts, "METATABLE.__newindex = " .. self:newindex_compile(S))
    append(parts, "METATABLE.__tostring = " .. self:tostring_compile(S))
    append(parts, "TYPETABLE.create = " .. self:create_compile(S))
    append(parts, "return TYPETABLE")
    local callable = ("\n"):join(parts)
    local type = callstring(callable)
    type.__builder = self
    return type
end

return TypeCompiler