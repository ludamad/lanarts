-- Usage: 'SpellType.define { ... attributes ... }', 'SpellType.lookup(<name or ID>)'
local ResourceTypes = import "@ResourceTypes"
local HookSet = import "@HookSet"

local function create(args)
    local type = newtype()
    type.priority = HookSet.MIN_PRIORITY
    for k,v in pairs(args) do
        type[k] = v
    end
    return type
end

local StatusType = ResourceTypes.type_create(create)

local function resolve(status_type)
    if type(status_type) ~= "string" then return status_type end
    return StatusType.lookup(status_type)
end

function StatusType.get_hook(hooks, status_type)
    status_type = resolve(status_type)
    for hook in values(hooks) do
        if getmetatable(hook) == status_type then
            return hook
        end
    end
    return nil
end

function StatusType.update_hook(hooks, status_type, ...)
    status_type = resolve(status_type)
    local hook = StatusType.get_hook(hooks, status_type)
    if not hook then
        hook = status_type.create(...)
        hooks:add_hook(hook)
    else
        hook:update(...)
    end
    return hook
end

return StatusType