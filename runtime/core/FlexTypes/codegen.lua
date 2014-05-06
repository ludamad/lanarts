local AnsiColors = import "terminal.AnsiColors"
local flexcore = import "@flexcore"

local Struct = import ".Struct"

local M = nilprotect {} -- Submodule

local AC = AnsiColors
local function hilight(s)
    for k in values {"local", "elseif", "else", "return", "function", "end", "rawget", "rawset"} do
        for pattern in values {"$", "[%s%(]"} do
            s = s:gsub(k..pattern, function(p1) 
                return p1:gsub(k, function(p2) return AC.WHITE(p2) end) 
            end)
        end
    end ; return s
end

function M.callstring(str)
    for i, s in ipairs(str:split("\n")) do
        local lineno = ("%3d) "):format(i)
        print( AC.WHITE(lineno) .. hilight(s))
    end
    local func_loader, err = loadstring(str)
    if err then error(err) end
    return func_loader()
end


local FuncBuilder = newtype() ; M.FuncBuilder = FuncBuilder

function FuncBuilder:init(args)
    self.signature = (", "):join(args)
    self.parts = {}
    self.indent = 1
end

function FuncBuilder:append(line)
    if type(line) == "table" then self:add(line)
    else local P = self.parts ; P[#P] = P[#P] .. line end
end
function FuncBuilder:add(line, ...)
    if type(line) == "table" then
        -- Actually a list of strings
        for real_line in values(line) do
            self:add(real_line, ...)
        end
    else
        if ... then line = line:format(...) end
        for i=1,self.indent do line = '    ' .. line end
        append(self.parts, line)
    end
end

function FuncBuilder:emit()
    return ("function(%s)\n%s\nend"):format(
        self.signature, ("\n"):join(self.parts)
    )
end

function M.func_string(args, ...)
    local mb = FuncBuilder.create(args)
    for part in values {...} do mb:add(part) end
    return mb:emit()
end
function M.method_string(T, args, ...)
    return M.func_string({"self", unpack(args)}, T:emit_pre_method(), ...)
end

M.metamethods = {}
M.methods = {}

function M.metamethods.__index(--[[MethodContext]] MC)
    local T = MC.type
    local body = {}
    for name in T:aliases() do
        appendf(body, (#body==0 and "if" or "elseif ") .. " k == '%s' then return %s", name, T:emit_direct_get(name))
    end
    return M.method_string(T, {"k"},
        unpack(body),
        'elseif rawget(TYPETABLE, k) then return rawget(TYPETABLE, k)',
        'else error(("No such key \'%s\'"):format(k)) end'
    )
end

function M.compile_type(--[[MethodContext]] MC)
    return {
    }
end

return M