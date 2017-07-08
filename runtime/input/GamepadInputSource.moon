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
IOState = newtype {
    init: (@id) =>
        @pressed, @held = {}, {}
        @highlighted_xy = {0,0}
        @left_axis_dir = {0,0}
        @right_axis_dir = {0,0}
    step: () =>
        @pressed.button_up_dpad = not @held.button_up_dpad and Gamepad.button_up_dpad(@id)
        @pressed.button_down_dpad = not @held.button_down_dpad and Gamepad.button_down_dpad(@id)
        @pressed.button_right_dpad = not @held.button_right_dpad and Gamepad.button_right_dpad(@id)
        @pressed.button_left_dpad = not @held.button_left_dpad and Gamepad.button_left_dpad(@id)
        @pressed.button_right_stick = not @held.button_right_stick and Gamepad.button_right_stick(@id)
        @pressed.button_left_stick = not @held.button_left_stick and Gamepad.button_left_stick(@id)

        @pressed.axis_left_trigger = not @held.axis_left_trigger and Gamepad.axis_left_trigger(@id) > 0.7
        @pressed.axis_right_trigger = not @held.axis_right_trigger and Gamepad.axis_right_trigger(@id) > 0.7

        @pressed.button_right_shoulder = not @held.button_right_shoulder and Gamepad.button_right_shoulder(@id)
        @pressed.button_left_shoulder = not @held.button_left_shoulder and Gamepad.button_left_shoulder(@id)
        @pressed.button_x = not @held.button_x and Gamepad.button_x(@id)
        @pressed.button_y = not @held.button_y and Gamepad.button_y(@id)
        @pressed.button_a = not @held.button_a and Gamepad.button_a(@id)
        @pressed.button_b = not @held.button_b and Gamepad.button_b(@id)
        @pressed.button_back = not @held.button_back and Gamepad.button_back(@id)
        @pressed.button_start = not @held.button_start and Gamepad.button_start(@id)

        -- remember previous state
        @held.button_x = Gamepad.button_x(@id)
        @held.button_y = Gamepad.button_y(@id)
        @held.button_a = Gamepad.button_a(@id)
        @held.button_b = Gamepad.button_b(@id)

        @held.button_up_dpad = Gamepad.button_up_dpad(@id)
        @held.button_down_dpad = Gamepad.button_down_dpad(@id)
        @held.button_right_dpad = Gamepad.button_right_dpad(@id)
        @held.button_left_dpad = Gamepad.button_left_dpad(@id)
        @held.button_right_stick = Gamepad.button_right_stick(@id)
        @held.button_left_stick = Gamepad.button_left_stick(@id)
        
        @held.axis_left_trigger = Gamepad.axis_left_trigger(@id) > 0.7
        @held.axis_right_trigger = Gamepad.axis_right_trigger(@id) > 0.7
        
        @held.button_left_shoulder = Gamepad.button_left_shoulder(@id)
        @held.button_right_shoulder = Gamepad.button_right_shoulder(@id)
        @held.button_back = Gamepad.button_back(@id)
        @held.button_start = Gamepad.button_start(@id)

        @left_axis_dir = {Gamepad.axis_left_x(@id), Gamepad.axis_left_y(@id)}
        for i=1,2
            if math.abs(@left_axis_dir[i]) < 0.2
                @left_axis_dir[i] = 0
        @right_axis_dir = {Gamepad.axis_right_x(@id), Gamepad.axis_right_y(@id)}
        for i=1,2
            if math.abs(@right_axis_dir[i]) < 0.2
                @right_axis_dir[i] = 0
        -- Lanarts sauce -- because we're evil, make 1 in both directions consider 1 total distance
        @left_axis_dir = lanarts_skew @left_axis_dir
        @right_axis_dir = lanarts_skew @right_axis_dir
        
        @pressed.right_axis_down = not @held.right_axis_down and (Gamepad.axis_right_y(@id) > 0.2)
        @held.right_axis_down = (Gamepad.axis_right_y(@id) > 0.2)
        @pressed.right_axis_up = not @held.right_axis_up and (Gamepad.axis_right_y(@id) < -0.2)
        @held.right_axis_up = (Gamepad.axis_right_y(@id) < -0.2)
        @pressed.right_axis_right = not @held.right_axis_right and (Gamepad.axis_right_x(@id) > 0.2)
        @held.right_axis_right = (Gamepad.axis_right_x(@id) > 0.2)
        @pressed.right_axis_left = not @held.right_axis_left and (Gamepad.axis_right_x(@id) < -0.2)
        @held.right_axis_left = (Gamepad.axis_right_x(@id) < -0.2)
        
    is_held: (s) => return @held[s] or false
    is_pressed: (s) => return @pressed[s] or false
}

make_bindings = () =>
    return {
        {"button_x", 0}
        {"button_y", 1}
        {"button_b", 2}
        {"button_a", 3}
        {"button_start", -1}
        {"button_back", -1}
        {"button_guide", -1}
        {"button_right_shoulder", -1}
        {"right_axis_down", -1}
        {"right_axis_left", -1}
        {"right_axis_up", -1}
        {"right_axis_right", -1}
    }

BINDABLE_BUTTONS = {
    "button_x"
    "button_y"
    "button_b"
    "button_a"
    "button_start"
    "button_back"
    "button_guide"
    "button_right_shoulder"
    "right_axis_down"
    "right_axis_left"
    "right_axis_up"
    "right_axis_right"
}

