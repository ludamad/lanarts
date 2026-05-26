local ProgrammableInputSource = require "input.ProgrammableInputSource"
local GameObject = require "core.GameObject"
local Map = require "core.Map"
local ExploreUtils = require "tests.ExploreUtils"

local DEFAULTS = {
    seed = 12345678,
    spell_policy = "mixed",
    spell_retry_delay = 18,
    emergency_hp_pct = 0.35,
    ranged_emergency_hp_pct = 0.60,
    emergency_mp_pct = 0.20,
    regen_hp_pct = 0.65,
    retreat_hp_pct = 0.28,
    necromancer_regen_hp_pct = 0.60,
    necromancer_retreat_hp_pct = 0.22,
    necromancer_pain_range = 72,
    pyrocaster_ring_min_range = 30,
    pyrocaster_ring_max_range = 62,
    ranged_keepaway = 80,
    melee_close = 46,
    stuck_end_frames = 720,
}

local CLASS_SPELL_PRIORITIES = {
    Fighter = {"Berserk", "Power Strike", "Dash Attack"},
    Templar = {"Power Strike", "Dash Attack"},
    Rogue = {"Dash Attack"},
    Necromancer = {"Baleful Regeneration", "Summon Dark Aspect", "Skeleton Army", "Link of Loyalty", "Unlink"},
    Pyrocaster = {"Dragonform", "Ring of Flames", "Fire Bolt"},
    Stormcaller = {"Energy Spear", "Chain Lightning", "Tornado Storm"},
    ["Blue Mage"] = {"Water Bolt"},
}

local RANGED_CLASSES = {
    Pyrocaster = true,
    Stormcaller = true,
    ["Blue Mage"] = true,
}

local CLASS_KEEPAWAY = {
    Pyrocaster = 112,
    Stormcaller = 112,
    ["Blue Mage"] = 96,
}

local SPELL_RETRY_DELAYS = {
    ["Dragonform"] = 650,
    ["Ring of Flames"] = 520,
    ["Fire Bolt"] = 35,
    ["Energy Spear"] = 35,
    ["Chain Lightning"] = 65,
    ["Tornado Storm"] = 300,
}

local AutomatedPlayerInputSource = newtype({parent = ProgrammableInputSource})

local function merge_defaults(opts)
    local result = {}
    for k, v in pairs(DEFAULTS) do
        result[k] = v
    end
    for k, v in pairs(opts or {}) do
        result[k] = v
    end
    return result
end

local function increment(tbl, key, amount)
    if not tbl then
        return
    end
    tbl[key] = (tbl[key] or 0) + (amount or 1)
end

local function distance_sqr(a, b)
    local dx, dy = a.x - b.x, a.y - b.y
    return dx * dx + dy * dy
end

local function direction_between(from_obj, to_obj, deadzone)
    local dx, dy = to_obj.x - from_obj.x, to_obj.y - from_obj.y
    deadzone = deadzone or 0
    if math.abs(dx) < deadzone then
        dx = 0
    end
    if math.abs(dy) < deadzone then
        dy = 0
    end
    if dx ~= 0 then
        dx = dx / math.abs(dx)
    end
    if dy ~= 0 then
        dy = dy / math.abs(dy)
    end
    return {dx, dy}
end

local function reverse_direction(dir)
    return {-dir[1], -dir[2]}
end

local function hp_pct(player)
    local max_hp = math.max(1, player:effective_stats().max_hp)
    return player.stats.hp / max_hp
end

local function mp_pct(player)
    local max_mp = math.max(1, player:effective_stats().max_mp)
    return player.stats.mp / max_mp
end

local function active_summon_count(player)
    local effect = player:get_effect("Summoner")
    return effect and (effect.n_summons or 0) or 0
end

local function max_dark_aspects(player)
    return math.max(1, math.ceil((player:effective_stats().willpower - 4) / 2))
end

local function known_spells(player)
    local result = {}
    for _, entry in ipairs(player.def.available_spells or {}) do
        if (entry.level_needed or 1) <= player.stats.level then
            result[#result + 1] = {
                slot = #result,
                name = entry.spell,
                def = entry,
            }
        end
    end
    return result
