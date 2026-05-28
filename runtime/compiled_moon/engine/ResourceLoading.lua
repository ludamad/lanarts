local Tasks = require("networking.Tasks")
local RESOURCE_TASK = Tasks.create(function()
  return with_mutable_globals(function()
    return Engine.resources_load()
  end)
end)
RESOURCE_TASK.duration = 1
local ensure_resources_before
ensure_resources_before = function(f)
  if RESOURCE_TASK.completed then
    return f()
  end
  local text = "Loading game..."
  local timer = timer_create()
  local loading_screen
  loading_screen = function()
    local percentage = math.floor(timer:get_milliseconds() / 10 * 100)
    require("menus.LoadingScreen").draw(math.min(100, percentage), text)
    if RESOURCE_TASK.completed then
      return f()
    end
    return loading_screen
  end
  return loading_screen()
end
return {
  ensure_resources_before = ensure_resources_before,
  start = function()
    return RESOURCE_TASK:start()
  end
}