-- Public
GamepadInputSource = newtype {
    init: (player, id) =>
        @player = assert player
        @id = id or tonumber(assert Gamepad.ids()[1])
        @menu_mode = false
        @prevs, @news = {}, {}
        @iostate = IOState.create(@id)
        @item_selected_xy = {-1, 0}
        @item_slot_bindings = make_bindings()
        @spell_slot_bindings = make_bindings()
}
_extend = (extension) ->
    for k,v in pairs extension
        GamepadInputSource[k] = v

-- Sidebar handling --
_extend {
    _handle_grid_menu: () =>
        inv_w, inv_h = 5, 8
        {:pressed} = @iostate
        if pressed.button_up_dpad
            @iostate.highlighted_xy[2] = (@iostate.highlighted_xy[2] - 1) % inv_h
        if pressed.button_down_dpad
            @iostate.highlighted_xy[2] = (@iostate.highlighted_xy[2] + 1) % inv_h
        if pressed.button_left_dpad
            @iostate.highlighted_xy[1] = (@iostate.highlighted_xy[1] - 1) % inv_w
        if pressed.button_right_dpad
            @iostate.highlighted_xy[1] = (@iostate.highlighted_xy[1] + 1) % inv_w
    handle_sidebar: (switch_menu_left, switch_menu_right) =>
        if @iostate\is_pressed("button_start")
            switch_menu_right()
    handle_inventory: (drop_item, reposition_item) =>
        @_handle_grid_menu()
        if @_should_macro
            for button in *BINDABLE_BUTTONS 
                if @iostate\is_held(button)
                    @_bind(@item_slot_bindings, button, @slot_highlighted())
        if @_should_select_item
            if @slot_selected() ~= -1
                if @slot_highlighted() == @slot_selected() 
                    drop_item(@slot_selected(), @_should_halve)
                else
                    reposition_item(@slot_selected(), @slot_highlighted())
                @item_selected_xy = {-1, 0}
            else
                @item_selected_xy = {@iostate.highlighted_xy[1], @iostate.highlighted_xy[2]}
    handle_store: (try_buy_item) => 
        @_handle_grid_menu()
        if @_should_menu_confirm
            if @slot_highlighted() ~= -1
                try_buy_item(@slot_highlighted())
    _bind: (bindings, button, slot) =>
        for binding in *bindings
            {b, s} = binding
            if button == b
                binding[2] = slot
            elseif s == slot
                binding[2] = -1 -- Kick it off the slot
    handle_spells: () =>
        @_handle_grid_menu()
        if @_should_macro
            for button in *BINDABLE_BUTTONS 
                if @iostate\is_held(button)
                    @_bind(@spell_slot_bindings, button, @slot_highlighted())
    handle_enemy_info: () =>
        @_handle_grid_menu()
    slot_highlighted: () => @iostate.highlighted_xy[1] + @iostate.highlighted_xy[2] * 5
    slot_selected: () => 
        @item_selected_xy[1] + @item_selected_xy[2] * 5
    _draw_button: (button, xy) =>
        sprite = tosprite("spr_gamepad.xbox_#{button}")
        sprite\draw {color: {255,255,255, 100}}, xy
    draw_item_ui_hint: (xy, slot) =>
        if not @_should_use_item_bindings
            return
        for {b, s} in *@item_slot_bindings
            if slot == s
                @_draw_button(b, xy)
        return nil
    draw_spell_ui_hint: (xy, slot) =>
        if @_should_use_item_bindings
            return
        for {b, s} in *@spell_slot_bindings
            if slot == s
                @_draw_button(b, xy)
        return nil
    draw_action_bar_weapon_hint: (xy) =>
        above_xy = {xy[1], xy[2] - 32}
        tosprite("spr_gamepad.xbox_right_shoulder")\draw({color: {255,255,255, 100}}, above_xy)
    draw_action_bar_spell_hint: (xy, slot) =>
        for {b, s} in *@spell_slot_bindings
            if slot == s
                above_xy = {xy[1], xy[2] - 32}
                @_draw_button(b, above_xy)
        return nil
}

-- Core IO
_extend {
    poll_input: () =>
        @iostate\step()
        @_should_menu_confirm = @iostate\is_pressed("button_right_stick")
        @_should_select_item = @iostate\is_pressed "axis_right_trigger"
        @_should_halve = @iostate\is_held "axis_left_trigger"
        @_should_use_item_bindings = @iostate\is_held("button_left_shoulder") and not @_should_halve
        @_should_macro = @iostate\is_held("button_left_shoulder") and @_should_halve
        @_should_sell = not @_should_macro and @iostate\is_held("button_back")
    sell_mode: () => @_should_sell
    move_direction: () => @iostate.left_axis_dir
    target_position: () => 
        if @iostate.right_axis_dir[1] == 0 and @iostate.right_axis_dir[2] == 0
            return {0,0}
        return vector_add @player.xy, vector_scale(@iostate.right_axis_dir, 32)
    should_explore: () => @iostate\is_held "button_left_stick"

    use_spell_slot: () =>
        if @_should_use_item_bindings 
            return -1
        for binding in *@spell_slot_bindings
            {button, slot} = binding
            if @iostate\is_held(button)
                return slot
        return -1
    _item_slot: () =>
        if @_should_menu_confirm
            return @slot_highlighted()
        if @_should_use_item_bindings
            for binding in *@item_slot_bindings
                {button, slot} = binding
                if @iostate\is_pressed(button)
                    return slot
        return -1
    sell_item_slot: () =>
        if not @sell_mode()
            return -1
        return @_item_slot()
    use_item_slot: () =>
        if @sell_mode() or @_should_macro
            return -1
        return @_item_slot()
    should_shift_autotarget: () => false
    should_use_weapon: () => @iostate\is_held "button_right_shoulder"
    should_exit_game: () =>
        error "TODO"
}

return GamepadInputSource
