--- Create a tracked coroutine.
-- Provides a function to run all tasks, removing completed ones.

local task_list = {}

local M = nilprotect {} -- Submodule
local Task = newtype()

function Task:init(fn, err_handler) 
    self.task_coroutine = coroutine.create(fn)
    self.err_handler = err_handler or false
    task_list[#task_list+1] = self
end

-- Expose create function
M.create = Task.create

function Task:resume()
    return coroutine.resume(self.task_coroutine)
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
            table.remove(task_list, i)
        else
            i = i + 1
        end
    end
end

return M
