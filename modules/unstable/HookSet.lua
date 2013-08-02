local HookSet = newtype()

function HookSet:init()
    -- List of hooks, sorted by priority
    self.hooks = {}
    -- Recently added hooks that have not yet been placed into sorted list
    self.new_hooks = {}
end

function HookSet:hook_add(hook)
    table.insert(self.new_hooks, hook)
end

-- Place recently added hooks into the sorted list
local function merge_new_hooks(self)
    for hook in values(self.new_hooks) do
        if hook.priority == nil then
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

function HookSet:on_step(...)
    merge_new_hooks(self)
    -- Perform the hook step event, filtering finished hooks.
    local new_len, i = 0, 1
    for i=1,#self.hooks do
        local hook = self.hooks[i]
        if hook.on_step then
            if hook:on_step(...) ~= false then
               -- Perform compacting
               new_len = new_len + 1
               self.hooks[new_len] = hook
            end
        end
    end
    -- Perform clean-up
    for i=new_len+1,#self.hooks do
        self.hooks[i] = nil
    end
end

function HookSet:perform(method_name, ...)
    merge_new_hooks(self)
    for hook in values(self.hooks) do
        local method = hook[method_name]
        if method then method(hook, ...) end
    end
end

return HookSet
