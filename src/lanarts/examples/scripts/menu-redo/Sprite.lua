-- Stores options to be passed to a sprite
-- Keeps track of frames

require "utils" -- import utils for 'do_nothing'

Sprite = newtype()

function Sprite:init(sprite, options)
    self.sprite = sprite
    self.options = options
    if self.options.frame == nil then
       self.options.frame = 1
    end
end

function Sprite.get:size()
    return self.sprite.size
end

function Sprite:mouse_over(xy)
    return mouse_over(xy, self.size, self.options.origin)
end

function Sprite:step(xy)
    self.options.frame = self.options.frame + 1
end

function Sprite:draw(xy)
    self.sprite:draw( self.options, xy)
end

function Sprite:__tostring()
    return "[Sprite " .. toaddress(self) .. "]" 
end