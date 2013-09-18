local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"
local Display = import "core.Display"

local ObjectUtils = import ".ObjectUtils"

local M = nilprotect {} -- Submodule

M._door_closed, M._door_open = unpack(Display.images_load (path_resolve "plain_door.png%32x32"))
M.FEATURE_TRAIT = "feature"
M.FEATURE_DEPTH = 100

local DEACTIVATION_DISTANCE = 768

-- Base
local Base = ObjectUtils.type_create()
M.FeatureBase = Base

function Base.create(args)
    args.traits = args.trait or {M.FEATURE_TRAIT}
    return args.base_create(args)
end 

function Base:on_draw()
    if self.sprite and Display.object_within_view(self) then
        ObjectUtils.screen_draw(self.sprite, self.xy, self.alpha, self.frame)
    end
end

-- Decoration
local Decoration = ObjectUtils.type_create(Base)
M.Decoration = Decoration

function Decoration:on_step()
    if self.sprite or GameMap.distance_to_player(self.map, self.xy) >= DEACTIVATION_DISTANCE then
        return -- Need to be able to scale to many deactivated instances
    end 
    if GameMap.object_visible(self) then
        self.sprite = self.real_sprite
    end
end

function Decoration.create(args)
    assert(args.sprite)
    args.real_sprite = args.sprite
    args.sprite = nil
    args.depth = args.depth or M.FEATURE_DEPTH
    args.frame = args.frame or 0
    return Decoration.base.create(args)
end

-- Door
local Door = ObjectUtils.type_create(Base)
M.Door = Door
local DOOR_OPEN_TIMEOUT = 128

function Door:on_step()
    if GameMap.distance_to_player(self.map, self.xy) >= DEACTIVATION_DISTANCE then
        return -- Need to be able to scale to many deactivated instances
    end 

    if self.open_timeout > 0 then
        self.open_timeout = self.open_timeout - 1
        return
    end

    local is_open = false
    local collisions = GameMap.rectangle_collision_check(self.map, self.area, self)
    for object in values(collisions) do
        if object.team then -- TODO proper combat object detection
            is_open = true
            break
        end
    end

    if is_open then
        self.open_timeout = DOOR_OPEN_TIMEOUT
    end

    if is_open ~= self.was_open then
        local tile_xy = ObjectUtils.tile_xy(self, true)
        GameMap.tile_set_solid(self.map, tile_xy, not is_open)
        GameMap.tile_set_seethrough(self.map, tile_xy, is_open)
    end

    local real_sprite = is_open and self.open_sprite or self.closed_sprite
    if self.sprite ~= real_sprite and GameMap.object_visible(self) then
        self.sprite = real_sprite
    end

    self.was_open = is_open
end

function Door:on_init()
    local tile_xy = ObjectUtils.tile_xy(self, true)
    self.was_open = false
    GameMap.tile_set_solid(self.map, ObjectUtils.tile_xy(self, true), true)
    GameMap.tile_set_seethrough(self.map, tile_xy, false)

    local whalf = self.open_sprite.width / 2 + self.padding
    local hhalf = self.open_sprite.height / 2 + self.padding

    self.area = {
        self.x - whalf, self.y - hhalf,
        self.x + whalf, self.y + hhalf
    }

    self.open_timeout = 0
end

function Door.create(args)
    args.open_sprite = args.open_sprite or M._door_open
    args.closed_sprite = args.closed_sprite or M._door_closed
    args.depth = args.depth or M.FEATURE_DEPTH
    args.padding = 4
    args.type = Door
    return Door.base.create(args)
end

return M