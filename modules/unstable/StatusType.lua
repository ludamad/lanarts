-- Usage: 'SpellType.define { ... attributes ... }', 'SpellType.lookup(<name or ID>)'
local ResourceTypes = import "@ResourceTypes"
local HookSet = import "@HookSet"

-- Base for time-limited statuses
local TimeLimitedBase = {
    init = function(self, stats, time_left)
        self.time_left = time_left
    end,
    on_update = function(self, stats, time_left)
        self.time_left = math.max(self.time_left, time_left)
    end,
    on_step = function(self, stats)
        self.time_left = self.time_left - 1
        if self.time_left <= 0 then
            if self.on_deregister then
                self:on_deregister(stats)
            end
            return true -- Deregister
        end
    end
}

local function create(args)
    local type = newtype()
    type.priority = HookSet.MIN_PRIORITY
    for k,v in pairs(args) do
        type[k] = v
    end

    if type.time_limited then
        type.base = TimeLimitedBase
        for k,v in pairs(TimeLimitedBase) do
            if not type[k] then type[k] = v end
        end
    end

    return type
end

local StatusType = ResourceTypes.type_create(create)

-- Like 'lookup', but returns tables as they are.
function StatusType.resolve(status_type)
    if type(status_type) == "table" then return status_type end
    return StatusType.lookup(status_type)
end

function StatusType.get_hook(hooks, status_type)
    status_type = StatusType.resolve(status_type)
    for hook in values(hooks) do
        if getmetatable(hook) == status_type then
            return hook
        end
    end
    return nil
end

function StatusType.update_hook(hooks, status_type, ...)
    status_type = StatusType.resolve(status_type)
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