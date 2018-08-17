{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
Display = require "core.Display"
DataW = require "DataWrapped"

BONUSES = OrderedDict()
define_bonus = (bonus) ->
    BONUSES[bonus.name] = bonus

    if bonus.effect
        -- Use table to define an effect:
        DataW.effect_create table.merge bonus.effect, {
            name: "#{bonus.name}Bonus"
        }
        bonus.effects_granted = {"#{bonus.name}Bonus"}
    elseif bonus.additive_effect
        -- Use table to define an effect:
        DataW.additive_effect_create table.merge bonus.additive_effect, {
            name: "#{bonus.name}Bonus"
            key: "amount" -- Additive effect, accessed with @value().
        }
        bonus.effects_granted = {{"#{bonus.name}Bonus", {amount: bonus.effect_amount or 1}}}

ONE_OFF_LOCS = {
    {16,16}
}

BONUS_LOCS = {
    {0,16}
    {16,16}
    {0,0}
    {16,0}
}

DEFAULT_STAT_BONUSES = {
    mp: 0
    hp: 0
    hpregen: 0
    mpregen: 0
    strength: 0
    defence: 0
    willpower: 0
    magic: 0

    -- Spell-affecting stat bonuses:
    spell_velocity_multiplier: 1.0
    spell_cooldown_multiplier: 1.0
    melee_cooldown_multiplier: 1.0
    ranged_cooldown_multiplier: 1.0
}

compile_bonuses = (base, bonuses) ->
    entry = table.clone(base)
    adjective = table.concat bonuses, " "
    bonus_objs = (for bonus in *bonuses do BONUSES[bonus])

    -- Call 'apply' on each bonus object
    for bonus in *bonus_objs
        if bonus.apply
            bonus\apply(entry)

    for {:shop_cost} in *bonus_objs
        if shop_cost
            entry.shop_cost = vector_add(entry.shop_cost, shop_cost)
    entry.shop_cost = vector_scale(entry.shop_cost, math.max(1, #bonuses))

    -- Compute name
    entry.name =  "#{adjective} #{entry.name}"

    -- Draw console description
    entry.console_draw_func = (obj, get_next) =>
        for bonus in *bonus_objs
            if bonus.console_draw_func
                bonus\console_draw_func(obj, get_next)

    -- Calculate auto_equip status
    if entry.auto_equip == nil
        entry.auto_equip = true
    for {:auto_equip} in *bonus_objs
        if auto_equip == false
            entry.auto_equip = false

    -- Incorporate stat bonuses
    entry.stat_bonuses = table.merge DEFAULT_STAT_BONUSES, (entry.stat_bonuses or {})
    for {:stat_bonuses} in *bonus_objs
        for k,v in pairs(stat_bonuses or {})
            if k\endswith("multiplier")
                entry.stat_bonuses[k] *= v
            else
                entry.stat_bonuses[k] += v

    -- Draw item minisprite overlays
    entry.item_draw_func = (options, x, y) =>
        options = {color: options.color, scale: {0.5, 0.5}}
        locs = BONUS_LOCS
        if #bonus_objs == 1
            locs = ONE_OFF_LOCS
        for i, bonus in ipairs bonus_objs
            if bonus.item_draw_func and locs[i]
                {bx, by} = locs[i]
                bonus\item_draw_func(options, x + bx, y + by)

    entry.effects_granted or= {}
    for {:effects_granted} in *bonus_objs
        if effects_granted
            entry.effects_granted = table.tconcat(entry.effects_granted, effects_granted)
    return entry

return {:define_bonus, :compile_bonuses, :BONUSES}
