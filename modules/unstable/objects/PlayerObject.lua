local CombatObject = import ".CombatObject"
local Attacks = import "@Attacks"
local Display = import "core.Display"
local Relations = import "lanarts.objects.Relations"
local Map = import "core.Map"
local PlayerActionResolver = import ".PlayerActionResolver"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local SkillType = import "@SkillType"
local RaceType = import "@RaceType"
local StatContext = import "@StatContext"
local GameObject = import "core.GameObject"
local LogUtils = import "lanarts.LogUtils"
local ActionResolvers = import ".ActionResolvers"

local PlayerObject = ObjectUtils.type_create(CombatObject)
PlayerObject.PLAYER_TRAIT = "PLAYER_TRAIT"

local function player_preferences(args)
    return nilprotect {
        manual_skill_point_spending = args.manual_skill_point_spending or false 
    }
end

function PlayerObject.player_stats_create(race, --[[Can-be-nil]] class, name)
    local stats = RaceType.resolve(race).on_create(name)
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

function PlayerObject:on_death()
    print("YOU DIED")
    os.exit()
end

function PlayerObject:_autospend_skill_points()
    local B = self.base_stats
    if B.skill_points > 0 then
        self.class:on_spend_skill_points(self:stat_context(), B.skill_points, --[[Log]] true)
        B.skill_points = 0
    end
end

function PlayerObject:on_init()
    self.base.on_init(self)
end

function PlayerObject:gain_xp(xp)
    LogUtils.event_log_resolved(self, ("<The >$You gain{s} %dXP!"):format(xp), COL_YELLOW)
    self.base.gain_xp(self, xp)
    local P = self.preferences 
    if not P.manual_skill_point_spending then
        self:_autospend_skill_points()
    end
end

local shadow = Display.image_load(path_resolve "sprites/shadow.png")

function PlayerObject:on_predraw()
    if Map.object_visible(self) then
        ObjectUtils.screen_draw(shadow, self.xy)
    end
end

function PlayerObject.create(args)
    args.sprite = resolve_sprite(args.race)
    args.action_resolver = PlayerActionResolver.create(args.collision_group)
    args.preferences = player_preferences(args.preferences or {})
    assert(args.race and args.class and args.name)
    args.team = args.team or Relations.TEAM_PLAYER_DEFAULT
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