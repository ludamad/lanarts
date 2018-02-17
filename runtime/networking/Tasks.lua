--- Create a tracked coroutine.
-- Provides a function to run all tasks, removing completed ones.

local task_list = {}

local M = nilprotect {} -- Submodule
local Task = newtype()

function Task:init(fn, duration, err_handler)
    self.task_coroutine = coroutine.create(fn)
    self.err_handler = err_handler or false
    self.completed = false
    self.duration = duration or false
    self.completion_listeners = {}
    self.next_ret_values = {}
    self.started = false
end

function Task:start()
    if not self.started then
        task_list[#task_list+1] = self
        self.started = true
    end
end

-- Expose create function
M.create = Task.create

function Task:_handle_yield_result(status, f, ...)
    if status and f then
        -- Call any yielded functions on the main thread:
        print(...)
        self.next_ret_values[1] = f(...)
    end
    return status, f
end

function Task:_resume()
    return self:_handle_yield_result(coroutine.resume(self.task_coroutine, unpack(self.next_ret_values)))
end

function Task:resume(ms)
    ms = ms or self.duration
    if not ms then
        return self:_resume()
    else
        local timer = timer_create()
        local status, err = true, ""
        while timer:get_milliseconds() < ms do
            status, err = self:_resume()
            if not status then
                break
            end
        end
        return status, err
    end
end

function Task:on_finish(f)
    if self.completed then
        f()
    else
        table.insert(self.completion_listeners, f)
    end
end

-- Runs all tasks, removing any completed tasks from list
function M.run_all()
    local i = 1
    while i <= #task_list do
        local status, err = task_list[i]:resume()
        if not status then
            if err and err ~= 'cannot resume dead coroutine' then
            	if task_list[i].err_handler then
            		task_list[i]:err_handler()
            	else
		            if type(err) ~= "string" then err = pretty_tostring(err) end
		            print(err)
                end
            end
            task_list[i].completed = true
            for _, f in ipairs(task_list[i].completion_listeners) do
                f()
            end
            table.remove(task_list, i)
        else
            i = i + 1
        end
    end
end

return M
