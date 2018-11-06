M = {}

TASKS = {}

Task = newtype {

    init: (fn, duration, err_handler) =>
        @task_coroutine = coroutine.create(fn)
        @err_handler = err_handler or false
        @completed = false
        @duration = duration or false
        @completion_listeners = {}
        @next_ret_values = {}
        @started = false

    start: () =>
        if not @started
            step_task = () ->
                return @step(step_task)
            append TASKS, step_task

    step: (next_step) =>
        status, err = @resume()
        if not status
            if err and err ~= 'cannot resume dead coroutine'
                if @err_handler then
                    @err_handler()
                else
                    if type(err) ~= "string"
                        err = pretty_tostring(err)
                    print(err)
            @completed = true
            return nil
        return next_step
}

M.run_all = () ->
    while i <= #TASKS
        task_state,  = TASKS[i]()
        if not task_state then
            for _, f in ipairs(TASKS[i].completion_listeners) do
                f()
            table.remove(TASKS, i)
        else
            i = i + 1

return M
