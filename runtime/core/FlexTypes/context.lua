local M = nilprotect {} -- Submodule

local DefinitionContext = newtype()

function DefinitionContext:init(namespaces, --[[struct]] T) 
    self.namespaces = namespaces
    self.type = T
    self.upvalues = {}
end
function DefinitionContext:upvalue_add(k, v)
    if self.upvalues[k] ~= v then
        assertf(self.upvalues[k] == nil, "Conflicting value for injected upvalue '%s'!", k)
        self.upvalues[k] = v
    end
end
function DefinitionContext:type_resolve(typename)
    for _, namespace in pairs(self.namespaces) do
        if namespace[typename] then return namespace[typename] end
    end
    errorf("Unable to resolve '%s' as type!", typename)
end
function DefinitionContext:inject(func_s)
    local fields = table.key_list(self.upvalues)
    return ("function(%s)\n%s\nend"):format((","):join(fields), func_s)
end
function DefinitionContext:inject_and_load(func_s)
    local upvals = table.value_list(self.upvalues)
    return M.callstring(self:inject(func_s)) (unpack(upvals))
end

M.context_create = DefinitionContext.create

return M