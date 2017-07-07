Gamepad = require "core.Gamepad"
GameActions = require "core.GameActions"
GameState = require "core.GameState"

-- Make diagonals as fast in each direction as non-diagonals:
lanarts_skew = (dir) ->
    {x, y} = dir
    mx = math.max(math.abs(x), math.abs(y))
    mag = math.min(1, math.sqrt(x*x + y*y))
    if mx == 0
        mx = 1
    x, y = x / mx * mag, y / mx * mag
    x = math.min(math.max(-1, x), 1)
    y = math.min(math.max(-1, y), 1)
    return {x, y}

-- Private
ButtonState = newtype {
    init: (@id) =>
        @pressed, @held = {}, {}
    step: () =>
        @pressed.button_up_dpad = not @held.button_up_dpad and Gamepad.button_up_dpad(@id)
        @pressed.button_down_dpad = not @held.button_down_dpad and Gamepad.button_down_dpad(@id)
        @pressed.button_right_dpad = not @held.button_right_dpad and Gamepad.button_right_dpad(@id)
        @pressed.button_left_dpad = not @held.button_left_dpad and Gamepad.button_left_dpad(@id)

        @pressed.axis_left_trigger = not @held.axis_left_trigger and Gamepad.axis_left_trigger(@id) > 0.2
        @pressed.axis_right_trigger = not @held.axis_right_trigger and Gamepad.axis_right_trigger(@id) > 0.2

        @pressed.button_right_shoulder = not @held.button_right_shoulder and Gamepad.button_right_shoulder(@id)
        @pressed.button_left_shoulder = not @held.button_left_shoulder and Gamepad.button_left_shoulder(@id)
        @pressed.button_x = not @held.button_x and Gamepad.button_x(@id)
        @pressed.button_y = not @held.button_y and Gamepad.button_y(@id)
        @pressed.button_a = not @held.button_a and Gamepad.button_a(@id)
        @pressed.button_b = not @held.button_b and Gamepad.button_b(@id)

        -- remember previous state
        @held.button_x = Gamepad.button_x(@id)
        @held.button_y = Gamepad.button_y(@id)
        @held.button_a = Gamepad.button_a(@id)
        @held.button_b = Gamepad.button_b(@id)

        @held.button_up_dpad = Gamepad.button_up_dpad(@id)
        @held.button_down_dpad = Gamepad.button_down_dpad(@id)
        @held.button_right_dpad = Gamepad.button_right_dpad(@id)
        @held.button_left_dpad = Gamepad.button_left_dpad(@id)
        
        @held.axis_left_trigger = Gamepad.axis_left_trigger(@id) > 0.2
        @held.axis_right_trigger = Gamepad.axis_right_trigger(@id) > 0.2
        
        @held.button_left_shoulder = Gamepad.button_left_shoulder(@id)
        @held.button_right_shoulder = Gamepad.button_right_shoulder(@id)
    menu_mode: () =>
        return @held.button_left_shoulder
    is_held: (s) => return @held[s]
    is_pressed: (s) => return @pressed[s]
}

Spells = newtype {
    init: (@player, @id) =>
        @bindings = {
            {"button_x", 0}
            {"button_y", 1}
            {"button_b", 2}
            {"button_a", 3}
        }
        @spells = {}
    step: (focused, buttons) =>
        @spells = @player\effective_stats().spells
        if focused and buttons\menu_mode() 
            for binding in *@bindings
                if buttons\is_held binding[1]
                    return 
}

-- Public
GamepadInputSource = newtype {
    init: (player, id) =>
        @player = assert player
        @id = id or tonumber(assert Gamepad.ids()[1])
        @highlighted_xy = {0, 0}
        @selected_xy = {-1, 0}
        @menu_mode = false
        @prev_state, @new_state = {}, {}
        @spell_state = Spells.create(player, @id)
        @item_state = Items.create(player, @id)
        @button_state = ButtonState.create(@id)

        @item_slot_bindings = {
            {"button_x", 0}
            {"button_y", 1}
            {"button_b", 2}
            {"button_a", 3}
        }
    poll_input: () =>
        @button_state\step()
        @spell_state\step @button_state
        @item_state\step @button_state
        @_handle_grid_menu()

    sidebar_draw: () =>
        return false

    _handle_grid_menu: () =>
        inv_w, inv_h = 5, 8
        {:pressed} = @button_state.pressed
        if pressed.button_up_dpad
            @highlighted_xy[2] = (@highlighted_xy[2] - 1) % inv_h
        if pressed.button_down_dpad
            @highlighted_xy[2] = (@highlighted_xy[2] + 1) % inv_h
        if pressed.button_left_dpad
            @highlighted_xy[1] = (@highlighted_xy[1] - 1) % inv_w
        if pressed.button_right_dpad
            @highlighted_xy[1] = (@highlighted_xy[1] + 1) % inv_w

}

return GamepadInputSource
