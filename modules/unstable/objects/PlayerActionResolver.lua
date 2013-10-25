local Keys = import "core.Keyboard"
local Mouse = import "core.Mouse"
local Display = import "core.Display"
local Map = import "core.Map"
local ColorConstants = import "@ui.ColorConstants"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local ActionResolvers = import ".ActionResolvers"
local StatContext = import "@StatContext"
local PathUtils = import ".PathUtils"
local PathFinding = import "core.PathFinding"
local ItemTraits = import "@items.ItemTraits"

local M = nilprotect {} -- Submodule

local Base = ActionResolvers.ActionResolverBase
local PlayerResolver = newtype {parent = Base}
M.create = PlayerResolver.create

--------------------------------------------------------------------------------
--                           IO Events                                        --
--------------------------------------------------------------------------------
local function resolve_movement(R, obj, event)
    if event.dx and not Map.object_tile_check(obj, {obj.x+event.dx,obj.y+R.dy}) then
        R.dx = event.dx
    end
    if event.dy and not Map.object_tile_check(obj, {obj.x+R.dx,obj.y+event.dy}) then
        R.dy = event.dy
    end
end

local PATH_FINDING_CONTEXT = {96, 96}
local path_buffer = PathFinding.astar_buffer_create()
local function resolve_pathfind(R, obj, event)
    local coords = vector_add(Mouse.mouse_xy, Display.display_xy)
    R.path = PathUtils.path_calculate(path_buffer, obj.map, obj.xy, coords, PATH_FINDING_CONTEXT, --[[Imperfect knowledge]] true)
end

local function resolve_action(R, obj, event)
    local action = event.action
    if type(action) == "function" then action = action(event, obj) end
    R.action = action
    R.target = ObjectUtils.find_closest_hostile(obj):stat_context()
end

local io_events = {
    {
        triggers = {'H'}, callback = resolve_action,
        action = function(self, obj) return obj:weapon_action_context() end,
        -- Drawing hooks
        draw_width = 64,
        _options = {origin = Display.CENTER_TOP},
        draw = function(self, obj, xy)
            local action = self:action(obj)
            self._options.color = with_alpha(COL_PALE_YELLOW, 1.0)
            Fonts.small:draw(self._options, {xy[1] + 16, xy[2] - 16}, self.triggers[1])
            action.source.sprite:draw(xy)
            local ammo = StatContext.get_equipped_item(obj:stat_context(), ItemTraits.AMMUNITION)
            if ammo then
                local ammo_xy = vector_add(xy, {32,0})
                ammo.sprite:draw(ammo_xy)
                ammo_xy[1], ammo_xy[2] = ammo_xy[1] + 3, ammo_xy[2] + 3
                self._options.color = COL_WHITE 
                Fonts.small:draw(self._options, ammo_xy, self.amount or 1)
            end
        end,
        post_draw = function(self, obj, xy)
            local color = ColorConstants.FILLED_OUTLINE
            local ammo = StatContext.get_equipped_item(obj:stat_context(), ItemTraits.AMMUNITION)
            Display.draw_rectangle_outline(color, bbox_create(xy, {ammo and 63 or 31,32}))
        end
    },
    {triggers = {'w', Keys.UP}, callback = resolve_movement, dy = -1},
    {triggers = {'s', Keys.DOWN}, callback = resolve_movement, dy = 1},
    {triggers = {'a', Keys.LEFT}, callback = resolve_movement, dx = -1},
    {triggers = {'d', Keys.RIGHT}, callback = resolve_movement, dx = 1},
    {triggers = {function() return Mouse.mouse_left_held end}, callback = resolve_pathfind}
}

local function get_spell(obj, i)
    return obj.base_stats.spells.spells[i]
end

for i,key in ipairs {'Y', 'U', 'I', 'O', 'P'} do
    table.insert(io_events, {
        triggers = {key}, callback = resolve_action,
        action = function(self, obj) return obj:spell_action_context(get_spell(obj, i)) end,
        -- Drawing hooks
        draw_width = 32,
        _options = {origin = Display.CENTER_TOP},
        draw = function(self, obj, xy)
            local spell = get_spell(obj, i)
            self._options.color = with_alpha(COL_PALE_YELLOW, (spell ~= nil) and 1.0 or 0.2)
            Fonts.small:draw(self._options, {xy[1] + 16, xy[2] - 16}, self.triggers[1])
            if spell then
                spell.sprite:draw(xy)
            end
        end,
        post_draw = function(self, obj, xy)
            local color = get_spell(obj, i) and ColorConstants.FILLED_OUTLINE or ColorConstants.UNFILLED_OUTLINE
            Display.draw_rectangle_outline(color, bbox_create(xy, {32,32}))
        end
    })
end

local function trigger_matches(io_event)
    for _, trigger in ipairs(io_event.triggers) do
        if type(trigger) == "function" then
            if trigger(io_event) then return true end
        elseif Keys.key_held(trigger) then return true end
    end
    return false
end

function PlayerResolver:init(cgroup)
    Base.init(self, cgroup)
    self.io_events = io_events 
    self.path = false
end

function PlayerResolver:actionbar_size()
    local sum = 0
    for _, event in ipairs(io_events) do
        if event.draw then sum = sum + 1 end
    end
    return {sum * 32, 32}
end

local _xy = {0,0} -- Cache
function PlayerResolver:actionbar_draw(obj, xy)
    if self.path then
        self.path:draw_path(obj)
    end
    for method in values {"draw", "post_draw"} do
        _xy[1], _xy[2] = xy[1] + 1, xy[2]
        for _, event in ipairs(io_events) do
            if event.draw then
                event[method](event, obj, _xy)
                _xy[1] = _xy[1] + event.draw_width
            end
        end
    end
end

function PlayerResolver:resolve_action(obj)
    perf.timing_begin("resolve_action")
    self.dx, self.dy = 0,0
    -- Poll all io events
    for _, io_event in ipairs(self.io_events) do
        if trigger_matches(io_event) then
            io_event.callback(self, obj, io_event)
        end
    end
    if self.dx == 0 and self.dy == 0 and self.path then
        self.preferred_velocity[1], self.preferred_velocity[2] = self.path:follow_path(obj, obj:speed())
    else
        self.path = false
        local speed = obj:speed()
        self.preferred_velocity[1] = self.dx * speed
        self.preferred_velocity[2] = self.dy * speed
    end
    perf.timing_end("resolve_action")
end

return M
