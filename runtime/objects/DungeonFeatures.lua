local GameObject = require "core.GameObject"
local GameState = require "core.GameState"
local Map = require "core.Map"
local Display = require "core.Display"
local EventLog = require "ui.EventLog"

local ObjectUtils = require "objects.ObjectUtils"
local LuaGameObject = require "objects.LuaGameObject"
local GlobalData = require "core.GlobalData"

local M = nilprotect {} -- Submodule

M._door_closed, M._door_open = unpack(Display.images_load ("objects/plain_door.png%32x32"))
M.FEATURE_TRAIT = "feature"
M.FEATURE_DEPTH = 100

local DEACTIVATION_DISTANCE = 768

local function n_lanarts_picked_up()
    local n = 0
    for _, _ in pairs(GlobalData.lanarts_picked_up) do
        n = n + 1
    end
    return n
end

-- Base
M.FeatureBase = LuaGameObject.type_create()
local Base = M.FeatureBase
function Base:init(args)
    Base.parent_init(self, args.xy, args.radius or 15, args.solid, args.depth or M.FEATURE_DEPTH)
    self.traits = self.traits or {}
    self.sprites = {}
    table.insert(self.traits, M.FEATURE_TRAIT)
end 
function Base:on_draw()
    if Display.object_within_view(self) then
        local sprite = self.sprite or self.sprites[GameState.screen_get()] 
        if sprite ~= nil then
            ObjectUtils.screen_draw(sprite, self.xy, self.alpha, self.frame)
        end
    end
end

-- Decoration
M.Decoration = LuaGameObject.type_create({base = Base})
local Decoration = M.Decoration
function Decoration:on_step()
--    if self.sprite or Map.distance_to_player(self.map, self.xy) >= DEACTIVATION_DISTANCE then
--        return -- Need to be able to scale to many deactivated instances
--    end 
    for screen_idx in screens() do
        if self.sprites[screen_idx] ~= self.real_sprite and Map.object_visible(self) then
            self.sprites[screen_idx] = self.real_sprite
        end
    end
end
function Decoration:init(args)
    Decoration.parent_init(self, args)
    self.real_sprite = assert(args.sprite)
    self.depth = args.depth or M.FEATURE_DEPTH
    self.frame = args.frame or 0
end

local test_mode = os.getenv("LANARTS_TESTCASE")
M.Door = LuaGameObject.type_create({base = Base})
local Door = M.Door
local DOOR_OPEN_TIMEOUT = 128
local function is_solid(obj) 
    return obj.solid
end

function Door:on_step()
--    if Map.distance_to_player(self.map, self.xy) >= DEACTIVATION_DISTANCE then
--        return -- Need to be able to scale to many deactivated instances
--    end 

    if self.open_timeout > 0 then
        self.open_timeout = self.open_timeout - 1
        return
    end

    local needs_key = (self.required_key ~= false)
    local needs_lanarts = false
    local can_player_open = true
    if needs_key then
        if not GlobalData.keys_picked_up[self.required_key] then
            can_player_open = false
        end
    end
    if self.lanarts_needed > 0 then
        if n_lanarts_picked_up() < self.lanarts_needed then
            can_player_open = false
            needs_lanarts = true
        end
    end
    -- Honour invincible mode as exploration mode:
    if settings.invincible then
        can_player_open = true
    end
    local is_open = false
    local collisions = Map.rectangle_collision_check(self.map, self.unpadded_area, self)
    collisions = table.filter(collisions, is_solid) 
    if #collisions > 0 and can_player_open then
        is_open = true
    else 
        local collisions = Map.rectangle_collision_check(self.map, self.area, self)
        for _, object in ipairs(collisions) do
            if not needs_key and not needs_lanarts and (object.is_enemy ~= nil) then
                is_open = true
                break
            elseif (object.is_enemy == false) and can_player_open then
                is_open = true
                break
            end
            if object.is_enemy == false and needs_lanarts then
                for _ in screens() do
                    EventLog.add("You require " .. self.lanarts_needed .. " Lanarts to open these doors!", COL_RED)
                end
            end
        end
    end
    if test_mode then
        is_open = true
    end

    if is_open then
        self.open_timeout = DOOR_OPEN_TIMEOUT
    end

    if is_open ~= self.was_open then
        local tile_xy = ObjectUtils.tile_xy(self, true)
        if is_open then
            play_sound "sound/door.ogg"
        else
            --play_sound "sound/doorClose_3.ogg"
        end
        Map.tile_set_solid(self.map, tile_xy, not is_open)
        Map.tile_set_seethrough(self.map, tile_xy, is_open)
    end

    for screen_idx in screens() do
        local real_sprite = is_open and self.open_sprite or self.closed_sprite
        if self.sprites[screen_idx] ~= real_sprite and Map.object_visible(self) then
            self.sprites[screen_idx] = real_sprite
        end
    end
    self.was_open = is_open
end
function Door:on_map_init()
    local tile_xy = ObjectUtils.tile_xy(self, true)
    self.was_open = false
    if not test_mode then
        Map.tile_set_solid(self.map, ObjectUtils.tile_xy(self, true), true)
    end
    Map.tile_set_seethrough(self.map, tile_xy, false)

    local whalf = self.open_sprite.width / 2 + self.padding
    local hhalf = self.open_sprite.height / 2 + self.padding

    self.area = {
        self.x - whalf, self.y - hhalf,
        self.x + whalf, self.y + hhalf
    }

    self.unpadded_area = {
        self.x - whalf + self.padding, self.y - hhalf + self.padding,
        self.x + whalf - self.padding, self.y + hhalf - self.padding
    }
    self.open_timeout = 0
end
function Door:init(args)
    Door.parent_init(self, args)
    self.open_sprite = args.open_sprite or M._door_open
    self.closed_sprite = args.closed_sprite or M._door_closed
    self.depth = args.depth or M.FEATURE_DEPTH
    self.padding = 6
    self.required_key = args.required_key or false
    self.lanarts_needed = args.lanarts_needed or 0
end

M.Chest = LuaGameObject.type_create({base = Decoration})
local Chest = M.Chest
local DOOR_OPEN_TIMEOUT = 128
function Chest:on_map_init()
    local whalf = self.real_sprite.width / 2
    local hhalf = self.real_sprite.height / 2 
    self.area = {
        self.x - whalf, self.y - hhalf,
        self.x + whalf, self.y + hhalf
    }
end

function Chest:on_step()
    Decoration.on_step(self)

    if self.was_opened then
        return
    end
    -- If not opened, check for players:
    local collisions = Map.rectangle_collision_check(self.map, self.area, self)
    for _, object in ipairs(collisions) do
        -- TODO hackish single-shot player check:
        if (object.is_enemy == false) then
            self.was_opened = true
            self.real_sprite = M._chest_open
            for _, content in ipairs(self.contents) do
                ObjectUtils.spawn_item_near(self, content.type, content.amount or 1, --[[Avoid self]] true)
            end
            play_sound "sound/inventory_sound_effects/metal-clash.ogg"
            break
        end
    end

end

M._chest_open = tosprite("spr_gates.chest-open")
M._chest_closed = tosprite("spr_gates.chest-closed")
function Chest:init(args)
    args.sprite = M._chest_closed
    Chest.parent_init(self, args)
    self.was_opened = false
    -- Check contents arg:
    for _, content in ipairs(args.contents) do
        assert(content.type)
    end
    self.contents = args.contents
    self.depth = M.FEATURE_DEPTH
end

return M
