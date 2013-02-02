require "InstanceBox"
require "Sprite"
require "utils"

local TextButton = newtype()

function TextButton:init(text, action)
    self.font = font_cached_load(settings.menu_font, 20)

    self.size = self.font:draw_size(text)
    self.text = text

    self.action = action

    self.padding = 5
end

function TextButton:_click_area(xy) 
    return bbox_padded( xy, self.size, self.padding )
end

function TextButton:step(xy)
    if mouse_left_pressed and self.action ~= nil then
        if bbox_mouse_over( self:_click_area(xy)) then
            self.action()
        end
    end
end

function TextButton:draw(xy)
    local text_color = COL_WHITE

    if bbox_mouse_over( self:_click_area(xy) ) then 
        text_color = COL_RED
    end

    self.font:draw( {color = text_color }, xy, self.text )
end

-- MENUS -- 

function start_menu_create(on_start_click)
    local menu = InstanceBox.create( { size = display.window_size } )

    menu:add_instance(
        Sprite.image_create("res/interface/sprites/lanarts_logo.png"),
        CENTER_TOP,
        --[[Down 10 pixels]]
        {0, 10}
    )

    menu:add_instance(
        TextButton.create("Start a New Game", on_start_click),
        CENTER
    )

    return menu
end