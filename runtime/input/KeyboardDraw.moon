-- Sidebar handling --
Display = require 'core.Display'
spell_keys = {'y', 'u', 'i', 'o', 'p'}

return {
    _draw_button: (button, xy) =>
        -- sprite = tosprite("spr_gamepad.Keyboard_Black_#{button}")
        xy = vector_add(xy, {16, 16})
        Fonts.large\draw {color: {255,255,50, 100}, origin: Display.CENTER}, xy, button\upper()
    draw_item_ui_hint: (xy, slot) =>
        --if not @_should_use_item_bindings
        --    return
        --for s, b in ipairs @item_slot_bindings
        --    if slot == s
        --        @_draw_button(b, xy)
        --return nil
    draw_spell_ui_hint: (xy, slot) =>
        for s, b in ipairs spell_keys
            if slot == s
                @_draw_button(b, xy)
        return nil
    draw_action_bar_weapon_hint: (xy) =>
        above_xy = {xy[1], xy[2] - 32}
        @_draw_button('h', above_xy)
    draw_action_bar_spell_hint: (xy, slot) =>
        for s, b in ipairs spell_keys
            if slot == s - 1
                above_xy = {xy[1], xy[2] - 32}
                @_draw_button(b, above_xy)
        return nil
}

