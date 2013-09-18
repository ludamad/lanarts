local AttackableObject = import ".AttackableObject"
local Attacks = import "@Attacks"
local Apts = import "@stats.AptitudeTypes"
local Relations = import "lanarts.objects.Relations"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local SkillType = import "@SkillType"
local StatContext = import "@StatContext"
local GameObject = import "core.GameObject"

local PlayerObject = ObjectUtils.type_create(AttackableObject) 
PlayerObject.PLAYER_TRAIT = "PLAYER_TRAIT"

function PlayerObject.create_player_stats(race, --[[Can-be-nil]] class, name, --[[Optional]] team)
    local stats = race.on_create(name, team or Relations.TEAM_PLAYER_DEFAULT)
    if class then
        local context = StatContext.stat_context_create(stats)
        class:on_init(context)
    end

    return stats
end

local function resolve_sprite(race)
    local dir = path_resolve("sprites/"..race.name:lower())
    local results = io.directory_search(dir, "*.png", true)
    return image_cached_load(random_choice(results))
end

function PlayerObject.create(args)
    args.sprite = resolve_sprite(args.race)
    assert(args.race and args.class and args.name)

    -- Set up type signature
    args.type = args.type or PlayerObject
    args.traits = args.traits or {}
    table.insert(args.traits, PlayerObject.PLAYER_TRAIT)

    -- AttackableObject configuration
    args.can_attack = true
    args.base_stats = PlayerObject.create_player_stats(args.race, args.class, args.name)

    -- Create pseudo-objects
    args.unarmed_attack = args.unarmed_attack or args.race.attack 
    args.base_create = GameObject.player_create

    return PlayerObject.base_create(args)
end

function PlayerObject:on_draw()
    self.sprite:draw(self.xy)
end

function PlayerObject.is_player(obj)
    return table.contains(obj.traits, PlayerObject.PLAYER_TRAIT)
end

function PlayerObject:is_local_player() 
    return true -- TODO
end

return PlayerObject
