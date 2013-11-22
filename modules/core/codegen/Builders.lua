local Types = import ".Types"

local M = nilprotect {} -- Submodule

local Field = newtype()
function Field:init(name, type, is_ref)
	self.name, self.type, self.is_ref = name, type, is_ref
end
function Field:assign_body(slot, other)
    return ("rawset(self, %d, %s)"):format(slot, other)
end
function Field:index_body(slot)
    return ('k == "%s" then return rawget(self, %d)'):format(self.name, slot)
end
function Field:typecheck_body(val)
    if not self.type.typecheck then return nil end
    return self.type.typecheck:interpolate { self = val}
end
function Field:newindex_body(slot)
    local typecheck = self:typecheck_body("v") 
    local typecheck_str = typecheck and 'assert('..typecheck..',k) ; ' or ' '
    return ('k == "%s" then %s%s'):format(self.name, typecheck_str, self:assign_body(slot, "v"))
end

local TypeBuilder = newtype()
function TypeBuilder:init(source, fields)
    self.source, self.fields = source, fields
end

local function callstring(str) 
    for i, s in ipairs(str:split("\n")) do
        print(i, s)
    end
    local func_loader, err = loadstring(str)
    if err then error(err) end
    return func_loader()
end

M.callstring = callstring -- For testing purposes

--    local prelude = ("do local a,b = a%s, b%s\n"):format(R(attrib), R(attrib))

local FOUR_SPACE = '    '

function TypeBuilder:index_compile()
    local parts = {}
    for i,field in ipairs(self.fields) do
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

function TypeBuilder:tostring_compile()
    local parts = {"local parts = {}"}
    for i,field in ipairs(self.fields) do
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

function TypeBuilder:newindex_compile()
    local parts = {}
    for i,field in ipairs(self.fields) do
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

function TypeBuilder:create_compile()
    local signature_parts = {}
    for v in values(self.fields) do
        append(signature_parts, v.name)
    end
    local signature = "function(" .. (", "):join(signature_parts) .. ")"
    local parts = {}
    for i,field in ipairs(self.fields) do
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

function TypeBuilder:compile()
    local mt = {}
    mt.__index, mt.create = self:index_compile(), self:create_compile()
    local parts = {
        "local rawget,rawset,setmetatable,getmetatable=rawget,rawset,setmetatable,getmetatable", -- Performance 
        "local TYPETABLE = {} ; local METATABLE = {}"
    }
    append(parts, "METATABLE.__index = " .. self:index_compile())
    append(parts, "METATABLE.__newindex = " .. self:newindex_compile())
    append(parts, "METATABLE.__tostring = " .. self:tostring_compile())
    append(parts, "TYPETABLE.create = " .. self:create_compile())
    append(parts, "return TYPETABLE")
    local callable = ("\n"):join(parts)
    return callstring(callable)
end

-- Create a TypeBuilder from a string representation of a type
function M.type_parse(str)
    local fields = {}
    local lines = str:split("\n")
    for i,line in ipairs(lines) do
        if line:match("^%s*[%w_,%s]+:%s*%w+%s*") then
            local parts = line:split(":")
            local names = parts[1]:split(",")
            local typename = parts[2]:trim()
            if typename == "*" then typename = "any" end
            local type = assert(Types[typename], "Invalid type: " .. typename)
            for name in values(names) do
                name = name:trim()
                assert(name:match("^[%w_]+$"), "'" .. name .. "' is not a valid name.")
                append(fields, Field.create(name, type, false))
            end
        elseif line:match("^%w*$") then
            error("Unexpected line " .. i .. " in type_parse: '" .. line .. "'")
        end
    end
    return TypeBuilder.create(str, fields)
end

return M