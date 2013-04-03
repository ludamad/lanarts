require "utils"
require "InstanceBox"
require "InstanceLine"
require "TextInputBox"
require "TextLabel"
require "Sprite"

DEBUG_LAYOUTS = false

-- Need to expose inventory to Lua ??

function inventory_view_create(size)
    local box = InstanceBox.create( { size = size } )
    function box:draw(xy) -- makeshift inheritance
        local bbox = bbox_create(xy, self.size)
        draw_rectangle_outline(COL_WHITE, bbox, 1)
    end 
    return box
end

function main()
    local size = {160, 300}
    local inv = inventory_view_create(size)

    while Game.input_capture() and not key_pressed(keys.ESCAPE) do
        if key_pressed(keys.F9) then
            -- note, globals are usually protected against being changed
            -- but a bypass is allowed for cases where it must be done
            setglobal("DEBUG_LAYOUTS", not DEBUG_LAYOUTS) -- flip on/off
        end

        Display.draw_start()
        inv:draw( {100, 100} )
        Display.draw_finish()

        io.flush()
        Game.wait(10)
    end

    return false -- User has quit the game
end