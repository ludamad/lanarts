local AnsiColors = import "terminal.AnsiColors"
local M = nilprotect {} -- Submodule

local MethodBuilder = newtype() ; M.MethodBuilder = MethodBuilder

-- Field kinds

M.OBJECT_FIELD,M.PRIMITIVE_FIELD,M.HIDDEN_FIELD = 1,2,3

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

function MethodBuilder:init(typeinfo, ...)
    self.typeinfo = typeinfo
    self.signature = (", "):join{...}
    self.parts = {}
    self.indent = 1
end

function MethodBuilder:append(line)
    if type(line) == "table" then self:add(line)
    else local P = self.parts ; P[#P] = P[#P] .. line end
end
function MethodBuilder:add(line, ...)
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

function MethodBuilder:emit()
    return ("function(%s)\n%s\nend"):format(
        self.signature, ("\n"):join(self.parts)
    )
end

local UpvalueInjector = newtype() ; M.UpvalueInjector = UpvalueInjector

function UpvalueInjector:init()
    self.upvalues = {}
end
function UpvalueInjector:upvalue_add(k, v)
    if self.upvalues[k] ~= v then
        assertf(self.upvalues[k] == nil, "Conflicting value for injected upvalue '%s'!", k)
        self.upvalues[k] = v
    end
end
function UpvalueInjector:inject(func_s)
    local fields = table.key_list(self.upvalues)
    return fmt("function(%s)\n%s\nend", 
        (","):join(fields), func_s
    )
end
function UpvalueInjector:inject_and_load(func_s)
    return M.callstring(self:inject(func_s)) (
        unpack(table.value_list(self.upvalues))
    )
end

local FieldBase = newtype()
function FieldBase:init(typename, --[[Optional]] initializer)
	self.typename = typename
	self.initializer = initializer or false
end

function FieldBase:emit_assign(get_s, set_f, val_s, --[[Optional]] dont_check_value)
    local valuecheck = self:emit_valuecheck(val_s)
    if dont_check_value or (valuecheck == nil) then
        return set_f(val_s)
    else
        return fmt("assert(%s, 'Incorrect type (failed %s).') ; %s", 
            valuecheck, valuecheck:gsub("'",'"'), set_f(val_s))
    end
end

function FieldBase:emit_default() 
    return nil
end

function FieldBase:emit_valuecheck(val_s, meta_s)
    return nil
end

function M.field_type_create()
	return newtype {parent = FieldBase}
end

return M