end

local function find_spell(player, name)
    for _, spell in ipairs(known_spells(player)) do
        if spell.name == name then
            return spell
        end
    end
    return nil
end

local function visible_enemies(player)
    local result = {}
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if not enemy.destroyed and Map.object_visible(enemy, enemy.xy, player) then
            result[#result + 1] = enemy
        end
    end
    table.sort(result, function(a, b)
        return distance_sqr(player, a) < distance_sqr(player, b)
    end)
    return result
end

local function inventory_slot(player, wanted)
    for _, entry in ipairs(player:inventory()) do
        if wanted[entry.item] then
            return entry.slot, entry.item
        end
    end
    return nil
end

local function nearest_alive_enemy(player)
    local best, best_dist
    for _, enemy in ipairs(Map.enemies_list(player)) do
        if not enemy.destroyed and enemy.is_enemy ~= false and enemy.team ~= player.team then
            local dist = distance_sqr(player, enemy)
            if not best_dist or dist < best_dist then
                best = enemy
                best_dist = dist
            end
        end
    end
    return best
end

local function is_on_portal(player)
    local collisions = Map.rectangle_collision_check(
        player.map,
        {player.x - 8, player.y - 8, player.x + 8, player.y + 8},
        player
    )
    for _, obj in ipairs(collisions) do
        if GameObject.get_type(obj) == "feature" and not obj.has_been_used then
            return true
        end
    end
    return false
end

function AutomatedPlayerInputSource:init(player, telemetry, opts)
    ProgrammableInputSource.init(self, player)
    self.telemetry = telemetry or {}
    self.options = merge_defaults(opts)
    self.ai_state = ExploreUtils.ai_state(player)
    self.rng = require("mtwist").create(self.options.seed)
    self.queued_movements = {}
    self.spell_retry_at = {}
    self.round_robin_index = 1
    self.last_x = player.x
    self.last_y = player.y
    self.same_square_frames = 0
    self.ticks = 0
end

function AutomatedPlayerInputSource:_record_counter(group, key, amount)
    local telemetry = self.telemetry
    telemetry[group] = telemetry[group] or {}
    increment(telemetry[group], key, amount)
end

function AutomatedPlayerInputSource:_record_scalar(key, amount)
    increment(self.telemetry, key, amount)
end

function AutomatedPlayerInputSource:_record_visible_enemies(enemies)
    for _, enemy in ipairs(enemies or {}) do
        self:_record_counter("visible_enemy_frames", enemy.name)
    end
end

function AutomatedPlayerInputSource:_set_move(dir)
    if dir and (dir[1] ~= 0 or dir[2] ~= 0) then
        self:set("move_direction", dir)
        return true
    end
    return false
end

function AutomatedPlayerInputSource:_spell_throttled(name)
    return (self.spell_retry_at[name] or 0) > self.ticks
end

function AutomatedPlayerInputSource:_order_spell(spell)
    self:set("use_spell_slot", spell.slot)
    self.spell_retry_at[spell.name] = self.ticks + (SPELL_RETRY_DELAYS[spell.name] or self.options.spell_retry_delay)
    self:_record_counter("spell_orders", spell.name)
    self:_record_counter("spell_slot_orders", tostring(spell.slot))
    return true
end

function AutomatedPlayerInputSource:_spell_reasonable(name, enemies)
    local player = self.player
    local pct = hp_pct(player)
    local closest = enemies[1]
    local closest_dist = closest and math.sqrt(distance_sqr(player, closest)) or math.huge

    if name == "Baleful Regeneration" then
        if player.class_name == "Necromancer" then
            return pct < self.options.necromancer_regen_hp_pct
                and not player:has_effect("Baleful Regeneration")
        end
        return pct < self.options.regen_hp_pct and not player:has_effect("Baleful Regeneration")
    elseif name == "Berserk" then
        return pct > self.options.retreat_hp_pct and not player:has_effect("Berserk")
    elseif name == "Dragonform" then
        return pct > self.options.retreat_hp_pct and not player:has_effect("Dragonform")
    elseif name == "Ring of Flames" then
        for _, enemy in ipairs(enemies) do
            if enemy.unique then
                return false
            end
        end
        return pct > self.options.ranged_emergency_hp_pct
            and #enemies <= 2
            and closest_dist <= self.options.pyrocaster_ring_max_range
            and not player:has_effect("Ring of Flames Stat Boost")
    elseif name == "Power Strike" then
        return closest_dist <= 96
    elseif name == "Dash Attack" then
        return closest_dist >= 28
    elseif name == "Summon Dark Aspect" then
        return pct > self.options.retreat_hp_pct
            and not player:has_effect("Summoning")
            and active_summon_count(player) < max_dark_aspects(player)
    elseif name == "Link of Loyalty" then
        return pct > self.options.retreat_hp_pct
            and not player:has_effect("Summoning")
            and active_summon_count(player) < 2
    elseif name == "Unlink" then
        return pct < self.options.emergency_hp_pct
    elseif name == "Skeleton Army" then
        return #enemies >= 2 and pct > self.options.retreat_hp_pct
    end
    return true
end

function AutomatedPlayerInputSource:_priority_spell(enemies)
    local priorities = CLASS_SPELL_PRIORITIES[self.player.class_name] or {}
    for _, name in ipairs(priorities) do
        local spell = find_spell(self.player, name)
        if spell and not self:_spell_throttled(name) and self:_spell_reasonable(name, enemies) then
            return spell
        end
    end
    return nil
end

function AutomatedPlayerInputSource:_round_robin_spell(enemies)
    local known = known_spells(self.player)
    if #known == 0 then
        return nil
    end
    for offset = 0, #known - 1 do
        local idx = ((self.round_robin_index + offset - 1) % #known) + 1
        local spell = known[idx]
        if not self:_spell_throttled(spell.name) and self:_spell_reasonable(spell.name, enemies) then
            self.round_robin_index = (idx % #known) + 1
            return spell
        end
    end
    return nil
end

function AutomatedPlayerInputSource:_choose_spell(enemies)
    if self.options.spell_policy == "none" then
        return nil
    end
    local can_use_spells = self.player:effective_stats().can_use_spells
    if can_use_spells == false or can_use_spells == 0 then
        return nil
    end
    local emergency_regen = find_spell(self.player, "Baleful Regeneration")
    if emergency_regen and not self:_spell_throttled(emergency_regen.name)
        and self:_spell_reasonable(emergency_regen.name, enemies) then
        return emergency_regen
    end
    if self.options.spell_policy == "round_robin" then
        return self:_round_robin_spell(enemies)
    end
    if self.options.spell_policy == "priority" then
        return self:_priority_spell(enemies)
    end
    return self:_priority_spell(enemies) or self:_round_robin_spell(enemies)
end

function AutomatedPlayerInputSource:_try_emergency_item(enemies)
    local player = self.player
    local hp_threshold = self.options.emergency_hp_pct
    if RANGED_CLASSES[player.class_name] then
        hp_threshold = self.options.ranged_emergency_hp_pct
    end
    if hp_pct(player) < hp_threshold then
        local slot, item = inventory_slot(player, {["Health Potion"] = true, Honeycomb = true})
        if slot then
            self:set("use_item_slot", slot)
            self:_record_counter("item_orders", item)
            return true
        end
    end
    if enemies and #enemies > 0 and mp_pct(player) < self.options.emergency_mp_pct then
        local slot, item = inventory_slot(player, {["Mana Potion"] = true})
        if slot then
            self:set("use_item_slot", slot)
            self:_record_counter("item_orders", item)
            return true
        end
    end
    return false
end

function AutomatedPlayerInputSource:_combat_move(enemies, ordered_spell)
    local closest = enemies[1]
    if not closest then
        return false
    end
    local player = self.player
    local dist = math.sqrt(distance_sqr(player, closest))
    local toward = direction_between(player, closest, 6)

    if hp_pct(player) < self.options.retreat_hp_pct then
        return self:_set_move(reverse_direction(toward))
    end
    local keepaway = CLASS_KEEPAWAY[player.class_name] or self.options.ranged_keepaway
    if RANGED_CLASSES[player.class_name] and dist < keepaway then
        return self:_set_move(reverse_direction(toward))
    end
    if not ordered_spell and dist > self.options.melee_close then
        return self:_set_move(toward)
    end
    return false
end

function AutomatedPlayerInputSource:_pyrocaster_ring_move(enemies, ordered_spell)
    local closest = enemies[1]
    if not closest then
        return false
    end
    local player = self.player
    local dist = math.sqrt(distance_sqr(player, closest))
    local toward = direction_between(player, closest, 6)
    local using_ring = player:has_effect("Ring of Flames Stat Boost")
        or (ordered_spell and ordered_spell.name == "Ring of Flames")

    if not using_ring or closest.unique then
        return self:_combat_move(enemies, ordered_spell ~= nil)
    end

    self:_record_scalar("pyrocaster_ring_combat_frames")
    if hp_pct(player) < self.options.retreat_hp_pct then
        return self:_set_move(reverse_direction(toward))
    end
    if dist > self.options.pyrocaster_ring_max_range then
        self:_record_scalar("pyrocaster_ring_close_frames")
        return self:_set_move(toward)
    elseif dist < self.options.pyrocaster_ring_min_range then
        self:_record_scalar("pyrocaster_ring_reposition_frames")
        return self:_set_move(reverse_direction(toward))
    end
    return false
end

function AutomatedPlayerInputSource:_necromancer_combat_action(enemies)
    local player = self.player
    local closest = enemies[1]
    if not closest then
        return false
    end

    local pct = hp_pct(player)
    local regenerating = player:has_effect("Baleful Regeneration")
    local dist = math.sqrt(distance_sqr(player, closest))
    local toward = direction_between(player, closest, 6)

    if regenerating then
        self:_record_scalar("necromancer_regen_combat_frames")
    end

    local regen = find_spell(player, "Baleful Regeneration")
    if regen and not self:_spell_throttled(regen.name)
        and self:_spell_reasonable(regen.name, enemies) then
        return self:_order_spell(regen)
    end

    if self:_try_emergency_item(enemies) then
        return true
    end

    if pct < self.options.necromancer_retreat_hp_pct and not regenerating then
        self:_record_scalar("necromancer_retreat_frames")
        return self:_set_move(reverse_direction(toward))
    end

    local spell = self:_choose_spell(enemies)
    if spell and spell.name ~= "Baleful Regeneration" then
        return self:_order_spell(spell)
    end

    if dist > self.options.necromancer_pain_range then
        self:_record_scalar("necromancer_close_frames")
        return self:_set_move(toward)
    end

    self:set("should_use_weapon", true)
    self:_record_scalar("weapon_orders")
    self:_record_scalar("necromancer_pain_orders")
    if regenerating then
        self:_record_scalar("necromancer_pain_during_regen_orders")
    end
    return true
end

function AutomatedPlayerInputSource:_pyrocaster_combat_action(enemies)
    if self:_try_emergency_item(enemies) then
        return true
    end

    local spell = self:_choose_spell(enemies)
    if spell then
        self:_order_spell(spell)
        self:_pyrocaster_ring_move(enemies, spell)
        return true
    end

    self:set("should_use_weapon", true)
    self:_record_scalar("weapon_orders")
    self:_pyrocaster_ring_move(enemies, nil)
    return true
end

function AutomatedPlayerInputSource:_combat_action(enemies)
    if #enemies == 0 then
        return false
    end
    self:_record_scalar("combat_frames")

    if self.player.class_name == "Necromancer" then
        return self:_necromancer_combat_action(enemies)
    end
    if self.player.class_name == "Pyrocaster" then
        return self:_pyrocaster_combat_action(enemies)
    end

    if self:_try_emergency_item(enemies) then
        return true
    end

    local spell = self:_choose_spell(enemies)
    if spell then
        self:_order_spell(spell)
        self:_combat_move(enemies, true)
        return true
    end

    self:set("should_use_weapon", true)
    self:_record_scalar("weapon_orders")
    self:_combat_move(enemies, false)
    return true
end

function AutomatedPlayerInputSource:_try_rest_if_needed(enemies)
    if #enemies > 0 then
        return false
    end
    if self.player:can_benefit_from_rest() then
        self:_record_scalar("rest_frames")
        return true
    end
    return false
end

function AutomatedPlayerInputSource:_random_direction()
    local dir = {0, 0}
    while dir[1] == 0 and dir[2] == 0 do
        dir[1] = self.rng:random_choice({-1, 0, 1})
        dir[2] = self.rng:random_choice({-1, 0, 1})
    end
    return dir
end

function AutomatedPlayerInputSource:_travel_action()
    local force_unstick = self.same_square_frames > 80
    if force_unstick then
        self.queued_movements = {}
    elseif #self.queued_movements > 0 then
        return self:_set_move(table.remove(self.queued_movements, 1))
    end

    local dir
    if force_unstick then
        dir = self:_random_direction()
        self:_record_scalar("unstick_frames")
    else
        dir = self.ai_state:get_next_direction()
        if not dir and self.options.start_map_desc and self.options.start_map_desc ~= "" then
            local target = nearest_alive_enemy(self.player)
            if target then
                self.ai_state.path_planner:set_path_towards(target)
                dir = self.ai_state.path_planner:next_direction_towards()
                if dir then
                    self:_record_scalar("hunt_enemy_frames")
                end
            end
        end
        if not dir then
            dir = self.player:direction_towards_unexplored()
        end
        if not dir then
            dir = self.ai_state:get_next_wander_direction()
        end
        if not dir then
            dir = self:_random_direction()
            self:_record_scalar("random_walk_frames")
        end
    end

    if dir then
        self:_record_scalar("explore_frames")
        local queued_repeats = force_unstick and 0 or 3
        for _ = 1, queued_repeats do
            self.queued_movements[#self.queued_movements + 1] = dir
        end
        return self:_set_move(dir)
    end
    return false
end

function AutomatedPlayerInputSource:_update_stuck_state()
    local player = self.player
    if math.abs(player.x - self.last_x) < 1 and math.abs(player.y - self.last_y) < 1 then
        self.same_square_frames = self.same_square_frames + 1
    else
        self.same_square_frames = 0
    end
    self.last_x = player.x
    self.last_y = player.y
    self.telemetry.max_same_square_frames = math.max(
        self.telemetry.max_same_square_frames or 0,
        self.same_square_frames
    )
end

function AutomatedPlayerInputSource:_clear_stall_state()
    self.same_square_frames = 0
    self.last_x = self.player.x
    self.last_y = self.player.y
end

function AutomatedPlayerInputSource:step()
    self.ticks = self.ticks + 1
    self:_record_scalar("controlled_frames")
    self.ai_state:step()
    self:_update_stuck_state()

    local enemies = visible_enemies(self.player)
    self.telemetry.max_visible_enemies = math.max(self.telemetry.max_visible_enemies or 0, #enemies)
    self:_record_visible_enemies(enemies)

    if self:_combat_action(enemies) then
        self:_clear_stall_state()
        return
    end
    if is_on_portal(self.player) then
        self:_record_scalar("portal_stop_frames")
        self:_clear_stall_state()
        return
    end
    if self:_try_rest_if_needed(enemies) then
        self:_clear_stall_state()
        return
    end
    if self:_travel_action() then
        return
    end

    self:_record_scalar("idle_frames")
end

function AutomatedPlayerInputSource:should_end()
    return self.same_square_frames >= self.options.stuck_end_frames
end

function AutomatedPlayerInputSource:known_spell_names()
    local names = {}
    for _, spell in ipairs(known_spells(self.player)) do
        names[#names + 1] = spell.name
    end
    return names
end

return AutomatedPlayerInputSource
