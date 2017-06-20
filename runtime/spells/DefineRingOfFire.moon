EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"
DataW = require "DataWrapped"
LuaGameObject = require "objects.LuaGameObject"

spell_object_create = (T) ->

    LuaGameObject.type_create(T)

RingOfFireObject = LuaGameObject.type_create {
    init: (caster) =>
        @owner = caster
        @parent_init(@, @owner.xy)
    on_step: () =>
        if @owner.destroyed
            @owner = nil
            @destroy()
            return
    _position: () =>

}

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


DataW.spell_create {
    name: "Ring of Fire",
    description: ""
    types: {"Red"}
    prereq_func: (caster) -> return true
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
    projectile: {
        speed: 5
        damage_multiplier: 2.00
    }
    mp_cost: 10
    spell_cooldown: 400
    cooldown: 35
}
