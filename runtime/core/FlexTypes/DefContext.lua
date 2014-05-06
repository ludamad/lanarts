local primitives = import "@primitives"

local M = nilprotect {} -- Submodule

local Struct = newtype()

function Struct:init(name)
    self.name = name or '<anon>'
    self.fields = {}
    self.subfields = {}
    self.metadata = {}
    self.aliases = {}
end

local DefContext = newtype()

function DefContext:init(namespaces, --[[struct]] T) 
    self.namespaces = table.clone(namespaces)
    append(self.namespaces)
    self.type = T
    self.upvalues = {}
end
function DefContext:upvalue_add(k, v)
    if self.upvalues[k] ~= v then
        assertf(self.upvalues[k] == nil, "Conflicting value for injected upvalue '%s'!", k)
        self.upvalues[k] = v
    end
end
function DefContext:field_def(name, typename, initializer, --[[Optional]] is_embedded)
    for _, namespace in pairs(self.namespaces) do
        if namespace[typename] then return namespace[typename] end
    end
    errorf("Unable to resolve '%s' as type!", typename)
end
function DefContext:inject(func_s)
    local fields = table.key_list(self.upvalues)
    return ("function(%s)\n%s\nend"):format((","):join(fields), func_s)
end
function DefContext:inject_and_load(func_s)
    local upvals = table.value_list(self.upvalues)
    return M.callstring(self:inject(func_s)) (unpack(upvals))
end

M.context_create = DefContext.create

return M