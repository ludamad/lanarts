local InstanceBox = import "core.ui.InstanceBox"
local GameState = import "core.GameState"
local Display = import "core.Display"
local Mouse = import "core.Mouse"

local M = nilprotect {} -- Submodule

local ActionBar = newtype()

function ActionBar:init(obj) self.obj = obj end
function ActionBar:draw(xy)
    self.obj.action_resolver:actionbar_draw(self.obj, xy)    
end
function ActionBar.get:size()
    return self.obj.action_resolver:actionbar_size()
end

local fps_timer = timer_create()
local fps_count, fps = 1, nil

local function fps_draw()
    fps_count = fps_count + 1

    if fps then
        local w,h = unpack( Display.display_size ) 
        Fonts.small:draw( {origin=Display.RIGHT_BOTTOM}, {w, h}, "FPS: " .. math.floor(fps) )
    end

    local ms = fps_timer:get_milliseconds()
    if ms > 1000 then
        fps = fps_count / ms * 1000
        fps_timer:start()
        fps_count = 0
    end
end

local ORIGIN = {0,0}
function M.create(size, focus_object)
    local window = InstanceBox.create { size = size }
    function window:draw()
        InstanceBox.draw(self, ORIGIN)
        fps_draw()
    end

    local action_bar = ActionBar.create(focus_object)
    window:add_instance(action_bar, Display.LEFT_BOTTOM)

    return window
end

return M
