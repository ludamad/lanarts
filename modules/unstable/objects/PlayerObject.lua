local ObjectUtils = import "lanarts.objects.ObjectUtils"
local AttackableObject = import ".AttackableObject"
local Attacks = import "@Attacks"
local Apts = import "@stats.AptitudeTypes"
local Relations = import "lanarts.objects.Relations"
local SkillType = import "@SkillType"
local StatContext = import "@StatContext"

local PlayerObject = ObjectUtils.type_create(AttackableObject) 
PlayerObject.PLAYER_TRAIT = "PLAYER_TRAIT"

function PlayerObject.create_player_stats(race, --[[Can-be-nil]] class, name, --[[Optional]] team)
    local stats = race.on_create(name, team or Relations.TEAM_PLAYER_DEFAULT)
    for skill in values(SkillType.list) do
        table.insert(stats.skills, skill:on_create())
    end

    if class then
        local context = StatContext.stat_context_create(stats)
        class:on_init(context)
    end

    return stats
end

local DEFAULT_UNARMED_ATTACK = Attacks.attack_create(0, 5, Apts.MELEE)

function PlayerObject.create(args)
    assert(args.sprite and args.race and args.class and args.name)

    -- Set up type signature
    args.type = args.type or PlayerObject
    args.traits = args.traits or {}
    table.insert(args.traits, PlayerObject.PLAYER_TRAIT)

    -- AttackableObject configuration
    args.can_attack = true
    args.base_stats = PlayerObject.create_player_stats(args.race, args.class, args.name)

    -- Create pseudo-objects
    args.unarmed_attack = args.unarmed_attack or args.race.unarmed_attack or DEFAULT_UNARMED_ATTACK

    return PlayerObject._base_create(PlayerObject, args)
end

function PlayerObject.is_player(obj)
    return table.contains(obj.traits, PlayerObject.PLAYER_TRAIT)
end

function PlayerObject:is_local_player() 
    return true -- TODO
end

return PlayerObject