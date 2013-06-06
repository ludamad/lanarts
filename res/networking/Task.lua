--- Create a tracked coroutine.
-- Provides a function to run all tasks, removing completed ones.

require "utils"

local task_list = {}

Task = newtype()

function Task:init(fn) 
    self.task_coroutine = coroutine.create(fn)
    task_list[#task_list] = self
end

function Task:resume(...)
    return coroutine.resume(self.task_coroutine,...)
end

-- Runs all tasks, removing any completed tasks from list
function tasks_run_all() 
    local i = 1
    while i < #task_list do
        local status, err = task_list[i]:resume()
        if not status then
            if err and err ~= 'cannot resume dead coroutine' then error(err) end
            table.remove(task_list, i)
        else
            i = i + 1
        end
    end
end