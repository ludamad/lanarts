{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
Display = require "core.Display"
DataW = require "DataWrapped"

BONUSES = OrderedDict()
define_bonus = (bonus) ->
    BONUSES[bonus.name] = bonus

    if bonus.stat_func
        DataW.additive_effect_create {
            name: "#{bonus.name}Effect"
            key: "amount" -- Additive effect, accessed with @_get_value().
            stat_func: (obj, old, new) =>
                bonus\stat_func(@_get_value(), obj, old, new)
        }
        bonus.effects_granted = {{"#{bonus.name}Effect", {amount: 1}}}

BONUS_LOCS = {
    {0,16}
    {16,16}
    {0,0}
    {16,0}
}

compile_bonuses = (base, bonuses) ->
    entry = table.clone(base)
    adjective = table.concat bonuses, " "
    bonus_objs = (for bonus in *bonuses do BONUSES[bonus])

    -- Call 'apply' on each bonus object
    for bonus in *bonus_objs
        if bonus.apply
            bonus\apply(entry)

    -- Compute name
    entry.name =  "#{adjective} #{entry.name}"

    -- Draw console description
    entry.console_draw_func = (inst, get_next) =>
        for bonus in *bonus_objs
            if bonus.console_draw_func
                bonus\console_draw_func(inst, get_next)

    -- Calculate auto_equip status
    if entry.auto_equip == nil
        entry.auto_equip = true
    for {:auto_equip} in *bonus_objs
        if auto_equip == false
            entry.auto_equip = false

    -- Draw item minisprite overlays
    entry.item_draw_func = (options, x, y) =>
        options = {color: options.color, scale: {0.5, 0.5}}
        for i, bonus in ipairs bonus_objs
            if bonus.item_draw_func and BONUS_LOCS[i]
                {bx, by} = BONUS_LOCS[i]
                bonus\item_draw_func(options, x + bx, y + by)

    entry.effects_granted or= {}
    for {:effects_granted} in *bonus_objs
        if effects_granted
            entry.effects_granted = table.tconcat(entry.effects_granted, effects_granted)
    return entry

return {:define_bonus, :compile_bonuses, :BONUSES}
