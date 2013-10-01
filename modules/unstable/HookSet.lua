local M = nilprotect {} -- Submodule

local HookSet = newtype()

M.MIN_PRIORITY = -math.huge

function HookSet:init()
    -- List of hooks, sorted by priority
    self.hooks = {}
    -- Recently added hooks that have not yet been placed into sorted list
    self.new_hooks = {}
end

function HookSet:__copy(other)
    -- Ensure a shallow copy when used with table.deep_copy
    other.hooks = rawget(other, "hooks") or {}
    other.new_hooks = rawget(other, "new_hooks") or {}
    table.copy(self.hooks, other.hooks)
    table.copy(self.new_hooks, other.new_hooks)
end

function HookSet:add_hook(hook)
    table.insert(self.new_hooks, hook)
end

-- Place recently added hooks into the sorted list
function HookSet:merge_new_hooks()
    for hook in values(self.new_hooks) do
        if hook.priority <= M.MIN_PRIORITY then
            table.insert(self.hooks, hook)
        else 
            local i = #self.hooks
            while i >= 1 do
                local p = self.hooks[i].priority
                if p ~= nil and p > hook.priority then
                    break
                end
                i = i - 1
            end
            table.insert(self.hooks, i+1, hook)
        end
    end
    table.clear(self.new_hooks)
end

function HookSet:on_draw(stats, drawf, options)
    self:merge_new_hooks()
    for hook in values(self.hooks) do
        if hook.on_draw then
            drawf, options = hook:on_draw(stats, drawf, options)
        end
    end
    return drawf, options
end

function HookSet:on_step(...)
    self:merge_new_hooks()
    -- Perform the hook step event, filtering finished hooks.
    local new_len, i = 0, 1
    for i=1,#self.hooks do
        local hook = self.hooks[i]
        if hook.on_step and hook:on_step(...) then
           -- Perform compacting
           self.hooks[new_len] = hook
        else
           new_len = new_len + 1
        end
    end
    -- Perform clean-up
    for i=new_len+1,#self.hooks do
        self.hooks[i] = nil
    end
end

function HookSet:perform(method_name, ...)
    self:merge_new_hooks()
    for hook in values(self.hooks) do
        local method = hook[method_name]
        if method then method(hook, ...) end
    end
end

M.create = HookSet.create
return M