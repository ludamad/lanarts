local M = { }
local TASKS = { }
local Task = newtype({
  init = function(self, fn, duration, err_handler)
    self.task_coroutine = coroutine.create(fn)
    self.err_handler = err_handler or false
    self.completed = false
    self.duration = duration or false
    self.completion_listeners = { }
    self.next_ret_values = { }
    self.started = false
  end,
  start = function(self)
    if not self.started then
      local step_task
      step_task = function()
        return self:step(step_task)
      end
      return append(TASKS, step_task)
    end
  end,
  step = function(self, next_step)
    local status, err = self:resume()
    if not status then
      if err and err ~= 'cannot resume dead coroutine' then
        if self.err_handler then
          self:err_handler()
        else
          if type(err) ~= "string" then
            err = pretty_tostring(err)
          end
          print(err)
        end
      end
      self.completed = true
      return nil
    end
    return next_step
  end
})
M.run_all = function()
  local i = 1
  while i <= #TASKS do
    local task_state = TASKS[i]()
    if not task_state then
      for _, f in ipairs(TASKS[i].completion_listeners) do
        f()
      end
      table.remove(TASKS, i)
    else
      i = i + 1
    end
  end
end
return M
