require "utils" -- import utils for 'do_nothing'

TextLabel = newtype( )

function TextLabel:init(font, arg1, arg2)
    self.font = font
    if arg2 then
        self.options, self.text = arg1, arg2
    else 
        self.options, self.text = {}, arg1
    end
end

TextLabel.step = do_nothing

function TextLabel:__tostring()
    return "[TextLabel " .. toaddress(self) .. "]" 
end

function TextLabel.get:size()
    return self.font:draw_size(self.text)
end

function TextLabel:mouse_over(xy)
    return mouse_over(xy, self.size, self.options.origin)
end

function TextLabel:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
    self.font:draw( self.options, xy, self.text )
end