--------------------------------------------------------------------------------
-- Finds the standard location of global variables within all the packages.
--------------------------------------------------------------------------------
local error,setmetatable=error,setmetatable

return function(--[[Optional]] dont_load_draw_globals) 
    local nilprotect_meta = {__index = function(self, k)
        error( ("Key '%s' does not exist in table!"):format(k) )
    end}    
    -- Set to a metatable that does not allow nil accesses
    function nilprotect(t)
        return setmetatable(t, nilprotect_meta)
    end

    -- Initial global protection phase, only protect against undefined lookups:
    setmetatable(_G, {__index = function(self, k)
        error( ("Global variable '%s' does not exist!"):format(k) )
    end})

    require "Logging"

    log_verbose "Requiring box2d while global access is allowed."
    require "b2"

    log "Read core globals."
    require "globals.CoreGlobals"
    require "globals.StringUtils"

    log "Read modules that must be initialized."
    require "ErrorReporting"

    if not dont_load_draw_globals then
        log "Read globals that involve core.Display or similar"
        require "globals.Draw"
        if package.loaded["core.GameState"] then
            require "globals.GameUtils"
        end
        require "globals.TextComponent"
    end

    log "Read other globals."
    require "globals.Debug"
    require "globals.FileUtils"
    require "globals.LuaJITReplacements"
    require "globals.Math"
    require "globals.TableUtils"

    log "Preload external packages that set globals:"

    require "json"
    require "socket"
    require "ltn12"
    require "mime"

    log "Preload legacy internal modules that set globals (and need refactoring to not):"

    require "effects.Effects"

    log "Assigning, in effect declaring, global variables used by the engine."
    items = false
    spells = false
    effects = false
    enemies = false
    sprites = false
    projectiles = false
    classes = false
    player = false

    -- TODO get rid of these:
    enemy_init = do_nothing
    enemy_step = do_nothing
    enemy_berserker_step = do_nothing
    enemy_berserker_init = do_nothing

    argv_configuration = nilprotect {
        save_file = false,
        load_file = false,
    }

    -- Now that global variable mutation has been done, protect globals even more strongly, prevent creation of new globals:
    setmetatable(_G, {__index = function(self, k)
        error( ("Global variable '%s' does not exist!"):format(k) )
    end, __newindex = function(self, k)
        if k ~= 'player' and k ~= "start_lanarts" then  -- Hacks for engine
            error( ("Not in file called by GlobalVariableSetup, cannot set global variable '%s'!"):format(k) )
        end
    end})
end
