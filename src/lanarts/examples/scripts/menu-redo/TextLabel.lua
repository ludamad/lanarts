require "utils" -- import utils for 'do_nothing'

TextLabel = newtype()

function TextLabel:init(font, options, text)
    self.font = font
    self.options = options
    self.text = text
end

TextLabel.step = do_nothing

function TextLabel:draw(xy)
    self.font:draw( self.options, xy, self.text )
end