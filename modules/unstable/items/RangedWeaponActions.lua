local StatContext = import "@StatContext"
local ItemTraits = import ".ItemTraits"
local Actions = import "@Actions"
local StatPrereqs = import "@StatPrereqs"
local ProjectileEffect = import "@stats.ProjectileEffect"

local M = nilprotect {} -- Submodule

local AmmoCostEffect = newtype()
M.AmmoCostEffect = AmmoCostEffect
function AmmoCostEffect:init(--[[Optional]] amount) 
    self.amount = amount or 1
end
function AmmoCostEffect:apply(user)
    local ammo = StatContext.get_equipped_item(user, ItemTraits.AMMUNITION)
    assert(ammo)
    StatContext.remove_item(user, ammo, self.amount)
end

local AmmoFireEffect = newtype()
M.AmmoFireEffect = AmmoFireEffect

function AmmoFireEffect:init(action, trait)
    self.trait = trait
    self.action = action
    self.use_weapon = true
end
function AmmoFireEffect:apply(user, target)
    local ammo = StatContext.get_equipped_item(user, ItemTraits.AMMUNITION)
    local weapon = StatContext.get_equipped_item(user, ItemTraits.WEAPON)
    ammo:on_fire(user, self.use_weapon and weapon or nil, self.action, target)
end

function M.add_ranged_weapon_effect_and_prereq(actions, action_projectile_hit, ammo_trait, --[[Optional]] amount)
    local equip_prereq = StatPrereqs.EquipmentPrereq.create(
        ItemTraits.AMMUNITION, ItemTraits.ammunition_types[ammo_trait], 
        --[[No type check]] nil, ammo_trait, amount
    )
    table.insert(actions.prerequisites, equip_prereq)
    table.insert_all(actions.effects, {AmmoFireEffect.create(action_projectile_hit, ammo_trait), AmmoCostEffect.create(amount)})
end

local ActionModUtils
function M.default_ammunition_on_fire(self, user, weapon, action, target)
    ActionModUtils = ActionModUtils or import "@stats.ActionModUtils" -- Lazy import
    if self.damage_bonus or self.effectiveness_bonus then
        action = ActionModUtils.add_attack_damage_and_effectiveness(
            action, --[[Clone]] true, 
            self.damage_bonus or 0,
            self.effectiveness_bonus or 0
        )
    end
    ProjectileEffect.create {
        sprite=self.sprite, 
        radius=self.radius, 
        speed=self.speed, 
        action = action
    }:apply(user, target)
end

return M