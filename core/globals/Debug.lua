-- TODO: Figure out what it would take to have this to work for, say, people who aren't me.
function debug.attach_debugger()
    local DBG_PATH, DBG_MODULE = "/usr/share/lua/5.1/", "debugger"
    local DBG_FILE = DBG_PATH .. DBG_MODULE .. ".lua"
    if not file_exists(DBG_FILE) then
        error("Debugging requires " .. DBG_FILE .. " to exist!")
    end
    require_path_add(DBG_PATH .. "?.lua")
    require(DBG_MODULE)()
    debug.attach_debugger = do_nothing
end