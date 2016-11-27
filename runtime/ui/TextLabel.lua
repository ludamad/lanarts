--- A static, drawable text instance. Implements everything 
-- needed to be added to an InstanceGroup, InstanceBox or InstanceLine.
local TextLabel = newtype( )

--- Takes either (font, text) or (font, options, text) and creates a
-- TextLabel. 
--
-- In addition to the usual font:draw options, one can specify
-- 'max_width' in the options table. If this is specified, the 
-- font object is drawn using draw_wrapped.
function TextLabel:init(font, options_or_text, text)
    self.font = font
    if text then
        self.options, self.text = options_or_text, text
    else 
        self.options, self.text = {}, options_or_text
    end
    self.wrapped = (self.options.max_width ~= nil)
    self.max_width = self.options.max_width or -1
end

TextLabel.step = do_nothing

function TextLabel:__tostring()
    return "[TextLabel " .. toaddress(self) .. "]" 
end

--- Getter for dynamically created size.
-- A size member is needed for InstanceGroup & InstanceBox.
function TextLabel.get:size()
    return  self.wrapped and 
                self.font:draw_size(self.text, self.max_width) 
                 or self.font:draw_size(self.text)
end

--- Convenience function for checking if the mouse is within
-- the rectangular area around the TextLabel
function TextLabel:mouse_over(xy)
    return mouse_over(xy, self.size, self.options.origin)
end

function TextLabel:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
    if self.wrapped then
        self.font:draw_wrapped( self.options, xy, self.max_width, self.text )
    else
        self.font:draw( self.options, xy, self.text )
    end
end

return TextLabel
