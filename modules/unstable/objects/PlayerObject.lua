local CombatObject = import ".CombatObject"
local Attacks = import "@Attacks"
local Apts = import "@stats.AptitudeTypes"
local Relations = import "lanarts.objects.Relations"
local PlayerIOActions = import ".PlayerIOActions"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local SkillType = import "@SkillType"
local StatContext = import "@StatContext"
local GameObject = import "core.GameObject"

local PlayerObject = ObjectUtils.type_create(CombatObject)
PlayerObject.PLAYER_TRAIT = "PLAYER_TRAIT"

function PlayerObject.player_stats_create(race, --[[Can-be-nil]] class, name, --[[Optional]] team)
    local stats = race.on_create(name, team or Relations.TEAM_PLAYER_DEFAULT)
    if class then
        local context = StatContext.stat_context_create(stats)
        class:on_init(context)
    end
    for skill in values(stats.skills) do
        if skill.level ~= 0 then
        io.write('[',skill.name, '=>',skill.level,']\n')
        end
    end

    return stats
end

local function resolve_sprite(race)
    local dir = path_resolve("sprites/"..race.name:lower())
    local results = io.directory_search(dir, "*.png", true)
    return image_cached_load(random_choice(results))
end

function PlayerObject:on_step()
    -- Death event:
    if self.base_stats.hp <= 0 then
        print("YOU DIED")
        os.exit(0)
    end
    self.base.on_step(self)
    table.clear(self.queued_io_actions)
    self:io_action_handler(self.queued_io_actions)
    for io_action in values(self.queued_io_actions) do
        PlayerIOActions.use_io_action(self, unpack(io_action))
    end
end

function PlayerObject.create(args)
    args.sprite = resolve_sprite(args.race)
    args.queued_io_actions = {}
    args.io_action_handler = args.io_action_handler or do_nothing
    assert(args.race and args.class and args.name and args.io_action_handler)

    -- Set up type signature
    args.type = args.type or PlayerObject
    args.traits = args.traits or {}
    table.insert(args.traits, PlayerObject.PLAYER_TRAIT)

    -- CombatObject configuration
    args.base_stats = PlayerObject.player_stats_create(args.race, args.class, args.name)

    -- Create pseudo-objects
    args.unarmed_action = args.unarmed_action or args.race.unarmed_action 
    args.base_create = GameObject.player_create

    return PlayerObject.base_create(args)
end

function PlayerObject.is_player(obj)
    return table.contains(obj.traits, PlayerObject.PLAYER_TRAIT)
end

function PlayerObject:is_local_player() 
    return true -- TODO
end

return PlayerObject
