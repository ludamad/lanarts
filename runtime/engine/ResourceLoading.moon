Tasks = require "networking.Tasks"
RESOURCE_TASK = Tasks.create () ->
    with_mutable_globals () ->
        Engine.resources_load()
RESOURCE_TASK.duration = 1

-- State function
ensure_resources_before = (f) ->
    -- Resources have been loaded; proceed to our provided flow
    if RESOURCE_TASK.completed
        return f()

    text = "Loading game..."
    timer = timer_create()
    loading_screen = () ->
        percentage = math.floor(timer\get_milliseconds() / 10 * 100)
        require("menus.LoadingScreen").draw(math.min(100, percentage), text)
        -- Continue to our provided flow once resources are loaded
        if RESOURCE_TASK.completed
            return f()
        return loading_screen
    return loading_screen()

return {:ensure_resources_before, start: () -> RESOURCE_TASK\start()}
