local ObjectUtils = import "lanarts.objects.ObjectUtils"
local AttackableObject = import ".AttackableObject"
local Attacks = import "@Attacks"
local Apts = import "@stats.AptitudeTypes"
local Relations = import "lanarts.objects.Relations"
local SkillType = import "@SkillType"

local PlayerObject = ObjectUtils.type_create(AttackableObject) 
PlayerObject.PLAYER_TRAIT = "PLAYER_TRAIT"

local function create_player_stats(race, name, --[[Optional]] team)
    local meta = {__copy = function(t1,t2) 
        table.copy(t1,t2, --[[Do not invoke meta]] false)
    end}

    local stats = race.on_create(name, team or Relations.TEAM_PLAYER_DEFAULT)
    for skill in values(SkillType.list) do
        local slot = {type = skill, level = 0, experience = 0}
        table.insert(stats.skills, setmetatable(slot, meta))
    end
    return stats
end

local DEFAULT_UNARMED_ATTACK = Attacks.attack_create(0, 5, Apts.MELEE)

function PlayerObject.create(args)
    args(args.sprite and args.race and args.name)

    -- Set up type signature
    args.type = args.type or PlayerObject
    args.traits = args.traits or {}
    table.insert(args.traits, PlayerObject.PLAYER_TRAIT)

    -- AttackableObject configuration
    args.can_attack = true
    args.base_stats = create_player_stats(args.race, args.name)

    -- Create pseudo-objects
    args.unarmed_attack = args.unarmed_attack or args.race.unarmed_attack or DEFAULT_UNARMED_ATTACK

    return PlayerObject.base.create(args)
end

function PlayerObject.is_player(obj)
    return table.contains(obj.traits, PlayerObject.PLAYER_TRAIT)
end

function PlayerObject:is_local_player() 
    return true -- TODO
end

return PlayerObject